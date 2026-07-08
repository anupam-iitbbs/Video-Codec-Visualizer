#include "ivcv/core/BlockPartitioner.h"

#include <algorithm>
#include <stdexcept>

#include "ivcv/core/PipelineContext.h"

namespace ivcv::core {

namespace {

[[nodiscard]] bool isPowerOfTwo(std::size_t value) noexcept {
    return value > 0 && (value & (value - 1)) == 0;
}

/// Recursively splits the size x size (or smaller, at plane edges) block
/// rooted at (x, y) into BlockPartitioner::partition()'s output, appending
/// leaves to blocks and assigning each the next sequential id.
void partitionRecursive(
    const ImageBuffer<std::uint8_t>& plane, std::size_t x, std::size_t y, std::size_t size,
    std::size_t minBlockSize, double varianceThreshold, std::size_t& nextId,
    std::vector<Block>& blocks) {
    const std::size_t width = std::min(size, plane.width() - x);
    const std::size_t height = std::min(size, plane.height() - y);
    const double variance = BlockPartitioner::computeVariance(plane, x, y, width, height);

    const bool canSplit = size > minBlockSize;
    if (canSplit && variance > varianceThreshold) {
        const std::size_t half = size / 2;
        // The parent block itself was reached with a valid (x, y), so the
        // top-left child is always in bounds; the other three quadrants may
        // fall entirely outside the plane when it is not an exact multiple
        // of the top-level maxBlockSize, and are simply skipped.
        partitionRecursive(plane, x, y, half, minBlockSize, varianceThreshold, nextId, blocks);
        if (x + half < plane.width()) {
            partitionRecursive(
                plane, x + half, y, half, minBlockSize, varianceThreshold, nextId, blocks);
        }
        if (y + half < plane.height()) {
            partitionRecursive(
                plane, x, y + half, half, minBlockSize, varianceThreshold, nextId, blocks);
        }
        if (x + half < plane.width() && y + half < plane.height()) {
            partitionRecursive(
                plane, x + half, y + half, half, minBlockSize, varianceThreshold, nextId, blocks);
        }
        return;
    }

    blocks.push_back(Block{nextId++, x, y, width, height, variance});
}

} // namespace

BlockPartitioner::BlockPartitioner(
    std::size_t maxBlockSize, std::size_t minBlockSize, double varianceThreshold)
    : maxBlockSize_(maxBlockSize), minBlockSize_(minBlockSize),
      varianceThreshold_(varianceThreshold) {}

std::string_view BlockPartitioner::name() const noexcept {
    return "Block Partitioning";
}

void BlockPartitioner::process(PipelineContext& context) {
    context.blocks() =
        partition(context.yPlane(), maxBlockSize_, minBlockSize_, varianceThreshold_);
}

ParameterSet BlockPartitioner::parameters() const {
    return ParameterSet{
        {"maxBlockSize", static_cast<int>(maxBlockSize_)},
        {"minBlockSize", static_cast<int>(minBlockSize_)},
        {"varianceThreshold", varianceThreshold_},
    };
}

void BlockPartitioner::reset() {
    maxBlockSize_ = 64;
    minBlockSize_ = 8;
    varianceThreshold_ = 50.0;
}

void BlockPartitioner::setMaxBlockSize(std::size_t maxBlockSize) noexcept {
    maxBlockSize_ = maxBlockSize;
}

std::size_t BlockPartitioner::maxBlockSize() const noexcept {
    return maxBlockSize_;
}

void BlockPartitioner::setMinBlockSize(std::size_t minBlockSize) noexcept {
    minBlockSize_ = minBlockSize;
}

std::size_t BlockPartitioner::minBlockSize() const noexcept {
    return minBlockSize_;
}

void BlockPartitioner::setVarianceThreshold(double varianceThreshold) noexcept {
    varianceThreshold_ = varianceThreshold;
}

double BlockPartitioner::varianceThreshold() const noexcept {
    return varianceThreshold_;
}

double BlockPartitioner::computeVariance(
    const ImageBuffer<std::uint8_t>& plane, std::size_t x, std::size_t y, std::size_t width,
    std::size_t height) {
    if (plane.channels() != 1 || width == 0 || height == 0 || x + width > plane.width() ||
        y + height > plane.height()) {
        throw std::invalid_argument(
            "BlockPartitioner::computeVariance: plane must have 1 channel and the "
            "region must be non-empty and within bounds");
    }

    double sum = 0.0;
    double sumSquares = 0.0;
    for (std::size_t row = y; row < y + height; ++row) {
        for (std::size_t col = x; col < x + width; ++col) {
            const double pixel = static_cast<double>(plane.at(col, row, 0));
            sum += pixel;
            sumSquares += pixel * pixel;
        }
    }
    const double count = static_cast<double>(width) * static_cast<double>(height);
    const double mean = sum / count;
    const double meanOfSquares = sumSquares / count;
    return meanOfSquares - mean * mean;
}

std::vector<Block> BlockPartitioner::partition(
    const ImageBuffer<std::uint8_t>& plane, std::size_t maxBlockSize, std::size_t minBlockSize,
    double varianceThreshold) {
    if (plane.channels() != 1 || !isPowerOfTwo(maxBlockSize) || !isPowerOfTwo(minBlockSize) ||
        minBlockSize > maxBlockSize) {
        throw std::invalid_argument(
            "BlockPartitioner::partition: plane must have 1 channel, maxBlockSize "
            "and minBlockSize must be powers of two, and minBlockSize must not "
            "exceed maxBlockSize");
    }

    std::vector<Block> blocks;
    std::size_t nextId = 0;
    for (std::size_t y = 0; y < plane.height(); y += maxBlockSize) {
        for (std::size_t x = 0; x < plane.width(); x += maxBlockSize) {
            partitionRecursive(
                plane, x, y, maxBlockSize, minBlockSize, varianceThreshold, nextId, blocks);
        }
    }
    return blocks;
}

} // namespace ivcv::core
