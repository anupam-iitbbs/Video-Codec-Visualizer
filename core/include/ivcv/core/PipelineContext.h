#pragma once

#include <cstdint>
#include <string>
#include <vector>

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
/// read by ChromaSubsampler starting in Stage 3). See
/// docs/ARCHITECTURE.md, section 6, for how later stages will extend this
/// context further (subsampled planes, per-block coefficient data, and so
/// on) without requiring a rewrite of earlier stages.
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

    /// Clears the execution log and resets both image buffers to empty,
    /// returning the context to its initial state.
    void reset();

private:
    std::vector<std::string> executionLog_;
    ImageBuffer<std::uint8_t> rgbImage_;
    ImageBuffer<std::uint8_t> yuvImage_;
};

} // namespace ivcv::core
