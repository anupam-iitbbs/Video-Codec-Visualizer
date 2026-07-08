#include <QApplication>

#include "MainWindow.h"

/// Application entry point. Constructs the Qt application object and shows
/// the main docked window shell (see docs/ARCHITECTURE.md, section 5).
int main(int argc, char** argv) {
    QApplication application(argc, argv);
    application.setApplicationName(QStringLiteral("Interactive Video Codec Visualizer"));
    application.setOrganizationName(QStringLiteral("ivcv"));

    ivcv::app::MainWindow window;
    window.show();

    return QApplication::exec();
}
