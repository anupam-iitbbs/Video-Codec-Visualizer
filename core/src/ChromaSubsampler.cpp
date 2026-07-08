#include "ivcv/core/ChromaSubsampler.h"

#include <algorithm>
#include <stdexcept>

#include "ivcv/core/PipelineContext.h"

namespace ivcv::core {

namespace {

/// Block dimensions (width, height) a single subsampled sample represents
/// for a given mode. 4:4:4 is 1x1 (no reduction).
struct BlockSize {
    std::size_t width;
    std::size_t height;
};

[[nodiscard]] BlockSize blockSizeFor(ChromaSubsamplingMode mode) noexcept {
    switch (mode) {
        case ChromaSubsamplingMode::Yuv444:
            return {1, 1};
        case ChromaSubsamplingMode::Yuv422:
            return {2, 1};
        case ChromaSubsamplingMode::Yuv420:
            return {2, 2};
    }
    return {1, 1};
}

[[nodiscard]] std::size_t ceilDiv(std::size_t numerator, std::size_t denominator) noexcept {
    return (numerator + denominator - 1) / denominator;
}

} // namespace

ChromaSubsampler::ChromaSubsampler(ChromaSubsamplingMode mode, ChromaFilterMethod filter)
    : mode_(mode), filter_(filter) {}

std::string_view ChromaSubsampler::name() const noexcept {
    return "Chroma Subsampling";
}

void ChromaSubsampler::process(PipelineContext& context) {
    const auto& yuv = context.yuvImage();
    context.yPlane() = extractPlane(yuv, 0);
    context.cbPlane() = downsample(extractPlane(yuv, 1), mode_, filter_);
    context.crPlane() = downsample(extractPlane(yuv, 2), mode_, filter_);
    context.setChromaSubsamplingMode(mode_);
}

ParameterSet ChromaSubsampler::parameters() const {
    return ParameterSet{
        {"mode", static_cast<int>(mode_)},
        {"filter", static_cast<int>(filter_)},
    };
}

void ChromaSubsampler::reset() {
    mode_ = ChromaSubsamplingMode::Yuv420;
    filter_ = ChromaFilterMethod::Box;
}

void ChromaSubsampler::setMode(ChromaSubsamplingMode mode) noexcept {
    mode_ = mode;
}

ChromaSubsamplingMode ChromaSubsampler::mode() const noexcept {
    return mode_;
}

void ChromaSubsampler::setFilterMethod(ChromaFilterMethod filter) noexcept {
    filter_ = filter;
}

ChromaFilterMethod ChromaSubsampler::filterMethod() const noexcept {
    return filter_;
}

ImageBuffer<std::uint8_t> ChromaSubsampler::extractPlane(
    const ImageBuffer<std::uint8_t>& yuv, std::size_t channelIndex) {
    if (yuv.channels() != 3 || channelIndex > 2) {
        throw std::invalid_argument(
            "ChromaSubsampler::extractPlane: source must have 3 channels and "
            "channelIndex must be 0, 1, or 2");
    }
    ImageBuffer<std::uint8_t> plane(yuv.width(), yuv.height(), 1);
    for (std::size_t y = 0; y < yuv.height(); ++y) {
        for (std::size_t x = 0; x < yuv.width(); ++x) {
            plane.at(x, y, 0) = yuv.at(x, y, channelIndex);
        }
    }
    return plane;
}

ImageBuffer<std::uint8_t> ChromaSubsampler::downsample(
    const ImageBuffer<std::uint8_t>& fullResPlane, ChromaSubsamplingMode mode,
    ChromaFilterMethod filter) {
    if (fullResPlane.channels() != 1) {
        throw std::invalid_argument("ChromaSubsampler::downsample: plane must have 1 channel");
    }

    const BlockSize block = blockSizeFor(mode);
    const std::size_t outWidth = ceilDiv(fullResPlane.width(), block.width);
    const std::size_t outHeight = ceilDiv(fullResPlane.height(), block.height);
    ImageBuffer<std::uint8_t> out(outWidth, outHeight, 1);

    for (std::size_t oy = 0; oy < outHeight; ++oy) {
        for (std::size_t ox = 0; ox < outWidth; ++ox) {
            const std::size_t srcX0 = ox * block.width;
            const std::size_t srcY0 = oy * block.height;

            if (filter == ChromaFilterMethod::Nearest) {
                out.at(ox, oy, 0) = fullResPlane.at(srcX0, srcY0, 0);
                continue;
            }

            // Box filter: average every source pixel in this block. The
            // block is clamped against the plane edges so odd
            // width/height never reads out of bounds -- the last block
            // along an odd dimension is simply 1 pixel wide/tall instead
            // of 2, and averaging a single value returns that value
            // unchanged.
            const std::size_t srcX1 = std::min(srcX0 + block.width, fullResPlane.width());
            const std::size_t srcY1 = std::min(srcY0 + block.height, fullResPlane.height());

            unsigned int sum = 0;
            std::size_t count = 0;
            for (std::size_t sy = srcY0; sy < srcY1; ++sy) {
                for (std::size_t sx = srcX0; sx < srcX1; ++sx) {
                    sum += fullResPlane.at(sx, sy, 0);
                    ++count;
                }
            }
            out.at(ox, oy, 0) = static_cast<std::uint8_t>(sum / count);
        }
    }
    return out;
}

ImageBuffer<std::uint8_t> ChromaSubsampler::upsample(
    const ImageBuffer<std::uint8_t>& subsampledPlane, std::size_t fullWidth,
    std::size_t fullHeight, ChromaSubsamplingMode mode) {
    if (subsampledPlane.channels() != 1) {
        throw std::invalid_argument("ChromaSubsampler::upsample: plane must have 1 channel");
    }

    const BlockSize block = blockSizeFor(mode);
    ImageBuffer<std::uint8_t> out(fullWidth, fullHeight, 1);
    for (std::size_t y = 0; y < fullHeight; ++y) {
        const std::size_t sy = std::min(y / block.height, subsampledPlane.height() - 1);
        for (std::size_t x = 0; x < fullWidth; ++x) {
            const std::size_t sx = std::min(x / block.width, subsampledPlane.width() - 1);
            out.at(x, y, 0) = subsampledPlane.at(sx, sy, 0);
        }
    }
    return out;
}

} // namespace ivcv::core
