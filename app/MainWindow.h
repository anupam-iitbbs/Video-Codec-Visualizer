#pragma once

#include <QMainWindow>

QT_BEGIN_NAMESPACE
class QListWidget;
class QLabel;
class QStackedWidget;
QT_END_NAMESPACE

namespace ivcv::ui {
class RgbYuvView;
class ChromaSubsamplingView;
}

namespace ivcv::app {

/// Top-level application window.
///
/// Stage 1 established the docked shell layout (pipeline stage list,
/// central canvas, parameter/metrics docks) described in
/// docs/ARCHITECTURE.md, section 5, with a placeholder central canvas.
/// Stage 2 wired in the first real module view: selecting "RGB to YUV" in
/// the stage list swaps the central canvas to ivcv::ui::RgbYuvView. Stage
/// 3 adds a second one: selecting "Chroma Subsampling" swaps to
/// ivcv::ui::ChromaSubsamplingView. Every other stage still shows the
/// placeholder until its own module view is implemented, so the shell
/// continues to work end-to-end at every stage.
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private slots:
    /// Swaps the central canvas to the module view matching the newly
    /// selected pipeline stage, or back to the placeholder if that
    /// stage's view has not been implemented yet.
    void onStageSelectionChanged(int row);

private:
    /// Creates the "Pipeline Stages" dock and populates it with the
    /// still-image pipeline stage names from docs/SRS.md.
    void setupDockPanels();

    /// Creates the central QStackedWidget holding the placeholder canvas
    /// and every implemented module view.
    void setupCentralCanvas();

    QListWidget* stageListWidget_ = nullptr;
    QStackedWidget* centralStack_ = nullptr;
    QLabel* canvasPlaceholder_ = nullptr;
    ivcv::ui::RgbYuvView* rgbYuvView_ = nullptr;
    ivcv::ui::ChromaSubsamplingView* chromaSubsamplingView_ = nullptr;
};

} // namespace ivcv::app
