#-------------------------------------------------
#
# Project created by QtCreator 2020-11-04T12:28:07
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = IterhardCapture
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += main.cpp\
        widget.cpp \
    rvideodecode.cpp \
    showimagewidget.cpp

HEADERS  += widget.h \
    rvideodecode.h \
    showimagewidget.h

FORMS    += widget.ui \
    showimagewidget.ui
INCLUDEPATH += $$PWD/include

LIBS += $$PWD/lib/avformat.lib \
        $$PWD/lib/avdevice.lib \
        $$PWD/lib/avcodec.lib \
        $$PWD/lib/swscale.lib \
        $$PWD/lib/swresample.lib\
        $$PWD/lib/avutil.lib


LIBS += $$PWD/lib/libx264.a
LIBS += $$PWD/lib/libfdk-aac.a
LIBS += $$PWD/lib/libmfx.lib
LIBS += $$PWD/lib/SDL2.lib
LIBS += $$PWD/lib/SDL2main.lib
LIBS += $$PWD/lib/SDL2test.lib
include($$PWD/libyuv/libyuvpri)
