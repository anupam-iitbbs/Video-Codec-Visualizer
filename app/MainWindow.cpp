#include "MainWindow.h"

#include <QDockWidget>
#include <QLabel>
#include <QListWidget>
#include <QStackedWidget>

#include "ChromaSubsamplingView.h"
#include "RgbYuvView.h"

namespace ivcv::app {

namespace {
/// Index of "RGB to YUV" and "Chroma Subsampling" within the stage list
/// populated by setupDockPanels(). Kept as single named constants so the
/// two places that care about stage order (the list contents and the
/// view-swapping logic) cannot silently drift apart.
constexpr int kRgbToYuvStageRow = 1;
constexpr int kChromaSubsamplingStageRow = 2;
} // namespace

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle(QStringLiteral("Interactive Video Codec Visualizer"));
    resize(1280, 800);

    setupCentralCanvas();
    setupDockPanels();
}

MainWindow::~MainWindow() = default;

void MainWindow::setupCentralCanvas() {
    centralStack_ = new QStackedWidget(this);

    canvasPlaceholder_ = new QLabel(
        QStringLiteral(
            "Select a pipeline stage to begin.

"
            "Module views appear here as each stage is implemented."),
        centralStack_);
    canvasPlaceholder_->setAlignment(Qt::AlignCenter);

    rgbYuvView_ = new ivcv::ui::RgbYuvView(centralStack_);
    chromaSubsamplingView_ = new ivcv::ui::ChromaSubsamplingView(centralStack_);

    centralStack_->addWidget(canvasPlaceholder_); // index 0: placeholder
    centralStack_->addWidget(rgbYuvView_); // index 1: RGB to YUV
    centralStack_->addWidget(chromaSubsamplingView_); // index 2: Chroma Subsampling

    setCentralWidget(centralStack_);
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
    connect(stageListWidget_, &QListWidget::currentRowChanged, this,
        &MainWindow::onStageSelectionChanged);

    stageDock->setWidget(stageListWidget_);
    addDockWidget(Qt::LeftDockWidgetArea, stageDock);
}

void MainWindow::onStageSelectionChanged(int row) {
    if (row == kRgbToYuvStageRow) {
        centralStack_->setCurrentWidget(rgbYuvView_);
    } else if (row == kChromaSubsamplingStageRow) {
        centralStack_->setCurrentWidget(chromaSubsamplingView_);
    } else {
        centralStack_->setCurrentWidget(canvasPlaceholder_);
    }
}

} // namespace ivcv::app
