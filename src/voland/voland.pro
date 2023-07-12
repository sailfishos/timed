TEMPLATE = lib

QT += dbus
QT -= gui

TARGET = timed-voland-qt$${QT_MAJOR_VERSION}
VERSION = 0.$$(TIMED_VERSION)

HEADERS = interface.h
SOURCES = interface.cpp reminder-pimple.cpp

INCLUDEPATH += ../h

target.path = $$[QT_INSTALL_LIBS]

devheaders.files = reminder interface ta_interface reminder.h interface.h
devheaders.path  = /usr/include/timed-voland-qt$${QT_MAJOR_VERSION}

pc.path = $$[QT_INSTALL_LIBS]/pkgconfig
pc.files = timed-voland-qt$${QT_MAJOR_VERSION}.pc

prf.path =  $$[QT_INSTALL_DATA]/mkspecs/features
prf.files = timed-voland-qt$${QT_MAJOR_VERSION}.prf

INSTALLS = target devheaders prf pc

OTHER_FILES += *.pc *.prf

QMAKE_CXXFLAGS  += -Wall -Wno-psabi
QMAKE_CXXFLAGS  += -Wall -Werror
