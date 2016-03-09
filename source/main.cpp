#include "mainwindow.h"
#include <QApplication>
#include <QSurfaceFormat>
#include <qdebug.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);


    QSurfaceFormat fmt(QSurfaceFormat::DeprecatedFunctions);
    fmt.setDepthBufferSize(24);
    fmt.setSamples(4);
    fmt.setVersion(3, 3);
    fmt.setProfile(QSurfaceFormat::CoreProfile);

    QSurfaceFormat::setDefaultFormat(fmt);
    qDebug() << "Using OpenGL version " << fmt.version() << fmt.profile(); // << fmt.version().second;


    MainWindow w;
    w.show();

    return a.exec();
}
