#pragma once

#include <cstddef>
#include <string_view>
#include <vector>

#include "ivcv/core/Block.h"
#include "ivcv/core/ImageBuffer.h"
#include "ivcv/core/PipelineStage.h"

namespace ivcv::core {

/// Splits the luma plane into a quadtree of blocks so later stages (DCT
/// onward) transform-code regions at a size that matches how much detail
/// they actually contain, instead of a single fixed block size everywhere.
///
/// Pipeline role: this is Stage 4 of the still-image pipeline. process()
/// reads PipelineContext::yPlane() (written by ChromaSubsampler in Stage
/// 3) and writes PipelineContext::blocks(): a flat list of Block leaves
/// covering the whole plane with no gaps and no overlaps. Each top-level
/// maxBlockSize() x maxBlockSize() region is recursively quartered while
/// its variance exceeds varianceThreshold() and its size is still above
/// minBlockSize(), mirroring the coding-tree-unit quadtree used by
/// HEVC/H.265 (see modules/block_partitioning/README.md for the full
/// derivation, pseudocode, and standards references).
///
/// Complexity: O(width * height * log2(maxBlockSize / minBlockSize))
/// time -- each pixel is read once per quadtree level it participates in,
/// and the number of levels is a small constant (3 for the default 64
/// down to 8). No dependency on Qt or OpenCV.
///
/// See also: modules/block_partitioning/README.md and FR-4 in
/// docs/SRS.md.
class BlockPartitioner : public IPipelineStage {
public:
    explicit BlockPartitioner(
        std::size_t maxBlockSize = 64, std::size_t minBlockSize = 8,
        double varianceThreshold = 50.0);

    [[nodiscard]] std::string_view name() const noexcept override;
    void process(PipelineContext& context) override;
    [[nodiscard]] ParameterSet parameters() const override;
    void reset() override;

    /// Changes the size of the top-level (un-split) blocks the plane is
    /// first tiled into. Must be a power of two and >= minBlockSize().
    /// Takes effect on the next process()/partition() call.
    void setMaxBlockSize(std::size_t maxBlockSize) noexcept;
    [[nodiscard]] std::size_t maxBlockSize() const noexcept;

    /// Changes the smallest block size the recursion is allowed to
    /// produce. Must be a power of two, >= 1, and <= maxBlockSize().
    /// Takes effect on the next process()/partition() call.
    void setMinBlockSize(std::size_t minBlockSize) noexcept;
    [[nodiscard]] std::size_t minBlockSize() const noexcept;

    /// Changes the per-block variance above which a block larger than
    /// minBlockSize() is split into four quadrants instead of being kept
    /// as a leaf. Takes effect on the next process()/partition() call.
    void setVarianceThreshold(double varianceThreshold) noexcept;
    [[nodiscard]] double varianceThreshold() const noexcept;

    /// Computes the population variance of the width x height region of
    /// plane starting at (x, y): mean(pixel^2) - mean(pixel)^2. Exposed
    /// as a free-standing operation so it is directly unit-testable
    /// without constructing a PipelineContext.
    /// @throws std::invalid_argument if plane does not have exactly 1
    /// channel, width or height is 0, or the region falls outside plane.
    [[nodiscard]] static double computeVariance(
        const ImageBuffer<std::uint8_t>& plane, std::size_t x, std::size_t y,
        std::size_t width, std::size_t height);

    /// Recursively partitions plane into a quadtree of Block leaves as
    /// described in the class documentation above. Blocks along the
    /// right/bottom edges are narrower/shorter than their nominal size
    /// whenever plane's dimensions are not an exact multiple of
    /// maxBlockSize, so every pixel belongs to exactly one leaf and no
    /// leaf extends outside plane. Leaves are returned in the order they
    /// are discovered (top-level blocks left-to-right, top-to-bottom;
    /// each block's own children before its siblings), and ids are
    /// assigned sequentially in that same order starting at 0.
    /// @throws std::invalid_argument if plane does not have exactly 1
    /// channel, maxBlockSize/minBlockSize are not powers of two, or
    /// minBlockSize > maxBlockSize.
    [[nodiscard]] static std::vector<Block> partition(
        const ImageBuffer<std::uint8_t>& plane, std::size_t maxBlockSize,
        std::size_t minBlockSize, double varianceThreshold);

private:
    std::size_t maxBlockSize_;
    std::size_t minBlockSize_;
    double varianceThreshold_;
};

} // namespace ivcv::core
