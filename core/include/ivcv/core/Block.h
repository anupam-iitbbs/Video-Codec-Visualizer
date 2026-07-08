#pragma once

#include <cstddef>

namespace ivcv::core {

/// One leaf node of the block-partitioning quadtree produced by
/// BlockPartitioner, in plane (pixel) coordinates.
struct Block {
  std::size_t id = 0;
  std::size_t x = 0;
  std::size_t y = 0;
  std::size_t width = 0;
  std::size_t height = 0;
  double variance = 0.0;
};

} // namespace ivcv::core
