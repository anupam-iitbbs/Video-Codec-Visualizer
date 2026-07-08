#include "ivcv/core/ChromaSubsampler.h"

#include <gtest/gtest.h>

#include "ivcv/core/PipelineContext.h"

namespace ivcv::core {
namespace {

TEST(ChromaSubsamplerTest, NameReturnsStageName) {
    ChromaSubsampler stage;
    EXPECT_EQ(stage.name(), "Chroma Subsampling");
}

TEST(ChromaSubsamplerTest, DefaultsToYuv420AndBoxFilter) {
    ChromaSubsampler stage;
    EXPECT_EQ(stage.mode(), ChromaSubsamplingMode::Yuv420);
    EXPECT_EQ(stage.filterMethod(), ChromaFilterMethod::Box);
}

TEST(ChromaSubsamplerTest, ExtractPlaneCopiesRequestedChannel) {
    ImageBuffer<std::uint8_t> yuv(2, 1, 3);
    yuv.at(0, 0, 0) = 100;
    yuv.at(0, 0, 1) = 150;
    yuv.at(0, 0, 2) = 200;
    yuv.at(1, 0, 0) = 10;
    yuv.at(1, 0, 1) = 20;
    yuv.at(1, 0, 2) = 30;

    const auto luma = ChromaSubsampler::extractPlane(yuv, 0);
    const auto cb = ChromaSubsampler::extractPlane(yuv, 1);
    const auto cr = ChromaSubsampler::extractPlane(yuv, 2);

    EXPECT_EQ(luma.at(0, 0, 0), 100);
    EXPECT_EQ(luma.at(1, 0, 0), 10);
    EXPECT_EQ(cb.at(0, 0, 0), 150);
    EXPECT_EQ(cb.at(1, 0, 0), 20);
    EXPECT_EQ(cr.at(0, 0, 0), 200);
    EXPECT_EQ(cr.at(1, 0, 0), 30);
}

TEST(ChromaSubsamplerTest, ExtractPlaneThrowsOnWrongChannelCount) {
    ImageBuffer<std::uint8_t> rgb(2, 2, 3);
    EXPECT_THROW(ChromaSubsampler::extractPlane(rgb, 3), std::invalid_argument);

    ImageBuffer<std::uint8_t> singleChannel(2, 2, 1);
    EXPECT_THROW(ChromaSubsampler::extractPlane(singleChannel, 0), std::invalid_argument);
}

TEST(ChromaSubsamplerTest, Downsample444ReturnsUnchangedCopy) {
    ImageBuffer<std::uint8_t> plane(2, 2, 1);
    plane.at(0, 0, 0) = 10;
    plane.at(1, 0, 0) = 20;
    plane.at(0, 1, 0) = 30;
    plane.at(1, 1, 0) = 40;

    const auto out = ChromaSubsampler::downsample(
        plane, ChromaSubsamplingMode::Yuv444, ChromaFilterMethod::Box);

    ASSERT_EQ(out.width(), 2u);
    ASSERT_EQ(out.height(), 2u);
    EXPECT_EQ(out.at(0, 0, 0), 10);
    EXPECT_EQ(out.at(1, 0, 0), 20);
    EXPECT_EQ(out.at(0, 1, 0), 30);
    EXPECT_EQ(out.at(1, 1, 0), 40);
}

TEST(ChromaSubsamplerTest, DownsampleBox420AveragesTwoByTwoBlock) {
    ImageBuffer<std::uint8_t> plane(2, 2, 1);
    plane.at(0, 0, 0) = 10;
    plane.at(1, 0, 0) = 20;
    plane.at(0, 1, 0) = 30;
    plane.at(1, 1, 0) = 40;

    const auto out = ChromaSubsampler::downsample(
        plane, ChromaSubsamplingMode::Yuv420, ChromaFilterMethod::Box);

    ASSERT_EQ(out.width(), 1u);
    ASSERT_EQ(out.height(), 1u);
    EXPECT_EQ(out.at(0, 0, 0), 25); // (10+20+30+40)/4
}

TEST(ChromaSubsamplerTest, DownsampleBox422AveragesTwoByOneBlock) {
    ImageBuffer<std::uint8_t> plane(4, 1, 1);
    plane.at(0, 0, 0) = 10;
    plane.at(1, 0, 0) = 20;
    plane.at(2, 0, 0) = 30;
    plane.at(3, 0, 0) = 40;

    const auto out = ChromaSubsampler::downsample(
        plane, ChromaSubsamplingMode::Yuv422, ChromaFilterMethod::Box);

    ASSERT_EQ(out.width(), 2u);
    ASSERT_EQ(out.height(), 1u);
    EXPECT_EQ(out.at(0, 0, 0), 15); // (10+20)/2
    EXPECT_EQ(out.at(1, 0, 0), 35); // (30+40)/2
}

TEST(ChromaSubsamplerTest, DownsampleNearestPicksTopLeftPixelOfEachBlock) {
    ImageBuffer<std::uint8_t> plane(2, 2, 1);
    plane.at(0, 0, 0) = 10;
    plane.at(1, 0, 0) = 20;
    plane.at(0, 1, 0) = 30;
    plane.at(1, 1, 0) = 40;

    const auto out = ChromaSubsampler::downsample(
        plane, ChromaSubsamplingMode::Yuv420, ChromaFilterMethod::Nearest);

    ASSERT_EQ(out.width(), 1u);
    ASSERT_EQ(out.height(), 1u);
    EXPECT_EQ(out.at(0, 0, 0), 10);
}

TEST(ChromaSubsamplerTest, DownsampleHandlesOddDimensionsWithoutOutOfBoundsRead) {
    ImageBuffer<std::uint8_t> plane(3, 1, 1);
    plane.at(0, 0, 0) = 10;
    plane.at(1, 0, 0) = 20;
    plane.at(2, 0, 0) = 30;

    const auto out = ChromaSubsampler::downsample(
        plane, ChromaSubsamplingMode::Yuv422, ChromaFilterMethod::Box);

    ASSERT_EQ(out.width(), 2u);
    EXPECT_EQ(out.at(0, 0, 0), 15); // (10+20)/2
    EXPECT_EQ(out.at(1, 0, 0), 30); // last block is a single pixel
}

TEST(ChromaSubsamplerTest, DownsampleThrowsOnWrongChannelCount) {
    ImageBuffer<std::uint8_t> threeChannel(2, 2, 3);
    EXPECT_THROW(
        ChromaSubsampler::downsample(
            threeChannel, ChromaSubsamplingMode::Yuv420, ChromaFilterMethod::Box),
        std::invalid_argument);
}

TEST(ChromaSubsamplerTest, UpsampleReplicatesEachSampleAcrossItsBlock) {
    ImageBuffer<std::uint8_t> subsampled(2, 1, 1);
    subsampled.at(0, 0, 0) = 10;
    subsampled.at(1, 0, 0) = 20;

    const auto out = ChromaSubsampler::upsample(subsampled, 4, 2, ChromaSubsamplingMode::Yuv420);

    ASSERT_EQ(out.width(), 4u);
    ASSERT_EQ(out.height(), 2u);
    for (std::size_t y = 0; y < 2; ++y) {
        EXPECT_EQ(out.at(0, y, 0), 10);
        EXPECT_EQ(out.at(1, y, 0), 10);
        EXPECT_EQ(out.at(2, y, 0), 20);
        EXPECT_EQ(out.at(3, y, 0), 20);
    }
}

TEST(ChromaSubsamplerTest, UpsampleThrowsOnWrongChannelCount) {
    ImageBuffer<std::uint8_t> threeChannel(2, 2, 3);
    EXPECT_THROW(
        ChromaSubsampler::upsample(threeChannel, 4, 4, ChromaSubsamplingMode::Yuv420),
        std::invalid_argument);
}

TEST(ChromaSubsamplerTest, ProcessWritesPlanesAndModeOnContext) {
    PipelineContext context;
    context.yuvImage() = ImageBuffer<std::uint8_t>(2, 2, 3);
    context.yuvImage().at(0, 0, 0) = 10;
    context.yuvImage().at(1, 0, 0) = 20;
    context.yuvImage().at(0, 1, 0) = 30;
    context.yuvImage().at(1, 1, 0) = 40;
    context.yuvImage().at(0, 0, 1) = 100;
    context.yuvImage().at(0, 0, 2) = 200;

    ChromaSubsampler stage(ChromaSubsamplingMode::Yuv420, ChromaFilterMethod::Box);
    stage.process(context);

    ASSERT_EQ(context.yPlane().width(), 2u);
    ASSERT_EQ(context.yPlane().height(), 2u);
    EXPECT_EQ(context.yPlane().at(0, 0, 0), 10);

    ASSERT_EQ(context.cbPlane().width(), 1u);
    ASSERT_EQ(context.cbPlane().height(), 1u);
    ASSERT_EQ(context.crPlane().width(), 1u);
    ASSERT_EQ(context.crPlane().height(), 1u);

    EXPECT_EQ(context.chromaSubsamplingMode(), ChromaSubsamplingMode::Yuv420);
}

TEST(ChromaSubsamplerTest, ParametersReportsModeAndFilter) {
    ChromaSubsampler stage(ChromaSubsamplingMode::Yuv422, ChromaFilterMethod::Nearest);
    const ParameterSet params = stage.parameters();

    ASSERT_TRUE(params.count("mode") > 0);
    ASSERT_TRUE(params.count("filter") > 0);
    EXPECT_EQ(std::get<int>(params.at("mode")), static_cast<int>(ChromaSubsamplingMode::Yuv422));
    EXPECT_EQ(
        std::get<int>(params.at("filter")), static_cast<int>(ChromaFilterMethod::Nearest));
}

TEST(ChromaSubsamplerTest, ResetRestoresDefaults) {
    ChromaSubsampler stage(ChromaSubsamplingMode::Yuv444, ChromaFilterMethod::Nearest);
    stage.reset();
    EXPECT_EQ(stage.mode(), ChromaSubsamplingMode::Yuv420);
    EXPECT_EQ(stage.filterMethod(), ChromaFilterMethod::Box);
}

} // namespace
} // namespace ivcv::core
