#include "ivcv/core/ImageBuffer.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <stdexcept>

namespace ivcv::core {
namespace {

TEST(ImageBufferTest, DefaultConstructedIsEmpty) {
    ImageBuffer<std::uint8_t> buffer;
    EXPECT_TRUE(buffer.empty());
    EXPECT_EQ(buffer.width(), 0u);
    EXPECT_EQ(buffer.height(), 0u);
    EXPECT_EQ(buffer.channels(), 0u);
}

TEST(ImageBufferTest, ConstructorReportsRequestedDimensions) {
    const ImageBuffer<std::uint8_t> buffer(4, 3, 3);
    EXPECT_FALSE(buffer.empty());
    EXPECT_EQ(buffer.width(), 4u);
    EXPECT_EQ(buffer.height(), 3u);
    EXPECT_EQ(buffer.channels(), 3u);
}

TEST(ImageBufferTest, IsZeroInitialized) {
    ImageBuffer<std::uint8_t> buffer(2, 2, 3);
    for (std::size_t y = 0; y < 2; ++y) {
        for (std::size_t x = 0; x < 2; ++x) {
            for (std::size_t c = 0; c < 3; ++c) {
                EXPECT_EQ(buffer.at(x, y, c), 0);
            }
        }
    }
}

TEST(ImageBufferTest, ReadWriteRoundTripDoesNotDisturbNeighbors) {
    ImageBuffer<std::uint8_t> buffer(2, 2, 3);
    buffer.at(1, 0, 2) = 200;

    EXPECT_EQ(buffer.at(1, 0, 2), 200);
    EXPECT_EQ(buffer.at(0, 0, 2), 0);
    EXPECT_EQ(buffer.at(1, 0, 1), 0);
    EXPECT_EQ(buffer.at(1, 1, 2), 0);
}

TEST(ImageBufferTest, ConstructorThrowsOnAnyZeroDimension) {
    EXPECT_THROW(ImageBuffer<std::uint8_t>(0, 3, 3), std::invalid_argument);
    EXPECT_THROW(ImageBuffer<std::uint8_t>(3, 0, 3), std::invalid_argument);
    EXPECT_THROW(ImageBuffer<std::uint8_t>(3, 3, 0), std::invalid_argument);
}

TEST(ImageBufferTest, AtThrowsOnOutOfRangeAccess) {
    ImageBuffer<std::uint8_t> buffer(2, 2, 3);
    EXPECT_THROW(buffer.at(2, 0, 0), std::out_of_range);
    EXPECT_THROW(buffer.at(0, 2, 0), std::out_of_range);
    EXPECT_THROW(buffer.at(0, 0, 3), std::out_of_range);

    const ImageBuffer<std::uint8_t>& constBuffer = buffer;
    EXPECT_THROW(constBuffer.at(2, 0, 0), std::out_of_range);
}

TEST(ImageBufferTest, FillSetsEveryElement) {
    ImageBuffer<std::uint8_t> buffer(2, 2, 1);
    buffer.fill(42);
    for (std::size_t y = 0; y < 2; ++y) {
        for (std::size_t x = 0; x < 2; ++x) {
            EXPECT_EQ(buffer.at(x, y, 0), 42);
        }
    }
}

TEST(ImageBufferTest, RawDataIsContiguousAndMatchesAt) {
    ImageBuffer<std::uint8_t> buffer(2, 2, 3);
    buffer.at(1, 1, 2) = 77;

    // index(x=1, y=1, c=2) with width=2, channels=3 -> (1*2+1)*3+2 = 11.
    EXPECT_EQ(buffer.rawData()[11], 77);
}

}  // namespace
}  // namespace ivcv::core
