QT -= gui
QT += dbus

TEMPLATE = app
TARGET = logging-test.launch

VERSION = 2.20

INCLUDEPATH += ../h

# QMAKE_LIBDIR_FLAGS += -L../lib -L../voland
# LIBS += -ltimed -ltimed-voland

# CONFIG += iodata

CONFIG += link_pkgconfig
PKGCONFIG += libpcrecpp

SOURCES = logging-test.cpp

INSTALLS += target script

target.path = $$(DESTDIR)/usr/bin
script.path = $$(DESTDIR)/usr/bin
script.files = logging-test

QMAKE_CXXFLAGS  += -Wall -Wno-psabi
