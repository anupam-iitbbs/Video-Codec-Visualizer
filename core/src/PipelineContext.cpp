#include "ivcv/core/PipelineContext.h"

namespace ivcv::core {

void PipelineContext::recordStageExecution(std::string stageName) {
    executionLog_.push_back(std::move(stageName));
}

const std::vector<std::string>& PipelineContext::executionLog() const noexcept {
    return executionLog_;
}

ImageBuffer<std::uint8_t>& PipelineContext::rgbImage() noexcept {
    return rgbImage_;
}

const ImageBuffer<std::uint8_t>& PipelineContext::rgbImage() const noexcept {
    return rgbImage_;
}

ImageBuffer<std::uint8_t>& PipelineContext::yuvImage() noexcept {
    return yuvImage_;
}

const ImageBuffer<std::uint8_t>& PipelineContext::yuvImage() const noexcept {
    return yuvImage_;
}

void PipelineContext::reset() {
    executionLog_.clear();
    rgbImage_ = ImageBuffer<std::uint8_t>();
    yuvImage_ = ImageBuffer<std::uint8_t>();
}

} // namespace ivcv::core
