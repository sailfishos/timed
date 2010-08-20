TEMPLATE = lib

QT += dbus
QT -= gui

TARGET = timed-voland
VERSION = 0.2.8

HEADERS = interface.h
SOURCES = interface.cpp reminder-pimple.cpp

INCLUDEPATH += ../h

target.path = /usr/lib

devheaders.files = reminder interface ta_interface reminder.h interface.h
devheaders.path  = /usr/include/timed-voland

prf.files = timed-voland.prf
prf.path = /usr/share/qt4/mkspecs/features

INSTALLS = target devheaders prf
