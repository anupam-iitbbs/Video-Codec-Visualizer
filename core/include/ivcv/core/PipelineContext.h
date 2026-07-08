#pragma once

#include <string>
#include <vector>

namespace ivcv::core {

/// Shared, mutable state passed between pipeline stages as they execute.
///
/// Stage 1 scope: PipelineContext currently tracks only the ordered log of
/// stage names that have executed, which is enough to prove the
/// PipelineController orchestration machinery end-to-end. Starting in
/// Stage 2 (RGB to YUV), image buffer members will be added here (see
/// docs/ARCHITECTURE.md, section 6, for the extensibility plan). Growing
/// this class incrementally is intentional: it avoids introducing
/// image-processing types before a stage exists that actually produces or
/// consumes them.
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

    /// Clears the execution log, returning the context to its initial
    /// state. Does not reset any other state because none exists yet in
    /// Stage 1.
    void reset();

private:
    std::vector<std::string> executionLog_;
};

}  // namespace ivcv::core
