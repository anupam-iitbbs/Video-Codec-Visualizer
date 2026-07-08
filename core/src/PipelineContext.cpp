#include "ivcv/core/PipelineContext.h"

namespace ivcv::core {

void PipelineContext::recordStageExecution(std::string stageName) {
    executionLog_.push_back(std::move(stageName));
}

const std::vector<std::string>& PipelineContext::executionLog() const noexcept {
    return executionLog_;
}

void PipelineContext::reset() {
    executionLog_.clear();
}

}  // namespace ivcv::core
