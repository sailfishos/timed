TEMPLATE = lib

QT += dbus
QT -= gui

TARGET = timed-voland
VERSION = 0.$$(TIMED_VERSION)

HEADERS = interface.h
SOURCES = interface.cpp reminder-pimple.cpp

INCLUDEPATH += ../h

target.path = /usr/lib

devheaders.files = reminder interface ta_interface reminder.h interface.h
devheaders.path  = /usr/include/timed-voland

prf.files = timed-voland.prf
prf.path = /usr/share/qt4/mkspecs/features

pc.files = timed-voland.pc
pc.path = /usr/lib/pkgconfig

INSTALLS = target devheaders prf pc

QMAKE_CXXFLAGS  += -Wall -Wno-psabi
QMAKE_CXXFLAGS  += -Wall -Werror
