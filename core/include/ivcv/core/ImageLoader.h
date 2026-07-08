#pragma once

#include <cstdint>
#include <string>

#include "ivcv/core/ImageBuffer.h"

namespace ivcv::core {

/// Loads an image file from disk into an ImageBuffer<uint8_t>, isolating
/// the rest of core/ from OpenCV's I/O routines and cv::Mat type (see
/// docs/ARCHITECTURE.md, section 1: the Platform/IO layer).
///
/// This is a stateless utility rather than an IPipelineStage: loading is a
/// one-shot action triggered by the user picking a file, not a repeatable
/// step in the compression pipeline, so it intentionally does not carry
/// parameters(), reset(), or a slot in PipelineController's stage sequence.
class ImageLoader {
public:
    ImageLoader() = delete;

    /// Reads the image file at @p path and returns it as a 3-channel
    /// (RGB, not OpenCV's native BGR) ImageBuffer<uint8_t>.
    ///
    /// Complexity: O(width * height), one decode pass plus one channel
    /// reorder/copy pass into the destination buffer.
    ///
    /// @throws std::runtime_error if the file cannot be found, read, or
    ///     decoded by OpenCV's image codecs.
    [[nodiscard]] static ImageBuffer<std::uint8_t> loadRgb(const std::string& path);
};

}  // namespace ivcv::core
