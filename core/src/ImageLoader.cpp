#include "ivcv/core/ImageLoader.h"

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <stdexcept>

namespace ivcv::core {

ImageBuffer<std::uint8_t> ImageLoader::loadRgb(const std::string& path) {
    // OpenCV decodes into BGR by convention; we reorder to RGB immediately
    // so every consumer of ImageBuffer in this project can assume channel
    // order 0=R, 1=G, 2=B without needing to know about OpenCV at all.
    cv::Mat bgr = cv::imread(path, cv::IMREAD_COLOR);
    if (bgr.empty()) {
        throw std::runtime_error("ImageLoader: failed to read image at '" + path + "'");
    }

    cv::Mat rgb;
    cv::cvtColor(bgr, rgb, cv::COLOR_BGR2RGB);

    ImageBuffer<std::uint8_t> buffer(
        static_cast<std::size_t>(rgb.cols), static_cast<std::size_t>(rgb.rows), 3);

    for (int y = 0; y < rgb.rows; ++y) {
        const auto* rowPtr = rgb.ptr<std::uint8_t>(y);
        for (int x = 0; x < rgb.cols; ++x) {
            const std::uint8_t* pixel = rowPtr + (x * 3);
            const auto ux = static_cast<std::size_t>(x);
            const auto uy = static_cast<std::size_t>(y);
            buffer.at(ux, uy, 0) = pixel[0];
            buffer.at(ux, uy, 1) = pixel[1];
            buffer.at(ux, uy, 2) = pixel[2];
        }
    }

    return buffer;
}

}  // namespace ivcv::core
