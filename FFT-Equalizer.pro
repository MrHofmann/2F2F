#-------------------------------------------------
#
# Project created by QtCreator 2017-04-28T08:54:52
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = FFT-Equalizer
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


LIBS += -lavcodec -lavformat -lavutil -lSDL

SOURCES += main.cpp\
        mainwindow.cpp \
    band_pass.cpp \
    fft.cpp \
    filter.cpp \
    high_pass.cpp \
    init.cpp \
    low_pass.cpp \
    playback_thread.cpp

HEADERS  += mainwindow.h \
    band_pass.h \
    fft.h \
    filter.h \
    high_pass.h \
    init.h \
    low_pass.h \
    playback_thread.h

FORMS    += mainwindow.ui
