#-------------------------------------------------
#
# Project created by QtCreator 2016-05-31T13:34:41
#
#-------------------------------------------------

QT       += core gui network sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = DF-RSS-filter
TEMPLATE = app


SOURCES += main.cpp\
        dfrssfilter.cpp \
    settings.cpp \
    filters_struct.cpp \
    feed.cpp \
    feeds.cpp \
    feedsandfilters.cpp \
    feedmodel.cpp \
    filtermodel.cpp \
    editwindow.cpp

HEADERS  += dfrssfilter.h \
    settings.h \
    filters_struct.h \
    feed.h \
    feeds.h \
    feedsandfilters.h \
    feedmodel.h \
    filtermodel.h \
    editwindow.h

FORMS    += dfrssfilter.ui \
    settings.ui

CONFIG += c++11

RESOURCES += \
    res.qrc

win32: LIBS += -L$$PWD/ -llibtag

INCLUDEPATH += $$PWD/
DEPENDPATH += $$PWD/
