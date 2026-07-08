#pragma once

#include <QWidget>

#include "ivcv/core/ColorSpaceConverter.h"
#include "ivcv/core/ImageBuffer.h"

QT_BEGIN_NAMESPACE
class QLabel;
class QComboBox;
class QPushButton;
QT_END_NAMESPACE

namespace ivcv::ui {

/// Module view for Stage 2 (RGB to YUV).
///
/// Lets the user load an image, pick a conversion standard (BT.601 or
/// BT.709), and see the original RGB image alongside a visualization of
/// the resulting Y' (luma) and Cb/Cr (chroma) planes.
///
/// Layout follows the shared module-view convention described in
/// docs/ARCHITECTURE.md, section 5: input top-left, transformed output
/// top-right, secondary visualization bottom-left, numeric/interactive
/// controls bottom-right. See modules/color_space/README.md for the
/// underlying algorithm's intuitive explanation and math derivation.
///
/// This view owns no algorithmic logic itself; it only loads data via
/// ImageLoader and delegates the actual conversion to
/// ivcv::core::ColorSpaceConverter, keeping the Qt/UI layer a thin
/// presentation shell over the core/ algorithm library (see
/// docs/ARCHITECTURE.md, section 1: the layering rule).
class RgbYuvView : public QWidget {
    Q_OBJECT

public:
    explicit RgbYuvView(QWidget* parent = nullptr);
    ~RgbYuvView() override;

private slots:
    /// Opens a file dialog, loads the chosen image via ImageLoader, and
    /// refreshes every panel.
    void onLoadImageClicked();

    /// Re-runs the conversion with the newly selected standard and
    /// refreshes the output panels, without reloading the source image.
    void onStandardChanged(int index);

private:
    /// Runs ColorSpaceConverter on the currently loaded RGB image (if any)
    /// and refreshes the luma/chroma panels and status label. No-op if no
    /// image has been loaded yet.
    void refreshConversion();

    /// Renders rgbImage_ into the top-left panel.
    void updateRgbPanel();

    core::ImageBuffer<std::uint8_t> rgbImage_;
    core::ColorSpaceConverter converter_;

    QLabel* rgbPanel_ = nullptr;
    QLabel* lumaPanel_ = nullptr;
    QLabel* chromaPanel_ = nullptr;
    QLabel* statusLabel_ = nullptr;
    QComboBox* standardSelector_ = nullptr;
    QPushButton* loadButton_ = nullptr;
};

}  // namespace ivcv::ui
