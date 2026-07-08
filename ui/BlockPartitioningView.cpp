#include "BlockPartitioningView.h"

#include "BlockGridPanel.h"

#include <QComboBox>
#include <QDoubleSpinBox>
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

#include "ivcv/core/Block.h"
#include "ivcv/core/ChromaSubsampler.h"
#include "ivcv/core/ImageLoader.h"

namespace ivcv::ui {

namespace {

constexpr int kPreviewMaxSize = 320;

/// Converts a single-channel plane to a grayscale-as-RGB QImage for
/// display. Mirrors the identical helper pattern in ChromaSubsamplingView
/// and RgbYuvView; kept as a small local duplicate rather than a shared
/// header so each module view remains a self-contained, independently
/// readable unit (see docs/ARCHITECTURE.md, section 5).
QImage planeToQImage(const core::ImageBuffer<std::uint8_t>& plane) {
    QImage image(
        static_cast<int>(plane.width()), static_cast<int>(plane.height()),
        QImage::Format_RGB888);
    for (std::size_t y = 0; y < plane.height(); ++y) {
        auto* line = image.scanLine(static_cast<int>(y));
        for (std::size_t x = 0; x < plane.width(); ++x) {
            const auto value = plane.at(x, y, 0);
            line[x * 3 + 0] = value;
            line[x * 3 + 1] = value;
            line[x * 3 + 2] = value;
        }
    }
    return image;
}

/// Converts a 3-channel RGB ImageBuffer (here, the variance heatmap) to a
/// QImage for display.
QImage rgbBufferToQImage(const core::ImageBuffer<std::uint8_t>& rgb) {
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

/// Builds a red/blue heatmap the same size as the partitioned plane,
/// coloring every pixel within a block by that block's variance (clamped
/// to a fixed display range) rather than by its own intensity, so flat
/// vs. detailed regions -- and therefore the partitioner's split
/// decisions -- are visually obvious at a glance.
core::ImageBuffer<std::uint8_t> varianceHeatmap(
    std::size_t planeWidth, std::size_t planeHeight, const std::vector<core::Block>& blocks) {
    constexpr double kMaxDisplayedVariance = 4000.0;
    core::ImageBuffer<std::uint8_t> heatmap(planeWidth, planeHeight, 3);
    for (const auto& block : blocks) {
        const double normalized = std::clamp(block.variance / kMaxDisplayedVariance, 0.0, 1.0);
        const auto intensity = static_cast<std::uint8_t>(normalized * 255.0);
        for (std::size_t y = block.y; y < block.y + block.height; ++y) {
            for (std::size_t x = block.x; x < block.x + block.width; ++x) {
                heatmap.at(x, y, 0) = intensity;
                heatmap.at(x, y, 1) = 0;
                heatmap.at(x, y, 2) = static_cast<std::uint8_t>(255 - intensity);
            }
        }
    }
    return heatmap;
}

} // namespace

BlockPartitioningView::BlockPartitioningView(QWidget* parent) : QWidget(parent) {
    auto* layout = new QGridLayout(this);

    originalPanel_ = new QLabel(QStringLiteral("No image loaded."), this);
    originalPanel_->setAlignment(Qt::AlignCenter);
    originalPanel_->setMinimumSize(kPreviewMaxSize, kPreviewMaxSize);
    originalPanel_->setFrameShape(QFrame::Box);

    gridPanel_ = new BlockGridPanel(this);
    connect(
        gridPanel_, &BlockGridPanel::blockClicked, this, &BlockPartitioningView::onBlockClicked);

    heatmapPanel_ = new QLabel(QStringLiteral("Variance heatmap"), this);
    heatmapPanel_->setAlignment(Qt::AlignCenter);
    heatmapPanel_->setMinimumSize(kPreviewMaxSize, kPreviewMaxSize);
    heatmapPanel_->setFrameShape(QFrame::Box);

    auto* controlsPanel = new QWidget(this);
    auto* controlsLayout = new QVBoxLayout(controlsPanel);

    loadButton_ = new QPushButton(QStringLiteral("Load Image..."), controlsPanel);
    connect(loadButton_, &QPushButton::clicked, this, &BlockPartitioningView::onLoadImageClicked);

    maxBlockSizeSelector_ = new QComboBox(controlsPanel);
    maxBlockSizeSelector_->addItem(QStringLiteral("32 x 32"));
    maxBlockSizeSelector_->addItem(QStringLiteral("64 x 64"));
    maxBlockSizeSelector_->setCurrentIndex(1);
    connect(
        maxBlockSizeSelector_, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
        &BlockPartitioningView::onMaxBlockSizeChanged);

    minBlockSizeSelector_ = new QComboBox(controlsPanel);
    minBlockSizeSelector_->addItem(QStringLiteral("4 x 4"));
    minBlockSizeSelector_->addItem(QStringLiteral("8 x 8"));
    minBlockSizeSelector_->addItem(QStringLiteral("16 x 16"));
    minBlockSizeSelector_->setCurrentIndex(1);
    connect(
        minBlockSizeSelector_, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
        &BlockPartitioningView::onMinBlockSizeChanged);

    varianceThresholdSpinBox_ = new QDoubleSpinBox(controlsPanel);
    varianceThresholdSpinBox_->setRange(0.0, 10000.0);
    varianceThresholdSpinBox_->setDecimals(1);
    varianceThresholdSpinBox_->setSingleStep(10.0);
    varianceThresholdSpinBox_->setValue(50.0);
    connect(
        varianceThresholdSpinBox_, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
        &BlockPartitioningView::onVarianceThresholdChanged);

    statusLabel_ = new QLabel(QStringLiteral("Load an image to begin."), controlsPanel);
    statusLabel_->setWordWrap(true);

    selectedBlockLabel_ =
        new QLabel(QStringLiteral("Click a block to inspect it."), controlsPanel);
    selectedBlockLabel_->setWordWrap(true);

    controlsLayout->addWidget(loadButton_);
    controlsLayout->addWidget(new QLabel(QStringLiteral("Max block size:"), controlsPanel));
    controlsLayout->addWidget(maxBlockSizeSelector_);
    controlsLayout->addWidget(new QLabel(QStringLiteral("Min block size:"), controlsPanel));
    controlsLayout->addWidget(minBlockSizeSelector_);
    controlsLayout->addWidget(new QLabel(QStringLiteral("Variance threshold:"), controlsPanel));
    controlsLayout->addWidget(varianceThresholdSpinBox_);
    controlsLayout->addWidget(statusLabel_);
    controlsLayout->addWidget(selectedBlockLabel_);
    controlsLayout->addStretch();

    // Layout follows the shared module-view convention (see class doc
    // comment in BlockPartitioningView.h): input top-left, transformed
    // output top-right, secondary visualization bottom-left, controls
    // bottom-right.
    layout->addWidget(originalPanel_, 0, 0);
    layout->addWidget(gridPanel_, 0, 1);
    layout->addWidget(heatmapPanel_, 1, 0);
    layout->addWidget(controlsPanel, 1, 1);
}

BlockPartitioningView::~BlockPartitioningView() = default;

void BlockPartitioningView::onLoadImageClicked() {
    const QString path = QFileDialog::getOpenFileName(
        this, QStringLiteral("Load Image"), QString(),
        QStringLiteral("Images (*.png *.jpg *.jpeg *.bmp)"));
    if (path.isEmpty()) {
        return;
    }

    try {
        rgbImage_ = core::ImageLoader::loadRgb(path.toStdString());
    } catch (const std::exception& ex) {
        QMessageBox::warning(
            this, QStringLiteral("Failed to Load Image"), QString::fromStdString(ex.what()));
        return;
    }

    refreshPartitioning();
}

void BlockPartitioningView::onMaxBlockSizeChanged(int index) {
    partitioner_.setMaxBlockSize(index == 0 ? 32 : 64);
    refreshPartitioning();
}

void BlockPartitioningView::onMinBlockSizeChanged(int index) {
    switch (index) {
        case 0:
            partitioner_.setMinBlockSize(4);
            break;
        case 2:
            partitioner_.setMinBlockSize(16);
            break;
        default:
            partitioner_.setMinBlockSize(8);
            break;
    }
    refreshPartitioning();
}

void BlockPartitioningView::onVarianceThresholdChanged(double value) {
    partitioner_.setVarianceThreshold(value);
    refreshPartitioning();
}

void BlockPartitioningView::onBlockClicked(const core::Block& block) {
    selectedBlockLabel_->setText(
        QStringLiteral("Block #%1\nPosition: (%2, %3)\nSize: %4 x %5\nVariance: %6")
            .arg(static_cast<int>(block.id))
            .arg(static_cast<int>(block.x))
            .arg(static_cast<int>(block.y))
            .arg(static_cast<int>(block.width))
            .arg(static_cast<int>(block.height))
            .arg(block.variance, 0, 'f', 2));
}

void BlockPartitioningView::refreshPartitioning() {
    if (rgbImage_.empty()) {
        return;
    }

    const auto yuv = converter_.convertRgbToYuv(rgbImage_);
    const auto yPlane = core::ChromaSubsampler::extractPlane(yuv, 0);

    const auto blocks = core::BlockPartitioner::partition(
        yPlane, partitioner_.maxBlockSize(), partitioner_.minBlockSize(),
        partitioner_.varianceThreshold());

    const QImage lumaImage = planeToQImage(yPlane);
    originalPanel_->setPixmap(toScaledPixmap(lumaImage));

    gridPanel_->setImage(lumaImage);
    gridPanel_->setBlocks(blocks, yPlane.width(), yPlane.height());

    const auto heatmap = varianceHeatmap(yPlane.width(), yPlane.height(), blocks);
    heatmapPanel_->setPixmap(toScaledPixmap(rgbBufferToQImage(heatmap)));

    statusLabel_->setText(QStringLiteral("Image: %1x%2  |  Blocks: %3")
                               .arg(static_cast<int>(yPlane.width()))
                               .arg(static_cast<int>(yPlane.height()))
                               .arg(static_cast<int>(blocks.size())));
}

} // namespace ivcv::ui
