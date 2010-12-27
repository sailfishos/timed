QT -= gui
QT += dbus network

TEMPLATE = app
TARGET = ntp-helper

VERSION = 2.32

INCLUDEPATH += ../h

# QMAKE_LIBDIR_FLAGS += -L../lib -L../voland
# LIBS += -ltimed -ltimed-voland

CONFIG += qmlog link_pkgconfig
PKGCONFIG += libpcrecpp

SOURCES = ntp.cpp
HEADERS = ntp.h

INSTALLS += target
target.path = $$(DESTDIR)/usr/bin

QMAKE_CXXFLAGS  += -Wall -Wno-psabi
