#pragma once

#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <vector>

namespace ivcv::core {

/// @brief A simple, dependency-free, owning 2D pixel buffer.
///
/// ImageBuffer<T> stores a rectangular grid of pixels with a configurable
/// number of interleaved channels (e.g. 3 for RGB, 1 for a single luma
/// plane). It intentionally avoids any dependency on OpenCV's cv::Mat so
/// that algorithms in core/ remain independent of any particular imaging
/// library. ImageLoader is the only class that touches cv::Mat directly;
/// it copies pixel data into an ImageBuffer at the system boundary.
///
/// Storage is row-major, interleaved by channel:
///   index(x, y, c) = (y * width + x) * channels + c
///
/// @tparam T Pixel component storage type (e.g. std::uint8_t, float).
template <typename T>
class ImageBuffer {
public:
    /// @brief Constructs an empty (0x0) buffer.
    ImageBuffer() = default;

    /// @brief Constructs a buffer with the given dimensions, zero-initialized.
    /// @param width Image width in pixels. Must be > 0.
    /// @param height Image height in pixels. Must be > 0.
    /// @param channels Number of interleaved channels per pixel. Must be > 0.
    /// @throws std::invalid_argument if any dimension is zero.
    ImageBuffer(std::size_t width, std::size_t height, std::size_t channels)
        : width_(width), height_(height), channels_(channels) {
        if (width_ == 0 || height_ == 0 || channels_ == 0) {
            throw std::invalid_argument(
                "ImageBuffer: width, height and channels must all be non-zero");
        }
        data_.resize(width_ * height_ * channels_, T{});
    }

    /// @brief Read/write access to a single pixel component.
    /// @throws std::out_of_range if x, y or c are outside the buffer bounds.
    [[nodiscard]] T& at(std::size_t x, std::size_t y, std::size_t c) {
        return data_[indexChecked(x, y, c)];
    }

    /// @brief Read-only access to a single pixel component.
    /// @throws std::out_of_range if x, y or c are outside the buffer bounds.
    [[nodiscard]] const T& at(std::size_t x, std::size_t y, std::size_t c) const {
        return data_[indexChecked(x, y, c)];
    }

    /// @brief Fills every element of the buffer with @p value.
    void fill(T value) {
        std::fill(data_.begin(), data_.end(), value);
    }

    /// @brief Raw contiguous pixel storage, row-major, channel-interleaved.
    [[nodiscard]] T* rawData() noexcept { return data_.data(); }

    /// @brief Raw contiguous pixel storage (read-only).
    [[nodiscard]] const T* rawData() const noexcept { return data_.data(); }

    [[nodiscard]] std::size_t width() const noexcept { return width_; }
    [[nodiscard]] std::size_t height() const noexcept { return height_; }
    [[nodiscard]] std::size_t channels() const noexcept { return channels_; }

    /// @brief True if the buffer has zero width or height (default-constructed).
    [[nodiscard]] bool empty() const noexcept { return width_ == 0 || height_ == 0; }

private:
    std::size_t indexChecked(std::size_t x, std::size_t y, std::size_t c) const {
        if (x >= width_ || y >= height_ || c >= channels_) {
            throw std::out_of_range("ImageBuffer: pixel access out of bounds");
        }
        return (y * width_ + x) * channels_ + c;
    }

    std::size_t width_ = 0;
    std::size_t height_ = 0;
    std::size_t channels_ = 0;
    std::vector<T> data_;
};

}  // namespace ivcv::core
