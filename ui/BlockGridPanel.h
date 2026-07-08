#pragma once

#include <QImage>
#include <QRect>
#include <QSize>
#include <QWidget>

#include <cstddef>
#include <vector>

#include "ivcv/core/Block.h"

QT_BEGIN_NAMESPACE
class QMouseEvent;
class QPaintEvent;
QT_END_NAMESPACE

namespace ivcv::ui {

/// Displays a grayscale image with a BlockPartitioner quadtree drawn as an
/// overlay grid, and reports which block the user clicked so a host view
/// (BlockPartitioningView) can show that block's id, coordinates, and
/// variance -- FR-4's "clickable blocks showing ID, coordinates,
/// variance" requirement in docs/SRS.md.
///
/// This widget owns no partitioning logic itself: it only renders a
/// QImage and a std::vector<core::Block> supplied by its owner, keeping
/// the actual algorithm in core::BlockPartitioner per the layering rule
/// in docs/ARCHITECTURE.md, section 1. It is kept as its own small,
/// reusable widget (rather than inline in BlockPartitioningView) because
/// it needs custom paintEvent()/mousePressEvent() overrides that a plain
/// QLabel cannot provide.
class BlockGridPanel : public QWidget {
    Q_OBJECT

public:
    explicit BlockGridPanel(QWidget* parent = nullptr);

    /// Replaces the displayed image. The image is drawn scaled to fit the
    /// widget while preserving aspect ratio, letterboxed and centered.
    void setImage(const QImage& image);

    /// Replaces the block grid drawn over the image. planeWidth/
    /// planeHeight must match the dimensions the blocks' coordinates were
    /// computed against (i.e. the plane passed to
    /// BlockPartitioner::partition()).
    void setBlocks(
        const std::vector<core::Block>& blocks, std::size_t planeWidth,
        std::size_t planeHeight);

    [[nodiscard]] QSize sizeHint() const override;

signals:
    /// Emitted when the user clicks inside a block's boundary.
    void blockClicked(const ivcv::core::Block& block);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

private:
    /// The rectangle (in widget coordinates) the image is actually drawn
    /// into, preserving aspect ratio and centered within the widget.
    [[nodiscard]] QRect imageDrawRect() const;

    QImage image_;
    std::vector<core::Block> blocks_;
    std::size_t planeWidth_ = 0;
    std::size_t planeHeight_ = 0;
};

} // namespace ivcv::ui
