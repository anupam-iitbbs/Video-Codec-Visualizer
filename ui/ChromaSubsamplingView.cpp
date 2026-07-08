#include "ChromaSubsamplingView.h"

#include <QComboBox>
#include <QFileDialog>
#include <QFrame>
#include <QGridLayout>
#include <QImage>
#include <QLabel>
#include <QMessageBox>
#include <QPixmap>
#include <QPushButton>
#include <QVBoxLayout>

#include <algorithm>
#include <cstdint>
#include <exception>

#include "ivcv/core/ImageLoader.h"

namespace ivcv::ui {

namespace {

constexpr int kPreviewMaxSize = 320;

/// Converts a 3-channel RGB ImageBuffer to a QImage for display. Copies
/// pixel-by-pixel rather than assuming a matching memory layout, so this
/// stays correct even if ImageBuffer's internal storage changes later.
/// Mirrors the identical helper in RgbYuvView.cpp; kept as a small local
/// duplicate rather than a shared header so each module view remains a
/// self-contained, independently readable unit (see
/// docs/ARCHITECTURE.md, section 5).
QImage toQImage(const core::ImageBuffer<std::uint8_t>& rgb) {
    QImage image(
        static_cast<int>(rgb.width()), static_cast<int>(rgb.height()), QImage::Format_RGB888);
    for (std::size_t y = 0; y < rgb.height(); ++y) {
        auto* line = image.scanLine(static_cast<int>(y));
        for (std::size_t x = 0; x < rgb.width(); ++x) {
            line[x * 3 + 0] = rgb.at(x, y, 0);
            line[x * 3 + 1] = rgb.at(x, y, 1);
            line[x * 3 + 2] = rgb.at(x, y, 2);
        }
    }
    return image;
}

QPixmap toScaledPixmap(const QImage& image) {
    return QPixmap::fromImage(image).scaled(
        kPreviewMaxSize, kPreviewMaxSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

/// Builds an RGB visualization of chroma alone by fixing luma at a neutral
/// mid-gray (128) and converting the given full-resolution Cb/Cr planes
/// back to RGB, so a learner can see chroma information in isolation
/// regardless of whether it came straight from the source image or was
/// reconstructed after subsampling.
core::ImageBuffer<std::uint8_t> chromaPlanesToRgb(
    const core::ImageBuffer<std::uint8_t>& cb, const core::ImageBuffer<std::uint8_t>& cr,
    const core::ColorSpaceConverter& converter) {
    core::ImageBuffer<std::uint8_t> yuv(cb.width(), cb.height(), 3);
    for (std::size_t y = 0; y < cb.height(); ++y) {
        for (std::size_t x = 0; x < cb.width(); ++x) {
            yuv.at(x, y, 0) = 128;
            yuv.at(x, y, 1) = cb.at(x, y, 0);
            yuv.at(x, y, 2) = cr.at(x, y, 0);
        }
    }
    return converter.convertYuvToRgb(yuv);
}

/// Builds a grayscale-as-RGB heatmap of the per-pixel absolute difference
/// between two same-sized RGB images, summed across channels and clamped
/// to [0, 255]. Used to make the information lost to chroma subsampling
/// visually tangible rather than just described in text.
core::ImageBuffer<std::uint8_t> absDifferenceHeatmap(
    const core::ImageBuffer<std::uint8_t>& a, const core::ImageBuffer<std::uint8_t>& b) {
    core::ImageBuffer<std::uint8_t> heatmap(a.width(), a.height(), 3);
    for (std::size_t y = 0; y < a.height(); ++y) {
        for (std::size_t x = 0; x < a.width(); ++x) {
            int sum = 0;
            for (std::size_t c = 0; c < 3; ++c) {
                sum += std::abs(static_cast<int>(a.at(x, y, c)) - static_cast<int>(b.at(x, y, c)));
            }
            const auto value = static_cast<std::uint8_t>(std::clamp(sum, 0, 255));
            heatmap.at(x, y, 0) = value;
            heatmap.at(x, y, 1) = value;
            heatmap.at(x, y, 2) = value;
        }
    }
    return heatmap;
}

} // namespace

ChromaSubsamplingView::ChromaSubsamplingView(QWidget* parent)
    : QWidget(parent), subsampler_(core::ChromaSubsamplingMode::Yuv420, core::ChromaFilterMethod::Box) {
    auto* layout = new QGridLayout(this);

    originalChromaPanel_ = new QLabel(QStringLiteral("No image loaded."), this);
    originalChromaPanel_->setAlignment(Qt::AlignCenter);
    originalChromaPanel_->setMinimumSize(kPreviewMaxSize, kPreviewMaxSize);
    originalChromaPanel_->setFrameShape(QFrame::Box);

    subsampledChromaPanel_ = new QLabel(QStringLiteral("Subsampled chroma"), this);
    subsampledChromaPanel_->setAlignment(Qt::AlignCenter);
    subsampledChromaPanel_->setMinimumSize(kPreviewMaxSize, kPreviewMaxSize);
    subsampledChromaPanel_->setFrameShape(QFrame::Box);

    differencePanel_ = new QLabel(QStringLiteral("Difference"), this);
    differencePanel_->setAlignment(Qt::AlignCenter);
    differencePanel_->setMinimumSize(kPreviewMaxSize, kPreviewMaxSize);
    differencePanel_->setFrameShape(QFrame::Box);

    auto* controlsPanel = new QWidget(this);
    auto* controlsLayout = new QVBoxLayout(controlsPanel);

    loadButton_ = new QPushButton(QStringLiteral("Load Image..."), controlsPanel);
    connect(loadButton_, &QPushButton::clicked, this, &ChromaSubsamplingView::onLoadImageClicked);

    modeSelector_ = new QComboBox(controlsPanel);
    modeSelector_->addItem(QStringLiteral("4:4:4 (no subsampling)"));
    modeSelector_->addItem(QStringLiteral("4:2:2 (horizontal only)"));
    modeSelector_->addItem(QStringLiteral("4:2:0 (horizontal + vertical)"));
    modeSelector_->setCurrentIndex(2);
    connect(modeSelector_, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
        &ChromaSubsamplingView::onModeChanged);

    filterSelector_ = new QComboBox(controlsPanel);
    filterSelector_->addItem(QStringLiteral("Nearest"));
    filterSelector_->addItem(QStringLiteral("Box (Average)"));
    filterSelector_->setCurrentIndex(1);
    connect(filterSelector_, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
        &ChromaSubsamplingView::onFilterChanged);

    statusLabel_ = new QLabel(QStringLiteral("Load an image to begin."), controlsPanel);
    statusLabel_->setWordWrap(true);

    controlsLayout->addWidget(loadButton_);
    controlsLayout->addWidget(modeSelector_);
    controlsLayout->addWidget(filterSelector_);
    controlsLayout->addWidget(statusLabel_);
    controlsLayout->addStretch();

    // Layout convention (docs/ARCHITECTURE.md, section 5): input top-left,
    // output top-right, secondary visualization bottom-left, controls
    // bottom-right. Here "input" is the full-resolution (4:4:4) chroma
    // reference and "output" is the subsampled-then-upsampled chroma.
    layout->addWidget(originalChromaPanel_, 0, 0);
    layout->addWidget(subsampledChromaPanel_, 0, 1);
    layout->addWidget(differencePanel_, 1, 0);
    layout->addWidget(controlsPanel, 1, 1);
}

ChromaSubsamplingView::~ChromaSubsamplingView() = default;

void ChromaSubsamplingView::onLoadImageClicked() {
    const QString path = QFileDialog::getOpenFileName(
        this, QStringLiteral("Load Image"), QString(),
        QStringLiteral("Images (*.png *.jpg *.jpeg *.bmp)"));
    if (path.isEmpty()) {
        return;
    }

    try {
        rgbImage_ = core::ImageLoader::loadRgb(path.toStdString());
        statusLabel_->setText(QStringLiteral("Loaded: %1").arg(path));
        refreshSubsampling();
    } catch (const std::exception& ex) {
        QMessageBox::warning(
            this, QStringLiteral("Failed to load image"), QString::fromStdString(ex.what()));
    }
}

void ChromaSubsamplingView::onModeChanged(int index) {
    switch (index) {
        case 0:
            subsampler_.setMode(core::ChromaSubsamplingMode::Yuv444);
            break;
        case 1:
            subsampler_.setMode(core::ChromaSubsamplingMode::Yuv422);
            break;
        default:
            subsampler_.setMode(core::ChromaSubsamplingMode::Yuv420);
            break;
    }
    refreshSubsampling();
}

void ChromaSubsamplingView::onFilterChanged(int index) {
    subsampler_.setFilterMethod(
        index == 0 ? core::ChromaFilterMethod::Nearest : core::ChromaFilterMethod::Box);
    refreshSubsampling();
}

void ChromaSubsamplingView::refreshSubsampling() {
    if (rgbImage_.empty()) {
        return;
    }

    const auto yuvFull = converter_.convertRgbToYuv(rgbImage_);
    const std::size_t width = yuvFull.width();
    const std::size_t height = yuvFull.height();

    const auto cbFull = core::ChromaSubsampler::extractPlane(yuvFull, 1);
    const auto crFull = core::ChromaSubsampler::extractPlane(yuvFull, 2);

    const auto cbSub = core::ChromaSubsampler::downsample(
        cbFull, subsampler_.mode(), subsampler_.filterMethod());
    const auto crSub = core::ChromaSubsampler::downsample(
        crFull, subsampler_.mode(), subsampler_.filterMethod());

    const auto cbRecon = core::ChromaSubsampler::upsample(cbSub, width, height, subsampler_.mode());
    const auto crRecon = core::ChromaSubsampler::upsample(crSub, width, height, subsampler_.mode());

    const auto originalRgb = chromaPlanesToRgb(cbFull, crFull, converter_);
    const auto reconstructedRgb = chromaPlanesToRgb(cbRecon, crRecon, converter_);
    const auto diff = absDifferenceHeatmap(originalRgb, reconstructedRgb);

    originalChromaPanel_->setPixmap(toScaledPixmap(toQImage(originalRgb)));
    subsampledChromaPanel_->setPixmap(toScaledPixmap(toQImage(reconstructedRgb)));
    differencePanel_->setPixmap(toScaledPixmap(toQImage(diff)));

    const double fullSamples = 2.0 * static_cast<double>(width) * static_cast<double>(height);
    const double subSamples =
        2.0 * static_cast<double>(cbSub.width()) * static_cast<double>(cbSub.height());
    const double savingsPercent = fullSamples > 0.0 ? 100.0 * (1.0 - subSamples / fullSamples) : 0.0;

    statusLabel_->setText(QStringLiteral(
                               "Cb/Cr plane: %1x%2 (full-res %3x%4)\n"
                               "Chroma data reduced by %5%")
                               .arg(static_cast<int>(cbSub.width()))
                               .arg(static_cast<int>(cbSub.height()))
                               .arg(static_cast<int>(width))
                               .arg(static_cast<int>(height))
                               .arg(savingsPercent, 0, 'f', 1));
}

} // namespace ivcv::ui
