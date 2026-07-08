#pragma once

#include <cstdint>
#include <string>
#include <string_view>

#include "ivcv/core/ImageBuffer.h"
#include "ivcv/core/PipelineStage.h"

namespace ivcv::core {

/// Which ITU-R recipient coefficients are used to convert between RGB and
/// Y'CbCr. The two standards differ only in the luma/chroma weighting
/// coefficients (Kr, Kg, Kb). See modules/color_space/README.md for the
/// full intuitive explanation and mathematical derivation.
enum class ColorSpaceStandard {
    BT601,  ///< ITU-R BT.601 (standard-definition): Kr = 0.299, Kb = 0.114.
    BT709,  ///< ITU-R BT.709 (high-definition): Kr = 0.2126, Kb = 0.0722.
};

/// Converts an 8-bit RGB image to full-range 8-bit Y'CbCr and back.
///
/// Pipeline role: this is Stage 2 of the still-image pipeline. process()
/// reads PipelineContext::rgbImage() (populated by ImageLoader in the
/// Input stage) and writes PipelineContext::yuvImage(), leaving rgbImage()
/// untouched so the UI and later stages can always show input and output
/// side by side.
///
/// Complexity: O(width * height) time, one pass, a single output
/// allocation, no dependency on Qt or OpenCV.
///
/// See also: modules/color_space/README.md (educational note: intuitive
/// explanation, derivation, pseudocode, complexity, standards references,
/// worked example) and docs/ARCHITECTURE.md section 2 (IPipelineStage).
class ColorSpaceConverter : public IPipelineStage {
public:
    explicit ColorSpaceConverter(ColorSpaceStandard standard = ColorSpaceStandard::BT601);

    [[nodiscard]] std::string_view name() const noexcept override;
    void process(PipelineContext& context) override;
    [[nodiscard]] ParameterSet parameters() const override;
    void reset() override;

    /// Changes the active conversion standard. Takes effect on the next
    /// process()/convertRgbToYuv()/convertYuvToRgb() call.
    void setStandard(ColorSpaceStandard standard) noexcept;
    [[nodiscard]] ColorSpaceStandard standard() const noexcept;

    /// Converts a single RGB image to Y'CbCr using this converter's active
    /// standard. Exposed as a free-standing operation (not only via
    /// process()) so the algorithm is directly unit-testable and reusable
    /// without constructing a PipelineContext.
    /// @param rgb A 3-channel image buffer (channels() == 3).
    /// @returns A 3-channel Y'CbCr image buffer with the same dimensions.
    /// @throws std::invalid_argument if rgb does not have exactly 3 channels.
    [[nodiscard]] ImageBuffer<std::uint8_t> convertRgbToYuv(
        const ImageBuffer<std::uint8_t>& rgb) const;

    /// Inverse of convertRgbToYuv, used by the UI to preview reconstruction
    /// fidelity and, from Stage 5 onward, by stages that must reconstruct
    /// a displayable image after lossy transforms.
    /// @throws std::invalid_argument if yuv does not have exactly 3 channels.
    [[nodiscard]] ImageBuffer<std::uint8_t> convertYuvToRgb(
        const ImageBuffer<std::uint8_t>& yuv) const;

private:
    ColorSpaceStandard standard_;
};

}  // namespace ivcv::core
