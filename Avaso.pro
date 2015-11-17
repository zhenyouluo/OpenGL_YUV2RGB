#-------------------------------------------------
#
# Project created by QtCreator 2015-11-03T10:35:07
#
#-------------------------------------------------

QT       += core gui

CONFIG += c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Avaso
TEMPLATE = app


INCLUDEPATH += /usr/include/ImageMagick-6/ \
        source

LIBS += \
    /usr/lib/libMagickCore-6.Q16HDRI.so \
    /usr/lib/libMagick++-6.Q16HDRI.so
#-lMagick++

HEADERS  += \
    source/mainwindow.h \
    source/glwidget.h \
    source/cameraparameterset.h \
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
    source/cameraparameterset.cpp \
    source/textureplusdepthsequencelistmodel.cpp \
    source/playbackcontroller.cpp \
    source/common.cpp \
    source/yuvfile.cpp \
    source/yuvsource.cpp

FORMS    += forms/mainwindow.ui

OTHER_FILES += \
    shaders/fragmentshader.glsl \
    shaders/fragmentYUV2RGBshader.glsl \
    shaders/vertexshader.glsl


RESOURCES += \
    resources.qrc

linux {
    QMAKE_CXXFLAGS += -fopenmp
    QMAKE_LFLAGS *= -fopenmp
}
