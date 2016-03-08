#-------------------------------------------------
#
# Project created by QtCreator 2015-11-03T10:35:07
#
#-------------------------------------------------

QT       += core gui

CONFIG += c++11

CONFIG(release, debug|release) {
    message("release mode")
    DEFINES += QT_NO_DEBUG_OUTPUT
} else {
    message("debug mode")
}


greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Avaso
TEMPLATE = app


INCLUDEPATH += source

LIBS +=

HEADERS  += \
    source/mainwindow.h \
    source/glwidget.h \    
    source/textureplusdepthsequencelistmodel.h \
    source/playbackcontroller.h \
    source/typedef.h \
    source/common.h \
    source/yuvfile.h \
    source/yuvsource.h

SOURCES += \
    source/main.cpp \
    source/mainwindow.cpp \
    source/glwidget.cpp \    
    source/textureplusdepthsequencelistmodel.cpp \
    source/playbackcontroller.cpp \
    source/common.cpp \
    source/yuvfile.cpp \
    source/yuvsource.cpp

FORMS    += forms/mainwindow.ui

OTHER_FILES += \
    shaders/fragmentYUV2RGBshader.glsl \
    shaders/vertexshader.glsl


RESOURCES += \
    resources.qrc

linux {
    QMAKE_CXXFLAGS += -fopenmp
    QMAKE_LFLAGS *= -fopenmp
}
