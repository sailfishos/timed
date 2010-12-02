QT -= gui
QT += dbus

TEMPLATE = app
TARGET = simple-client

VERSION = 2.30

INCLUDEPATH += ../h

QMAKE_LIBDIR_FLAGS += -L../lib -L../voland
LIBS += -ltimed -ltimed-voland

CONFIG += iodata

CONFIG += link_pkgconfig
PKGCONFIG += libpcrecpp

SOURCES = client.cpp

INSTALLS += target
target.path = $$(DESTDIR)/usr/bin

QMAKE_CXXFLAGS  += -Wall -Wno-psabi
