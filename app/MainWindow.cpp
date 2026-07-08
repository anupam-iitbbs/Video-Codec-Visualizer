#include "MainWindow.h"

#include <QDockWidget>
#include <QLabel>
#include <QListWidget>

namespace ivcv::app {

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle(QStringLiteral("Interactive Video Codec Visualizer"));
    resize(1280, 800);

    setupCentralCanvas();
    setupDockPanels();
}

MainWindow::~MainWindow() = default;

void MainWindow::setupCentralCanvas() {
    canvasPlaceholder_ = new QLabel(
        QStringLiteral(
            "Select a pipeline stage to begin.\n\n"
            "Module views will appear here starting with Stage 2 (RGB to YUV)."),
        this);
    canvasPlaceholder_->setAlignment(Qt::AlignCenter);
    setCentralWidget(canvasPlaceholder_);
}

void MainWindow::setupDockPanels() {
    auto* stageDock = new QDockWidget(QStringLiteral("Pipeline Stages"), this);

    stageListWidget_ = new QListWidget(stageDock);
    stageListWidget_->addItems({
        QStringLiteral("Input"),
        QStringLiteral("RGB to YUV"),
        QStringLiteral("Chroma Subsampling"),
        QStringLiteral("Block Partitioning"),
        QStringLiteral("DCT"),
        QStringLiteral("Quantization"),
        QStringLiteral("Zig-zag Scan"),
        QStringLiteral("Run-Length Encoding"),
        QStringLiteral("Entropy Coding"),
        QStringLiteral("Quality Metrics"),
    });

    stageDock->setWidget(stageListWidget_);
    addDockWidget(Qt::LeftDockWidgetArea, stageDock);
}

}  // namespace ivcv::app
