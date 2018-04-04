#-------------------------------------------------
#
# Project created by QtCreator 2016-05-31T13:34:41
#
#-------------------------------------------------

QT       += core gui network sql
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = DF-RSS-filter
TEMPLATE = app

# remove possible other optimization flags
#QMAKE_CXXFLAGS_RELEASE -= -O3
#QMAKE_CXXFLAGS_RELEASE -= -O1
#QMAKE_CXXFLAGS_RELEASE -= -O2
#QMAKE_CXXFLAGS_RELEASE -= -O0

SOURCES += main.cpp\
        dfrssfilter.cpp \
    settings.cpp \
    filters_struct.cpp \
    feed.cpp \
    feeds.cpp \
    feedsandfilters.cpp \
    feedmodel.cpp \
    filtermodel.cpp \
    editwindow.cpp \
    artist_scanner.cpp

HEADERS  += dfrssfilter.h \
    settings.h \
    filters_struct.h \
    feed.h \
    feeds.h \
    feedsandfilters.h \
    feedmodel.h \
    filtermodel.h \
    editwindow.h \
    artist_scanner.h

FORMS    += dfrssfilter.ui \
    settings.ui

CONFIG += c++11

RESOURCES += \
    res.qrc

DISTFILES += \
    android/gradlew \
    android/res/values/libs.xml \
    android/gradle/wrapper/gradle-wrapper.properties \
    android/gradlew.bat \
    android/res/values/libs.xml \
    android/res/values/libs.xml \
    android/AndroidManifest.xml \
    android/gradle/wrapper/gradle-wrapper.jar \
    android/gradlew \
    android/res/values/libs.xml \
    android/build.gradle \
    android/gradle/wrapper/gradle-wrapper.properties \
    android/gradlew.bat


#win32:CONFIG(release, debug|release): LIBS += -L$$PWD/./release/ -llibtag.dll
#else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/./debug/ -llibtag.dll

#INCLUDEPATH += $$PWD/taglib
#DEPENDPATH += $$PWD/taglib


win32: LIBS += -L$$PWD/./ -llibtag.dll

INCLUDEPATH += $$PWD/.
DEPENDPATH += $$PWD/.

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android
