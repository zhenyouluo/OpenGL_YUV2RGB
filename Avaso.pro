#-------------------------------------------------
#
# Project created by QtCreator 2015-11-03T10:35:07
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Avaso
TEMPLATE = app


INCLUDEPATH += /usr/include/ImageMagick-6/ \
        source

LIBS += \
    /usr/lib/libMagickCore-6.Q16HDRI.so \
    /usr/lib/libMagick++-6.Q16HDRI.so
#-lMagick++

SOURCES += \
    source/main.cpp \
    source/mainwindow.cpp \
    source/glwidget.cpp \
    source/cameraparameterset.cpp

HEADERS  += \
    source/mainwindow.h \
    source/glwidget.h \
    source/cameraparameterset.h

FORMS    += forms/mainwindow.ui

OTHER_FILES += \
    shaders/fragmentshader.glsl \
    shaders/vertexshader.glsl

RESOURCES += \
    resources.qrc
