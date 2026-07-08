#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

#include "ivcv/core/ChromaSubsamplingMode.h"
#include "ivcv/core/ImageBuffer.h"
#include "ivcv/core/PipelineStage.h"

namespace ivcv::core {

/// How the pixels within a chroma block are combined into one subsampled
/// sample. Named to mirror the "nearest / average / box" language used in
/// docs/SRS.md, FR-3. Average and Box are the same operation (the mean of
/// every pixel in the block) under two names learners recognize from
/// different textbooks and tools, so both are represented by the single
/// Box value here. See modules/chroma_subsampling/README.md for the full
/// derivation and a worked example of each method.
enum class ChromaFilterMethod {
    Nearest, ///< Keeps one representative pixel per block; cheapest, blockiest.
    Box,     ///< Mean of every pixel in the block (a.k.a. "average"); the
             ///< de facto default used by libjpeg and most real encoders.
};

/// Reduces the spatial resolution of the Cb/Cr planes relative to Y',
/// exploiting the human visual system's lower acuity for color detail
/// than for luminance detail.
///
/// Pipeline role: this is Stage 3 of the still-image pipeline. process()
/// reads PipelineContext::yuvImage() (written by ColorSpaceConverter in
/// Stage 2) and writes PipelineContext::yPlane()/cbPlane()/crPlane() as
/// three separate single-channel planes: Y' stays at full resolution,
/// Cb/Cr are reduced according to the active ChromaSubsamplingMode. Later
/// stages (Block Partitioning onward) read these separate planes rather
/// than the interleaved yuvImage(), since block-based coding always
/// operates per-plane. See docs/ARCHITECTURE.md, section 6.
///
/// Complexity: O(width * height) time, one pass per plane, no dependency
/// on Qt or OpenCV.
///
/// See also: modules/chroma_subsampling/README.md (educational note:
/// intuitive explanation, derivation, pseudocode, complexity, standards
/// references, worked example) and FR-3 in docs/SRS.md.
class ChromaSubsampler : public IPipelineStage {
public:
    explicit ChromaSubsampler(
        ChromaSubsamplingMode mode = ChromaSubsamplingMode::Yuv420,
        ChromaFilterMethod filter = ChromaFilterMethod::Box);

    [[nodiscard]] std::string_view name() const noexcept override;
    void process(PipelineContext& context) override;
    [[nodiscard]] ParameterSet parameters() const override;
    void reset() override;

    /// Changes the active subsampling mode. Takes effect on the next
    /// process()/downsample() call.
    void setMode(ChromaSubsamplingMode mode) noexcept;
    [[nodiscard]] ChromaSubsamplingMode mode() const noexcept;

    /// Changes the active filter method. Takes effect on the next
    /// process()/downsample() call.
    void setFilterMethod(ChromaFilterMethod filter) noexcept;
    [[nodiscard]] ChromaFilterMethod filterMethod() const noexcept;

    /// Copies a single channel (0 = Y', 1 = Cb, 2 = Cr) out of an
    /// interleaved 3-channel image into its own full-resolution,
    /// single-channel plane. Exposed as a free-standing operation so it
    /// is directly unit-testable without constructing a PipelineContext.
    /// @throws std::invalid_argument if yuv does not have exactly 3
    /// channels, or channelIndex is not 0, 1, or 2.
    [[nodiscard]] static ImageBuffer<std::uint8_t> extractPlane(
        const ImageBuffer<std::uint8_t>& yuv, std::size_t channelIndex);

    /// Reduces fullResPlane's resolution according to mode, combining
    /// each 2x1 (4:2:2) or 2x2 (4:2:0) block of source pixels into one
    /// output pixel using filter. 4:4:4 returns an unchanged copy. Odd
    /// width/height is handled by letting the last block along that
    /// dimension be 1 pixel instead of 2, so no source pixel is ever
    /// dropped or read out of bounds.
    /// @throws std::invalid_argument if fullResPlane does not have
    /// exactly 1 channel.
    [[nodiscard]] static ImageBuffer<std::uint8_t> downsample(
        const ImageBuffer<std::uint8_t>& fullResPlane, ChromaSubsamplingMode mode,
        ChromaFilterMethod filter);

    /// Reconstructs a full-resolution plane from a subsampled one by
    /// nearest-neighbor replication, so the UI can visualize what a
    /// decoder sees after subsampling. This is intentionally the
    /// simplest possible reconstruction; see
    /// modules/chroma_subsampling/README.md for why real decoders
    /// typically use bilinear upsampling instead, noted there as a
    /// future improvement.
    /// @throws std::invalid_argument if subsampledPlane does not have
    /// exactly 1 channel.
    [[nodiscard]] static ImageBuffer<std::uint8_t> upsample(
        const ImageBuffer<std::uint8_t>& subsampledPlane, std::size_t fullWidth,
        std::size_t fullHeight, ChromaSubsamplingMode mode);

private:
    ChromaSubsamplingMode mode_;
    ChromaFilterMethod filter_;
};

} // namespace ivcv::core
