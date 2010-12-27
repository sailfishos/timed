QT -= gui
QT += dbus

TEMPLATE = app
TARGET = fake-dialog-ui

VERSION = 2.32

INCLUDEPATH += ../h

QMAKE_LIBDIR_FLAGS += -L../voland -L../lib
LIBS += -ltimed-voland -ltimed

CONFIG += iodata qmlog

SOURCES = fake-dialog-ui.cpp
HEADERS = fake-dialog-ui.h

INSTALLS += target
target.path = $$(DESTDIR)/usr/bin

QMAKE_CXXFLAGS  += -Wall -Wno-psabi
