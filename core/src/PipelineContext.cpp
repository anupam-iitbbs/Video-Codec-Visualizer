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

ImageBuffer<std::uint8_t>& PipelineContext::yPlane() noexcept {
    return yPlane_;
}

const ImageBuffer<std::uint8_t>& PipelineContext::yPlane() const noexcept {
    return yPlane_;
}

ImageBuffer<std::uint8_t>& PipelineContext::cbPlane() noexcept {
    return cbPlane_;
}

const ImageBuffer<std::uint8_t>& PipelineContext::cbPlane() const noexcept {
    return cbPlane_;
}

ImageBuffer<std::uint8_t>& PipelineContext::crPlane() noexcept {
    return crPlane_;
}

const ImageBuffer<std::uint8_t>& PipelineContext::crPlane() const noexcept {
    return crPlane_;
}

void PipelineContext::setChromaSubsamplingMode(ChromaSubsamplingMode mode) noexcept {
    chromaSubsamplingMode_ = mode;
}

ChromaSubsamplingMode PipelineContext::chromaSubsamplingMode() const noexcept {
    return chromaSubsamplingMode_;
}

std::vector<Block>& PipelineContext::blocks() noexcept {
    return blocks_;
}

const std::vector<Block>& PipelineContext::blocks() const noexcept {
    return blocks_;
}

void PipelineContext::reset() {
    executionLog_.clear();
    rgbImage_ = ImageBuffer<std::uint8_t>();
    yuvImage_ = ImageBuffer<std::uint8_t>();
    yPlane_ = ImageBuffer<std::uint8_t>();
    cbPlane_ = ImageBuffer<std::uint8_t>();
    crPlane_ = ImageBuffer<std::uint8_t>();
    chromaSubsamplingMode_ = ChromaSubsamplingMode::Yuv444;
    blocks_.clear();
}

} // namespace ivcv::core
