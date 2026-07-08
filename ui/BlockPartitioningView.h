#pragma once

#include <QWidget>

#include "ivcv/core/BlockPartitioner.h"
#include "ivcv/core/ColorSpaceConverter.h"
#include "ivcv/core/ImageBuffer.h"

QT_BEGIN_NAMESPACE
class QLabel;
class QComboBox;
class QDoubleSpinBox;
class QPushButton;
QT_END_NAMESPACE

namespace ivcv::core {
struct Block;
} // namespace ivcv::core

namespace ivcv::ui {

class BlockGridPanel;

/// Module view for Stage 4 (Block Partitioning).
///
/// Lets the user load an image, tune maxBlockSize/minBlockSize/variance
/// threshold, and see the resulting quadtree drawn over the luma plane as
/// a clickable grid: clicking any block reports its id, coordinates, and
/// variance (FR-4's "clickable blocks showing ID, coordinates, variance"
/// requirement in docs/SRS.md), alongside a variance heatmap that colors
/// every block by how much detail it contains.
///
/// Layout follows the shared module-view convention described in
/// docs/ARCHITECTURE.md, section 5: input top-left, transformed output
/// (the clickable grid) top-right, secondary visualization (the variance
/// heatmap) bottom-left, numeric/interactive controls bottom-right. See
/// modules/block_partitioning/README.md for the underlying algorithm's
/// intuitive explanation and math derivation.
///
/// This view owns no algorithmic logic itself; it only loads data via
/// ImageLoader/ColorSpaceConverter and delegates the actual partitioning
/// to ivcv::core::BlockPartitioner, keeping the Qt/UI layer a thin
/// presentation shell over the core/ algorithm library (see
/// docs/ARCHITECTURE.md, section 1: the layering rule).
class BlockPartitioningView : public QWidget {
    Q_OBJECT

public:
    explicit BlockPartitioningView(QWidget* parent = nullptr);
    ~BlockPartitioningView() override;

private slots:
    /// Opens a file dialog, loads the chosen image via ImageLoader, and
    /// refreshes every panel.
    void onLoadImageClicked();

    /// Re-runs partitioning with the newly selected maximum block size,
    /// without reloading the source image.
    void onMaxBlockSizeChanged(int index);

    /// Re-runs partitioning with the newly selected minimum block size,
    /// without reloading the source image.
    void onMinBlockSizeChanged(int index);

    /// Re-runs partitioning with the newly entered variance threshold,
    /// without reloading the source image.
    void onVarianceThresholdChanged(double value);

    /// Updates the selected-block label with the clicked block's details.
    void onBlockClicked(const ivcv::core::Block& block);

private:
    /// Runs BlockPartitioner on the currently loaded image (if any) at
    /// the active parameters and refreshes the grid panel, the heatmap,
    /// and the status label. No-op if no image has been loaded yet.
    void refreshPartitioning();

    core::ImageBuffer<std::uint8_t> rgbImage_;
    core::ColorSpaceConverter converter_;
    core::BlockPartitioner partitioner_;

    QLabel* originalPanel_ = nullptr;
    BlockGridPanel* gridPanel_ = nullptr;
    QLabel* heatmapPanel_ = nullptr;
    QLabel* statusLabel_ = nullptr;
    QLabel* selectedBlockLabel_ = nullptr;
    QComboBox* maxBlockSizeSelector_ = nullptr;
    QComboBox* minBlockSizeSelector_ = nullptr;
    QDoubleSpinBox* varianceThresholdSpinBox_ = nullptr;
    QPushButton* loadButton_ = nullptr;
};

} // namespace ivcv::ui
