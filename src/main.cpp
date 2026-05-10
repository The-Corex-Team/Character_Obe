#include <QApplication>
#include "MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication::setHighDpiScaleFactorRoundingPolicy(
        Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);

    QApplication app(argc, argv);
    app.setApplicationName("Character Obe");
    app.setApplicationVersion("1.0");
    app.setOrganizationName("Obe Office");

    MainWindow window;
    window.show();

    return app.exec();
}
