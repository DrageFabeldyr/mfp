#-------------------------------------------------
#
# Project created by QtCreator 2016-05-31T13:34:41
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = DF-RSS-filter
TEMPLATE = app


SOURCES += main.cpp\
        dfrssfilter.cpp \
    filter.cpp \
    settings.cpp \
    feeds_settings.cpp \
    filters_struct.cpp

HEADERS  += dfrssfilter.h \
    filter.h \
    settings.h \
    feeds_settings.h \
    filters_struct.h

FORMS    += dfrssfilter.ui \
    filter.ui \
    settings.ui \
    feeds_settings.ui

CONFIG += c++11

RESOURCES += \
    res.qrc


win32: LIBS += -L$$PWD/ -llibtag

INCLUDEPATH += $$PWD/
DEPENDPATH += $$PWD/
