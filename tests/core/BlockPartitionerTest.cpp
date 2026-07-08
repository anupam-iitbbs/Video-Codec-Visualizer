#include "ivcv/core/BlockPartitioner.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <stdexcept>

#include "ivcv/core/PipelineContext.h"

namespace ivcv::core {
namespace {

TEST(BlockPartitionerTest, NameReturnsStageName) {
    BlockPartitioner partitioner;
    EXPECT_EQ(partitioner.name(), "Block Partitioning");
}

TEST(BlockPartitionerTest, DefaultsToMax64Min8Threshold50) {
    BlockPartitioner partitioner;
    EXPECT_EQ(partitioner.maxBlockSize(), 64u);
    EXPECT_EQ(partitioner.minBlockSize(), 8u);
    EXPECT_DOUBLE_EQ(partitioner.varianceThreshold(), 50.0);
}

TEST(BlockPartitionerTest, ComputeVarianceOfUniformRegionIsZero) {
    ImageBuffer<std::uint8_t> plane(4, 4, 1);
    plane.fill(100);
    EXPECT_DOUBLE_EQ(BlockPartitioner::computeVariance(plane, 0, 0, 4, 4), 0.0);
}

TEST(BlockPartitionerTest, ComputeVarianceOfKnownValues) {
    ImageBuffer<std::uint8_t> plane(2, 2, 1);
    plane.at(0, 0, 0) = 0;
    plane.at(1, 0, 0) = 0;
    plane.at(0, 1, 0) = 255;
    plane.at(1, 1, 0) = 255;
    // mean = 127.5, mean of squares = 32512.5, variance = 32512.5 - 127.5^2
    EXPECT_NEAR(BlockPartitioner::computeVariance(plane, 0, 0, 2, 2), 16256.25, 1e-9);
}

TEST(BlockPartitionerTest, ComputeVarianceThrowsOnWrongChannelCount) {
    ImageBuffer<std::uint8_t> plane(2, 2, 3);
    EXPECT_THROW(BlockPartitioner::computeVariance(plane, 0, 0, 2, 2), std::invalid_argument);
}

TEST(BlockPartitionerTest, ComputeVarianceThrowsOnOutOfBoundsRegion) {
    ImageBuffer<std::uint8_t> plane(4, 4, 1);
    EXPECT_THROW(BlockPartitioner::computeVariance(plane, 2, 2, 4, 4), std::invalid_argument);
}

TEST(BlockPartitionerTest, PartitionReturnsSingleBlockWhenVarianceBelowThreshold) {
    ImageBuffer<std::uint8_t> plane(16, 16, 1);
    plane.fill(100);
    auto blocks = BlockPartitioner::partition(plane, 16, 8, 10.0);
    ASSERT_EQ(blocks.size(), 1u);
    EXPECT_EQ(blocks[0].id, 0u);
    EXPECT_EQ(blocks[0].x, 0u);
    EXPECT_EQ(blocks[0].y, 0u);
    EXPECT_EQ(blocks[0].width, 16u);
    EXPECT_EQ(blocks[0].height, 16u);
    EXPECT_DOUBLE_EQ(blocks[0].variance, 0.0);
}

TEST(BlockPartitionerTest, PartitionSplitsHighVarianceBlockIntoFourQuadrants) {
    ImageBuffer<std::uint8_t> plane(16, 16, 1);
    for (std::size_t y = 0; y < 16; ++y) {
        for (std::size_t x = 0; x < 16; ++x) {
            plane.at(x, y, 0) = ((x + y) % 2 == 0) ? 0 : 255;
        }
    }
    auto blocks = BlockPartitioner::partition(plane, 16, 8, 0.0);
    ASSERT_EQ(blocks.size(), 4u);
    EXPECT_EQ(blocks[0].x, 0u);
    EXPECT_EQ(blocks[0].y, 0u);
    EXPECT_EQ(blocks[1].x, 8u);
    EXPECT_EQ(blocks[1].y, 0u);
    EXPECT_EQ(blocks[2].x, 0u);
    EXPECT_EQ(blocks[2].y, 8u);
    EXPECT_EQ(blocks[3].x, 8u);
    EXPECT_EQ(blocks[3].y, 8u);
    for (const auto& block : blocks) {
        EXPECT_EQ(block.width, 8u);
        EXPECT_EQ(block.height, 8u);
        EXPECT_GT(block.variance, 0.0);
    }
}

TEST(BlockPartitionerTest, PartitionStopsAtMinBlockSizeAfterMultipleLevels) {
    ImageBuffer<std::uint8_t> plane(32, 32, 1);
    for (std::size_t y = 0; y < 32; ++y) {
        for (std::size_t x = 0; x < 32; ++x) {
            plane.at(x, y, 0) = ((x + y) % 2 == 0) ? 0 : 255;
        }
    }
    auto blocks = BlockPartitioner::partition(plane, 32, 8, 0.0);
    EXPECT_EQ(blocks.size(), 16u);
    for (const auto& block : blocks) {
        EXPECT_EQ(block.width, 8u);
        EXPECT_EQ(block.height, 8u);
    }
}

TEST(BlockPartitionerTest, PartitionHandlesNonMultipleDimensionsWithoutOutOfBoundsRead) {
    ImageBuffer<std::uint8_t> plane(20, 20, 1);
    for (std::size_t y = 0; y < 20; ++y) {
        for (std::size_t x = 0; x < 20; ++x) {
            plane.at(x, y, 0) = static_cast<std::uint8_t>((x * 7 + y * 13) % 256);
        }
    }
    auto blocks = BlockPartitioner::partition(plane, 16, 8, 1000000.0);
    ASSERT_EQ(blocks.size(), 4u);
    std::size_t totalPixels = 0;
    for (const auto& block : blocks) {
        EXPECT_LE(block.x + block.width, 20u);
        EXPECT_LE(block.y + block.height, 20u);
        totalPixels += block.width * block.height;
    }
    EXPECT_EQ(totalPixels, 400u);
}

TEST(BlockPartitionerTest, PartitionThrowsOnWrongChannelCount) {
    ImageBuffer<std::uint8_t> plane(16, 16, 3);
    EXPECT_THROW(BlockPartitioner::partition(plane, 16, 8, 50.0), std::invalid_argument);
}

TEST(BlockPartitionerTest, PartitionThrowsOnNonPowerOfTwoMaxBlockSize) {
    ImageBuffer<std::uint8_t> plane(16, 16, 1);
    EXPECT_THROW(BlockPartitioner::partition(plane, 48, 8, 50.0), std::invalid_argument);
}

TEST(BlockPartitionerTest, PartitionThrowsOnNonPowerOfTwoMinBlockSize) {
    ImageBuffer<std::uint8_t> plane(16, 16, 1);
    EXPECT_THROW(BlockPartitioner::partition(plane, 16, 6, 50.0), std::invalid_argument);
}

TEST(BlockPartitionerTest, PartitionThrowsWhenMinBlockSizeExceedsMaxBlockSize) {
    ImageBuffer<std::uint8_t> plane(16, 16, 1);
    EXPECT_THROW(BlockPartitioner::partition(plane, 8, 16, 50.0), std::invalid_argument);
}

TEST(BlockPartitionerTest, ProcessWritesBlocksOnContext) {
    PipelineContext context;
    context.yPlane() = ImageBuffer<std::uint8_t>(8, 8, 1);
    context.yPlane().fill(50);

    BlockPartitioner partitioner;
    partitioner.process(context);

    ASSERT_EQ(context.blocks().size(), 1u);
    EXPECT_EQ(context.blocks()[0].width, 8u);
    EXPECT_EQ(context.blocks()[0].height, 8u);
}

TEST(BlockPartitionerTest, ParametersReportsConfiguredValues) {
    BlockPartitioner partitioner(32, 4, 75.5);
    auto params = partitioner.parameters();
    EXPECT_EQ(std::get<int>(params.at("maxBlockSize")), 32);
    EXPECT_EQ(std::get<int>(params.at("minBlockSize")), 4);
    EXPECT_DOUBLE_EQ(std::get<double>(params.at("varianceThreshold")), 75.5);
}

TEST(BlockPartitionerTest, ResetRestoresDefaults) {
    BlockPartitioner partitioner(32, 4, 75.5);
    partitioner.reset();
    EXPECT_EQ(partitioner.maxBlockSize(), 64u);
    EXPECT_EQ(partitioner.minBlockSize(), 8u);
    EXPECT_DOUBLE_EQ(partitioner.varianceThreshold(), 50.0);
}

} // namespace
} // namespace ivcv::core
