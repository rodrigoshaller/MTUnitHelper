#-------------------------------------------------
#
# Project created by QtCreator 2018-02-10T11:49:25
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET    = mtUnitHelper
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    mtUnitLogger.cpp \
    mtUnitTestsCompiler.cpp \
    mtUnitEALinker.cpp

HEADERS += \
    mtUnitTestsCompiler.h \
    mtUnitEALinker.h \
    mtUnitlogger.h
