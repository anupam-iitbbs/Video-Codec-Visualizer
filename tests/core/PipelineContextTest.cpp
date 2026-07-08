#include <gtest/gtest.h>

#include "ivcv/core/PipelineContext.h"

namespace ivcv::core {
namespace {

TEST(PipelineContextTest, StartsWithEmptyExecutionLog) {
    PipelineContext context;
    EXPECT_TRUE(context.executionLog().empty());
}

TEST(PipelineContextTest, RecordsStageExecutionInOrder) {
    PipelineContext context;

    context.recordStageExecution("RGB to YUV");
    context.recordStageExecution("Chroma Subsampling");

    const auto& log = context.executionLog();
    ASSERT_EQ(log.size(), 2u);
    EXPECT_EQ(log[0], "RGB to YUV");
    EXPECT_EQ(log[1], "Chroma Subsampling");
}

TEST(PipelineContextTest, ResetClearsExecutionLog) {
    PipelineContext context;
    context.recordStageExecution("RGB to YUV");

    context.reset();

    EXPECT_TRUE(context.executionLog().empty());
}

}  // namespace
}  // namespace ivcv::core
