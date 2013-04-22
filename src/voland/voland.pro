TEMPLATE = lib

QT += dbus
QT -= gui

equals(QT_MAJOR_VERSION, 4): TARGET = timed-voland
equals(QT_MAJOR_VERSION, 5): TARGET = timed-voland-qt5
VERSION = 0.$$(TIMED_VERSION)

HEADERS = interface.h
SOURCES = interface.cpp reminder-pimple.cpp

INCLUDEPATH += ../h

target.path = /usr/lib

devheaders.files = reminder interface ta_interface reminder.h interface.h
equals(QT_MAJOR_VERSION, 4): devheaders.path  = /usr/include/timed-voland
equals(QT_MAJOR_VERSION, 5): devheaders.path  = /usr/include/timed-voland-qt5

pc.path = /usr/lib/pkgconfig
equals(QT_MAJOR_VERSION, 4) {
    pc.files = timed-voland.pc
    prf.files = timed-voland.prf
    prf.path = /usr/share/qt4/mkspecs/features
}
equals(QT_MAJOR_VERSION, 5) {
    pc.files = timed-voland-qt5.pc
    prf.files = timed-voland-qt5.prf
    prf.path = /usr/share/qt5/mkspecs/features
}

INSTALLS = target devheaders prf pc

OTHER_FILES += *.pc *.prf

QMAKE_CXXFLAGS  += -Wall -Wno-psabi
QMAKE_CXXFLAGS  += -Wall -Werror
