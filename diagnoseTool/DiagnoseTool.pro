#-------------------------------------------------
#
# Project created by QtCreator 2015-11-30T11:11:40
#
#-------------------------------------------------
QT       += core gui network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = DiagnoseTool
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    diagnoseworker.cpp \
    wxconfig.cpp \
    wintool.cpp

HEADERS  += mainwindow.h \
    diagnoseworker.h \
    wxconfig.h \
    wintool.h

FORMS    += mainwindow.ui

RESOURCES += \
    diag.qrc

LIBS += -lversion
