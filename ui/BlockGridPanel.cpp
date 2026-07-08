#include "BlockGridPanel.h"

#include <QColor>
#include <QMouseEvent>
#include <QPainter>
#include <QPaintEvent>
#include <QPen>

#include <algorithm>

namespace ivcv::ui {

namespace {
constexpr int kMinPanelSize = 320;
} // namespace

BlockGridPanel::BlockGridPanel(QWidget* parent) : QWidget(parent) {
    setMinimumSize(kMinPanelSize, kMinPanelSize);
}

void BlockGridPanel::setImage(const QImage& image) {
    image_ = image;
    update();
}

void BlockGridPanel::setBlocks(
    const std::vector<core::Block>& blocks, std::size_t planeWidth, std::size_t planeHeight) {
    blocks_ = blocks;
    planeWidth_ = planeWidth;
    planeHeight_ = planeHeight;
    update();
}

QSize BlockGridPanel::sizeHint() const {
    return QSize(kMinPanelSize, kMinPanelSize);
}

QRect BlockGridPanel::imageDrawRect() const {
    if (image_.isNull()) {
        return rect();
    }
    const QSize scaled = image_.size().scaled(size(), Qt::KeepAspectRatio);
    const int x = (width() - scaled.width()) / 2;
    const int y = (height() - scaled.height()) / 2;
    return QRect(QPoint(x, y), scaled);
}

void BlockGridPanel::paintEvent(QPaintEvent* /*event*/) {
    QPainter painter(this);
    painter.fillRect(rect(), Qt::black);

    if (image_.isNull()) {
        painter.setPen(Qt::white);
        painter.drawText(rect(), Qt::AlignCenter, QStringLiteral("No image loaded."));
        return;
    }

    const QRect target = imageDrawRect();
    painter.drawImage(target, image_);

    if (planeWidth_ == 0 || planeHeight_ == 0) {
        return;
    }

    const double scaleX = static_cast<double>(target.width()) / static_cast<double>(planeWidth_);
    const double scaleY =
        static_cast<double>(target.height()) / static_cast<double>(planeHeight_);

    painter.setPen(QPen(QColor(0, 255, 0, 180), 1));
    for (const auto& block : blocks_) {
        const int blockX = target.x() + static_cast<int>(block.x * scaleX);
        const int blockY = target.y() + static_cast<int>(block.y * scaleY);
        const int blockWidth = std::max(1, static_cast<int>(block.width * scaleX));
        const int blockHeight = std::max(1, static_cast<int>(block.height * scaleY));
        painter.drawRect(blockX, blockY, blockWidth, blockHeight);
    }
}

void BlockGridPanel::mousePressEvent(QMouseEvent* event) {
    if (planeWidth_ == 0 || planeHeight_ == 0 || blocks_.empty()) {
        return;
    }

    const QRect target = imageDrawRect();
    if (!target.contains(event->pos())) {
        return;
    }

    const double scaleX = static_cast<double>(planeWidth_) / static_cast<double>(target.width());
    const double scaleY =
        static_cast<double>(planeHeight_) / static_cast<double>(target.height());
    const auto planeX = static_cast<std::size_t>((event->pos().x() - target.x()) * scaleX);
    const auto planeY = static_cast<std::size_t>((event->pos().y() - target.y()) * scaleY);

    for (const auto& block : blocks_) {
        if (planeX >= block.x && planeX < block.x + block.width && planeY >= block.y &&
            planeY < block.y + block.height) {
            emit blockClicked(block);
            return;
        }
    }
}

} // namespace ivcv::ui
