#-------------------------------------------------
#
# Project created by QtCreator 2015-05-21T10:05:14
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = SilvaTest5
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    dataholder.cpp \
    dialog.cpp \
    tracepickdialog.cpp \
    voltagedialog.cpp \
    exportform.cpp

HEADERS  += mainwindow.h \
    dataholder.h \
    dialog.h \
    tracepickdialog.h \
    voltagedialog.h \
    exportform.h

FORMS    += mainwindow.ui \
    dialog.ui \
    tracepickdialog.ui \
    voltagedialog.ui \
    exportform.ui


# Default rules for deployment.
include(deployment.pri)
include (C:\qwt-6.1.2/features/qwt.prf )

DISTFILES +=
