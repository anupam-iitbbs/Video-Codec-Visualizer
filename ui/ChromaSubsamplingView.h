#pragma once

#include <QWidget>

#include "ivcv/core/ChromaSubsampler.h"
#include "ivcv/core/ColorSpaceConverter.h"
#include "ivcv/core/ImageBuffer.h"

QT_BEGIN_NAMESPACE
class QLabel;
class QComboBox;
class QPushButton;
QT_END_NAMESPACE

namespace ivcv::ui {

/// Module view for Stage 3 (Chroma Subsampling).
///
/// Lets the user load an image, pick a subsampling mode (4:4:4/4:2:2/4:2:0)
/// and filter method (Nearest/Box), and see the full-resolution chroma
/// alongside the subsampled-then-upsampled chroma, plus a difference
/// heatmap and plane-size/storage-savings stats.
///
/// Layout follows the shared module-view convention described in
/// docs/ARCHITECTURE.md, section 5: input top-left, transformed output
/// top-right, secondary visualization bottom-left, numeric/interactive
/// controls bottom-right. See modules/chroma_subsampling/README.md for the
/// underlying algorithm's intuitive explanation and math derivation.
///
/// This view owns no algorithmic logic itself; it only loads data via
/// ImageLoader/ColorSpaceConverter and delegates the actual subsampling to
/// ivcv::core::ChromaSubsampler, keeping the Qt/UI layer a thin
/// presentation shell over the core/ algorithm library (see
/// docs/ARCHITECTURE.md, section 1: the layering rule).
class ChromaSubsamplingView : public QWidget {
    Q_OBJECT

public:
    explicit ChromaSubsamplingView(QWidget* parent = nullptr);
    ~ChromaSubsamplingView() override;

private slots:
    /// Opens a file dialog, loads the chosen image via ImageLoader, and
    /// refreshes every panel.
    void onLoadImageClicked();

    /// Re-runs subsampling with the newly selected mode, without
    /// reloading the source image.
    void onModeChanged(int index);

    /// Re-runs subsampling with the newly selected filter method, without
    /// reloading the source image.
    void onFilterChanged(int index);

private:
    /// Runs ChromaSubsampler on the currently loaded image (if any) at
    /// the active mode/filter and refreshes the chroma panels, the
    /// difference heatmap, and the stats label. No-op if no image has
    /// been loaded yet.
    void refreshSubsampling();

    core::ImageBuffer<std::uint8_t> rgbImage_;
    core::ColorSpaceConverter converter_;
    core::ChromaSubsampler subsampler_;

    QLabel* originalChromaPanel_ = nullptr;
    QLabel* subsampledChromaPanel_ = nullptr;
    QLabel* differencePanel_ = nullptr;
    QLabel* statusLabel_ = nullptr;
    QComboBox* modeSelector_ = nullptr;
    QComboBox* filterSelector_ = nullptr;
    QPushButton* loadButton_ = nullptr;
};

} // namespace ivcv::ui
