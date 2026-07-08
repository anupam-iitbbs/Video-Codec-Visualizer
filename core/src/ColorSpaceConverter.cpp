#include "ivcv/core/ColorSpaceConverter.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <utility>

#include "ivcv/core/PipelineContext.h"

namespace ivcv::core {

namespace {

/// Kr, Kg, Kb luma weighting coefficients for a given standard. Kg is
/// always 1 - Kr - Kb so the three sum to exactly 1 (a pixel of pure
/// white must map to full luma). See modules/color_space/README.md for
/// where these numbers come from.
struct LumaCoefficients {
    double kr;
    double kg;
    double kb;
};

[[nodiscard]] LumaCoefficients coefficientsFor(ColorSpaceStandard standard) {
    switch (standard) {
        case ColorSpaceStandard::BT601:
            return LumaCoefficients{0.299, 0.587, 0.114};
        case ColorSpaceStandard::BT709:
            return LumaCoefficients{0.2126, 0.7152, 0.0722};
    }
    // Reaching here means a new enumerator was added without updating this
    // function; fail loudly instead of silently defaulting to BT601.
    throw std::invalid_argument("ColorSpaceConverter: unknown ColorSpaceStandard");
}

[[nodiscard]] std::uint8_t clampToByte(double value) {
    return static_cast<std::uint8_t>(std::clamp(std::lround(value), 0L, 255L));
}

}  // namespace

ColorSpaceConverter::ColorSpaceConverter(ColorSpaceStandard standard) : standard_(standard) {}

std::string_view ColorSpaceConverter::name() const noexcept {
    return "RGB to YUV";
}

ParameterSet ColorSpaceConverter::parameters() const {
    return ParameterSet{
        {"standard", std::string(standard_ == ColorSpaceStandard::BT601 ? "BT601" : "BT709")},
    };
}

void ColorSpaceConverter::reset() {
    standard_ = ColorSpaceStandard::BT601;
}

void ColorSpaceConverter::setStandard(ColorSpaceStandard standard) noexcept {
    standard_ = standard;
}

ColorSpaceStandard ColorSpaceConverter::standard() const noexcept {
    return standard_;
}

void ColorSpaceConverter::process(PipelineContext& context) {
    context.yuvImage() = convertRgbToYuv(context.rgbImage());
    context.recordStageExecution(std::string(name()));
}

ImageBuffer<std::uint8_t> ColorSpaceConverter::convertRgbToYuv(
    const ImageBuffer<std::uint8_t>& rgb) const {
    if (rgb.channels() != 3) {
        throw std::invalid_argument("ColorSpaceConverter: input must have exactly 3 channels");
    }

    const LumaCoefficients coeffs = coefficientsFor(standard_);
    ImageBuffer<std::uint8_t> yuv(rgb.width(), rgb.height(), 3);

    for (std::size_t y = 0; y < rgb.height(); ++y) {
        for (std::size_t x = 0; x < rgb.width(); ++x) {
            const double r = rgb.at(x, y, 0);
            const double g = rgb.at(x, y, 1);
            const double b = rgb.at(x, y, 2);

            const double luma = coeffs.kr * r + coeffs.kg * g + coeffs.kb * b;
            const double cb = (b - luma) / (2.0 * (1.0 - coeffs.kb));
            const double cr = (r - luma) / (2.0 * (1.0 - coeffs.kr));

            yuv.at(x, y, 0) = clampToByte(luma);
            yuv.at(x, y, 1) = clampToByte(cb + 128.0);
            yuv.at(x, y, 2) = clampToByte(cr + 128.0);
        }
    }

    return yuv;
}

ImageBuffer<std::uint8_t> ColorSpaceConverter::convertYuvToRgb(
    const ImageBuffer<std::uint8_t>& yuv) const {
    if (yuv.channels() != 3) {
        throw std::invalid_argument("ColorSpaceConverter: input must have exactly 3 channels");
    }

    const LumaCoefficients coeffs = coefficientsFor(standard_);
    ImageBuffer<std::uint8_t> rgb(yuv.width(), yuv.height(), 3);

    for (std::size_t y = 0; y < yuv.height(); ++y) {
        for (std::size_t x = 0; x < yuv.width(); ++x) {
            const double luma = yuv.at(x, y, 0);
            const double cb = static_cast<double>(yuv.at(x, y, 1)) - 128.0;
            const double cr = static_cast<double>(yuv.at(x, y, 2)) - 128.0;

            const double r = luma + 2.0 * (1.0 - coeffs.kr) * cr;
            const double b = luma + 2.0 * (1.0 - coeffs.kb) * cb;
            const double g = (luma - coeffs.kr * r - coeffs.kb * b) / coeffs.kg;

            rgb.at(x, y, 0) = clampToByte(r);
            rgb.at(x, y, 1) = clampToByte(g);
            rgb.at(x, y, 2) = clampToByte(b);
        }
    }

    return rgb;
}

}  // namespace ivcv::core
