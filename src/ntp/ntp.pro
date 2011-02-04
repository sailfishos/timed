QT -= gui
QT += dbus network

TEMPLATE = app
TARGET = ntp-helper

VERSION = $$(TIMED_VERSION)

INCLUDEPATH += ../h

QMAKE_LIBDIR_FLAGS += -L../lib # -L../voland
LIBS += -ltimed

CONFIG += qmlog link_pkgconfig
PKGCONFIG += libpcrecpp

SOURCES = ntp.cpp resolver.cpp xxd.cpp # main.cpp
HEADERS = ntp.h   resolver.h   xxd.h

INSTALLS += target
target.path = $$(DESTDIR)/usr/bin

QMAKE_CXXFLAGS  += -Wall -Wno-psabi
