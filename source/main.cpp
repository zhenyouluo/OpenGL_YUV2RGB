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
    // trying different swap behaviours
//    fmt.setSwapBehavior(QSurfaceFormat::SingleBuffer); // only gives me a completely blacked out application
    fmt.setSwapBehavior(QSurfaceFormat::DoubleBuffer); // the default
//    fmt.setSwapBehavior(QSurfaceFormat::TripleBuffer); // same as double buffer for me. how to tell it's actually working?
    // swap interval of 0 turns of vsync which might lead to tearing, default is 1
    fmt.setSwapInterval(0); // no effect in windows
    fmt.setProfile(QSurfaceFormat::CoreProfile);

    QSurfaceFormat::setDefaultFormat(fmt);
    qDebug() << "Using OpenGL version " << fmt.version() << fmt.profile(); // << fmt.version().second;


    MainWindow w;
    w.show();

    return a.exec();
}
