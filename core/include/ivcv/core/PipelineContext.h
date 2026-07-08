#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "ivcv/core/Block.h"
#include "ivcv/core/ChromaSubsamplingMode.h"
#include "ivcv/core/ImageBuffer.h"

namespace ivcv::core {

/// Shared, mutable state passed between pipeline stages as they execute.
///
/// Stage 1 scope was limited to the execution log, which was enough to
/// prove the PipelineController orchestration machinery end-to-end. Stage 2
/// adds the two image buffers every still-image stage from here on reads
/// from and/or writes to: rgbImage (loaded once by ImageLoader and left
/// untouched thereafter so the UI can always show the original alongside
/// any transformed output) and yuvImage (written by ColorSpaceConverter,
/// read by ChromaSubsampler starting in Stage 3). Stage 3 adds
/// yPlane()/cbPlane()/crPlane(): ChromaSubsampler splits yuvImage() into a
/// full-resolution Y' plane and Cb/Cr planes reduced according to the
/// active ChromaSubsamplingMode, which is recorded here too so later
/// stages and the UI can interpret or upsample those planes without
/// re-deriving how they were produced. Stage 4 adds blocks(): the
/// quadtree of Block leaves BlockPartitioner computes by recursively
/// splitting yPlane() based on per-block variance, read by DCT (Stage 5)
/// onward as the unit of transform coding, and by the UI to draw the
/// partition grid and answer per-block queries. See docs/ARCHITECTURE.md,
/// section 6, for how later stages will extend this context further
/// (per-block coefficient data, and so on) without requiring a rewrite of
/// earlier stages.
class PipelineContext {
public:
    PipelineContext() = default;

    /// Appends stageName to the execution log. Called by
    /// PipelineController immediately after a stage's process() call
    /// completes successfully.
    void recordStageExecution(std::string stageName);

    /// Returns the ordered list of stage names executed so far, oldest
    /// first.
    [[nodiscard]] const std::vector<std::string>& executionLog() const noexcept;

    /// The original input image, populated by ImageLoader and never
    /// modified by any later stage.
    [[nodiscard]] ImageBuffer<std::uint8_t>& rgbImage() noexcept;
    [[nodiscard]] const ImageBuffer<std::uint8_t>& rgbImage() const noexcept;

    /// The Y'CbCr image produced by ColorSpaceConverter::process().
    [[nodiscard]] ImageBuffer<std::uint8_t>& yuvImage() noexcept;
    [[nodiscard]] const ImageBuffer<std::uint8_t>& yuvImage() const noexcept;

    /// The full-resolution Y' (luma) plane, written by
    /// ChromaSubsampler::process().
    [[nodiscard]] ImageBuffer<std::uint8_t>& yPlane() noexcept;
    [[nodiscard]] const ImageBuffer<std::uint8_t>& yPlane() const noexcept;

    /// The Cb (blue-difference chroma) plane. Its resolution depends on
    /// chromaSubsamplingMode().
    [[nodiscard]] ImageBuffer<std::uint8_t>& cbPlane() noexcept;
    [[nodiscard]] const ImageBuffer<std::uint8_t>& cbPlane() const noexcept;

    /// The Cr (red-difference chroma) plane. Its resolution depends on
    /// chromaSubsamplingMode().
    [[nodiscard]] ImageBuffer<std::uint8_t>& crPlane() noexcept;
    [[nodiscard]] const ImageBuffer<std::uint8_t>& crPlane() const noexcept;

    /// Records which subsampling mode produced the current cbPlane()/
    /// crPlane() contents, so later stages and UI code can upsample or
    /// otherwise interpret them correctly without the caller separately
    /// tracking which mode was used.
    void setChromaSubsamplingMode(ChromaSubsamplingMode mode) noexcept;
    [[nodiscard]] ChromaSubsamplingMode chromaSubsamplingMode() const noexcept;

    /// The quadtree partition of yPlane(), written by
    /// BlockPartitioner::process(). Empty until that stage has run.
    [[nodiscard]] std::vector<Block>& blocks() noexcept;
    [[nodiscard]] const std::vector<Block>& blocks() const noexcept;

    /// Clears the execution log and resets every image buffer, plane and
    /// the block partition to empty, returning the context to its initial
    /// state.
    void reset();

private:
    std::vector<std::string> executionLog_;
    ImageBuffer<std::uint8_t> rgbImage_;
    ImageBuffer<std::uint8_t> yuvImage_;
    ImageBuffer<std::uint8_t> yPlane_;
    ImageBuffer<std::uint8_t> cbPlane_;
    ImageBuffer<std::uint8_t> crPlane_;
    ChromaSubsamplingMode chromaSubsamplingMode_ = ChromaSubsamplingMode::Yuv444;
    std::vector<Block> blocks_;
};

} // namespace ivcv::core
