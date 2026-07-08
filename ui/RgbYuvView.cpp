#include "RgbYuvView.h"

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

#include <cstdint>
#include <exception>

#include "ivcv/core/ImageLoader.h"

namespace ivcv::ui {

namespace {

constexpr int kPreviewMaxSize = 320;

/// Converts a 3-channel RGB ImageBuffer to a QImage for display. Copies
/// pixel-by-pixel rather than assuming a matching memory layout, so this
/// stays correct even if ImageBuffer's internal storage changes later.
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

/// Builds a grayscale-as-RGB image from the Y' channel of a Y'CbCr buffer,
/// so the luma plane can be displayed with the same toQImage() helper.
core::ImageBuffer<std::uint8_t> lumaOnly(const core::ImageBuffer<std::uint8_t>& yuv) {
    core::ImageBuffer<std::uint8_t> gray(yuv.width(), yuv.height(), 3);
    for (std::size_t y = 0; y < yuv.height(); ++y) {
        for (std::size_t x = 0; x < yuv.width(); ++x) {
            const std::uint8_t luma = yuv.at(x, y, 0);
            gray.at(x, y, 0) = luma;
            gray.at(x, y, 1) = luma;
            gray.at(x, y, 2) = luma;
        }
    }
    return gray;
}

/// Builds an RGB visualization of chroma alone by fixing luma at a neutral
/// mid-gray (128) and converting the real Cb/Cr back to RGB. This lets a
/// learner literally see "what color information alone looks like",
/// which is the pedagogical point of separating luma from chroma.
core::ImageBuffer<std::uint8_t> chromaOnlyRgb(
    const core::ImageBuffer<std::uint8_t>& yuv, const core::ColorSpaceConverter& converter) {
    core::ImageBuffer<std::uint8_t> chromaYuv(yuv.width(), yuv.height(), 3);
    for (std::size_t y = 0; y < yuv.height(); ++y) {
        for (std::size_t x = 0; x < yuv.width(); ++x) {
            chromaYuv.at(x, y, 0) = 128;
            chromaYuv.at(x, y, 1) = yuv.at(x, y, 1);
            chromaYuv.at(x, y, 2) = yuv.at(x, y, 2);
        }
    }
    return converter.convertYuvToRgb(chromaYuv);
}

QPixmap toScaledPixmap(const QImage& image) {
    return QPixmap::fromImage(image).scaled(
        kPreviewMaxSize, kPreviewMaxSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

}  // namespace

RgbYuvView::RgbYuvView(QWidget* parent) : QWidget(parent) {
    auto* layout = new QGridLayout(this);

    rgbPanel_ = new QLabel(QStringLiteral("No image loaded."), this);
    rgbPanel_->setAlignment(Qt::AlignCenter);
    rgbPanel_->setMinimumSize(kPreviewMaxSize, kPreviewMaxSize);
    rgbPanel_->setFrameShape(QFrame::Box);

    lumaPanel_ = new QLabel(QStringLiteral("Y' (luma)"), this);
    lumaPanel_->setAlignment(Qt::AlignCenter);
    lumaPanel_->setMinimumSize(kPreviewMaxSize, kPreviewMaxSize);
    lumaPanel_->setFrameShape(QFrame::Box);

    chromaPanel_ = new QLabel(QStringLiteral("Cb/Cr (chroma)"), this);
    chromaPanel_->setAlignment(Qt::AlignCenter);
    chromaPanel_->setMinimumSize(kPreviewMaxSize, kPreviewMaxSize);
    chromaPanel_->setFrameShape(QFrame::Box);

    auto* controlsPanel = new QWidget(this);
    auto* controlsLayout = new QVBoxLayout(controlsPanel);

    loadButton_ = new QPushButton(QStringLiteral("Load Image..."), controlsPanel);
    connect(loadButton_, &QPushButton::clicked, this, &RgbYuvView::onLoadImageClicked);

    standardSelector_ = new QComboBox(controlsPanel);
    standardSelector_->addItem(QStringLiteral("BT.601 (SD)"));
    standardSelector_->addItem(QStringLiteral("BT.709 (HD)"));
    connect(standardSelector_, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &RgbYuvView::onStandardChanged);

    statusLabel_ = new QLabel(QStringLiteral("Load an image to begin."), controlsPanel);
    statusLabel_->setWordWrap(true);

    controlsLayout->addWidget(loadButton_);
    controlsLayout->addWidget(standardSelector_);
    controlsLayout->addWidget(statusLabel_);
    controlsLayout->addStretch();

    // Layout convention (docs/ARCHITECTURE.md, section 5): input top-left,
    // output top-right, secondary visualization bottom-left, controls
    // bottom-right.
    layout->addWidget(rgbPanel_, 0, 0);
    layout->addWidget(lumaPanel_, 0, 1);
    layout->addWidget(chromaPanel_, 1, 0);
    layout->addWidget(controlsPanel, 1, 1);
}

RgbYuvView::~RgbYuvView() = default;

void RgbYuvView::onLoadImageClicked() {
    const QString path = QFileDialog::getOpenFileName(
        this, QStringLiteral("Load Image"), QString(),
        QStringLiteral("Images (*.png *.jpg *.jpeg *.bmp)"));
    if (path.isEmpty()) {
        return;
    }

    try {
        rgbImage_ = core::ImageLoader::loadRgb(path.toStdString());
    } catch (const std::exception& error) {
        QMessageBox::warning(
            this, QStringLiteral("Failed to load image"), QString::fromStdString(error.what()));
        return;
    }

    updateRgbPanel();
    refreshConversion();
}

void RgbYuvView::onStandardChanged(int index) {
    converter_.setStandard(
        index == 1 ? core::ColorSpaceStandard::BT709 : core::ColorSpaceStandard::BT601);
    refreshConversion();
}

void RgbYuvView::updateRgbPanel() {
    if (rgbImage_.empty()) {
        rgbPanel_->setText(QStringLiteral("No image loaded."));
        return;
    }
    rgbPanel_->setPixmap(toScaledPixmap(toQImage(rgbImage_)));
}

void RgbYuvView::refreshConversion() {
    if (rgbImage_.empty()) {
        return;
    }

    const core::ImageBuffer<std::uint8_t> yuv = converter_.convertRgbToYuv(rgbImage_);

    lumaPanel_->setPixmap(toScaledPixmap(toQImage(lumaOnly(yuv))));
    chromaPanel_->setPixmap(toScaledPixmap(toQImage(chromaOnlyRgb(yuv, converter_))));

    statusLabel_->setText(QStringLiteral("%1x%2 image converted using %3.")
                               .arg(rgbImage_.width())
                               .arg(rgbImage_.height())
                               .arg(converter_.standard() == core::ColorSpaceStandard::BT601
                                        ? QStringLiteral("BT.601")
                                        : QStringLiteral("BT.709")));
}

}  // namespace ivcv::ui
