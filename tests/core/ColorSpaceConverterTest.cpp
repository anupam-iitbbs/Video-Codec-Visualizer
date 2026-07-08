#include "ivcv/core/ColorSpaceConverter.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <stdexcept>

#include "ivcv/core/PipelineContext.h"

namespace ivcv::core {
namespace {

ImageBuffer<std::uint8_t> makeSolidColor(std::uint8_t r, std::uint8_t g, std::uint8_t b) {
    ImageBuffer<std::uint8_t> image(1, 1, 3);
    image.at(0, 0, 0) = r;
    image.at(0, 0, 1) = g;
    image.at(0, 0, 2) = b;
    return image;
}

TEST(ColorSpaceConverterTest, DefaultsToBt601) {
    const ColorSpaceConverter converter;
    EXPECT_EQ(converter.standard(), ColorSpaceStandard::BT601);
}

TEST(ColorSpaceConverterTest, BlackMapsToZeroLumaAndNeutralChroma) {
    const ColorSpaceConverter converter(ColorSpaceStandard::BT601);
    const ImageBuffer<std::uint8_t> yuv = converter.convertRgbToYuv(makeSolidColor(0, 0, 0));

    EXPECT_EQ(yuv.at(0, 0, 0), 0);
    EXPECT_EQ(yuv.at(0, 0, 1), 128);
    EXPECT_EQ(yuv.at(0, 0, 2), 128);
}

TEST(ColorSpaceConverterTest, WhiteMapsToFullLumaAndNeutralChroma) {
    const ColorSpaceConverter converter(ColorSpaceStandard::BT601);
    const ImageBuffer<std::uint8_t> yuv = converter.convertRgbToYuv(makeSolidColor(255, 255, 255));

    EXPECT_NEAR(yuv.at(0, 0, 0), 255, 1);
    EXPECT_NEAR(yuv.at(0, 0, 1), 128, 1);
    EXPECT_NEAR(yuv.at(0, 0, 2), 128, 1);
}

TEST(ColorSpaceConverterTest, Bt601MatchesHandComputedLumaForPureRed) {
    // Pure red under BT.601: Y = 0.299 * 255 = 76.245 -> rounds to 76.
    const ColorSpaceConverter converter(ColorSpaceStandard::BT601);
    const ImageBuffer<std::uint8_t> yuv = converter.convertRgbToYuv(makeSolidColor(255, 0, 0));

    EXPECT_EQ(yuv.at(0, 0, 0), 76);
}

TEST(ColorSpaceConverterTest, Bt709DiffersFromBt601ForPureRed) {
    // BT.709: Y = 0.2126 * 255 = 54.213 -> rounds to 54, clearly different
    // from BT.601's 76, proving the standard actually changes the result.
    const ColorSpaceConverter converter(ColorSpaceStandard::BT709);
    const ImageBuffer<std::uint8_t> yuv = converter.convertRgbToYuv(makeSolidColor(255, 0, 0));

    EXPECT_EQ(yuv.at(0, 0, 0), 54);
}

TEST(ColorSpaceConverterTest, RoundTripIsApproximatelyIdentity) {
    const ColorSpaceConverter converter(ColorSpaceStandard::BT601);
    const ImageBuffer<std::uint8_t> original = makeSolidColor(123, 200, 40);

    const ImageBuffer<std::uint8_t> yuv = converter.convertRgbToYuv(original);
    const ImageBuffer<std::uint8_t> reconstructed = converter.convertYuvToRgb(yuv);

    // Rounding to 8-bit integers at each step means round-trip fidelity is
    // very high but not always bit-exact; a tolerance of 2 accommodates
    // that without hiding a real conversion bug.
    for (std::size_t c = 0; c < 3; ++c) {
        EXPECT_NEAR(reconstructed.at(0, 0, c), original.at(0, 0, c), 2);
    }
}

TEST(ColorSpaceConverterTest, ConvertRgbToYuvThrowsOnWrongChannelCount) {
    const ColorSpaceConverter converter;
    const ImageBuffer<std::uint8_t> grayscale(4, 4, 1);
    EXPECT_THROW(converter.convertRgbToYuv(grayscale), std::invalid_argument);
}

TEST(ColorSpaceConverterTest, ConvertYuvToRgbThrowsOnWrongChannelCount) {
    const ColorSpaceConverter converter;
    const ImageBuffer<std::uint8_t> grayscale(4, 4, 1);
    EXPECT_THROW(converter.convertYuvToRgb(grayscale), std::invalid_argument);
}

TEST(ColorSpaceConverterTest, ProcessReadsRgbImageAndWritesYuvImageOnContext) {
    ColorSpaceConverter converter(ColorSpaceStandard::BT601);
    PipelineContext context;
    context.rgbImage() = makeSolidColor(255, 0, 0);

    converter.process(context);

    ASSERT_EQ(context.yuvImage().width(), 1u);
    EXPECT_EQ(context.yuvImage().at(0, 0, 0), 76);
    // rgbImage must be left untouched so the UI can still show the input.
    EXPECT_EQ(context.rgbImage().at(0, 0, 0), 255);
    EXPECT_EQ(context.executionLog().back(), "RGB to YUV");
}

TEST(ColorSpaceConverterTest, ResetRestoresDefaultStandard) {
    ColorSpaceConverter converter(ColorSpaceStandard::BT709);
    converter.reset();
    EXPECT_EQ(converter.standard(), ColorSpaceStandard::BT601);
}

TEST(ColorSpaceConverterTest, ParametersReportsCurrentStandard) {
    ColorSpaceConverter converter(ColorSpaceStandard::BT709);
    const ParameterSet params = converter.parameters();

    const auto it = params.find("standard");
    ASSERT_NE(it, params.end());
    EXPECT_EQ(std::get<std::string>(it->second), "BT709");
}

TEST(ColorSpaceConverterTest, NameReturnsStageName) {
    const ColorSpaceConverter converter;
    EXPECT_EQ(converter.name(), "RGB to YUV");
}

}  // namespace
}  // namespace ivcv::core
