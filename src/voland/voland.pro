TEMPLATE = lib

QT += dbus
QT -= gui

TARGET = timed-voland-qt5
VERSION = 0.$$(TIMED_VERSION)

HEADERS = interface.h
SOURCES = interface.cpp reminder-pimple.cpp

INCLUDEPATH += ../h

target.path = /usr/lib

devheaders.files = reminder interface ta_interface reminder.h interface.h
devheaders.path  = /usr/include/timed-voland-qt5

pc.path = /usr/lib/pkgconfig
pc.files = timed-voland-qt5.pc

prf.path =  $$[QT_INSTALL_DATA]/mkspecs/features
prf.files = timed-voland-qt5.prf

INSTALLS = target devheaders prf pc

OTHER_FILES += *.pc *.prf

QMAKE_CXXFLAGS  += -Wall -Wno-psabi
QMAKE_CXXFLAGS  += -Wall -Werror
