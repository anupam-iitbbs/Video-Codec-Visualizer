#pragma once

#include <QMainWindow>

QT_BEGIN_NAMESPACE
class QListWidget;
class QLabel;
QT_END_NAMESPACE

namespace ivcv::app {

/// Top-level application window.
///
/// Stage 1 scope: establishes the docked shell layout (pipeline stage list,
/// central canvas, parameter/metrics docks) described in
/// docs/ARCHITECTURE.md, section 5, with placeholder panels. Real module
/// views are wired in starting with Stage 2 (RGB to YUV).
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private:
    /// Creates the "Pipeline Stages" dock and populates it with the
    /// still-image pipeline stage names from docs/SRS.md.
    void setupDockPanels();

    /// Creates the central placeholder canvas shown before any module view
    /// exists.
    void setupCentralCanvas();

    QListWidget* stageListWidget_ = nullptr;
    QLabel* canvasPlaceholder_ = nullptr;
};

}  // namespace ivcv::app
