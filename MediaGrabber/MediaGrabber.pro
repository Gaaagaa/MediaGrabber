#-------------------------------------------------
#
# Project created by QtCreator 2019-07-18T14:19:40
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MediaGrabber
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

QMAKE_CXXFLAGS += -Wno-unused-parameter
QMAKE_CXXFLAGS += -Wno-unused-variable
QMAKE_CXXFLAGS += -Wno-zero-as-null-pointer-constant

win32 {
    DEFINES += _WIN32_WINNT=0x0601
    INCLUDEPATH += $$PWD/../include

    contains(QT_ARCH, i386) {
        LIBS += -L$$PWD/../lib/vlc/x86
    } else {
        LIBS += -L$$PWD/../lib/vlc/x64
    }

    LIBS += -lws2_32 -lvlc -lvlccore
}

unix {
    LIBS += -lvlc
}

CONFIG += c++11

SOURCES += \
    main.cpp \
    widget.cpp \
    rframewidget.cpp \
    vlc_mgrabber.cpp \
    wave_file.cpp

HEADERS += \
    widget.h \
    xtypes.h \
    rframewidget.h \
    vlc_mgrabber.h \
    wave_file.h

FORMS += \
    widget.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
