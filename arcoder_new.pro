#-------------------------------------------------
#
# Project created by QtCreator 2011-04-21T18:50:26
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = arcoder_new
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    AdaptiveModel.cpp \
    ArithmeticCoderImplementation.cpp \
    BitStream.cpp \
    ArithmeticCoder.cpp

HEADERS += \
    my_exception.h \
    ArithmeticCoder.h \
    AdaptiveModel.h \
    ArithmeticCoderImplementation.h \
    BitStream.h
