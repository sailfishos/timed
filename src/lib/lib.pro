TEMPLATE = lib

QT += dbus
QT -= gui

equals(QT_MAJOR_VERSION, 4): TARGET = timed
equals(QT_MAJOR_VERSION, 5): TARGET = timed-qt5

VERSION = 0.$$(TIMED_VERSION)

HEADERS = interface.h
SOURCES = interface.cpp event-io.cpp event-pimple.cpp exception.cpp nanotime.cpp aliases.cpp

SOURCES += wall-settings.cpp wall-info.cpp qmacro.cpp

LIBS += -lrt

target.path = /usr/lib

devheaders.files += hidden/exception exception.h qmacro.h
devheaders.files += interface interface.h
devheaders.files += event event-declarations.h
devheaders.files += wallclock wall-declarations.h
equals(QT_MAJOR_VERSION, 4): devheaders.path = /usr/include/timed
equals(QT_MAJOR_VERSION, 5): devheaders.path = /usr/include/timed-qt5
devheaders.path  = /usr/include/$$TARGET


pc.path = /usr/lib/pkgconfig
prf.path =  $$[QT_INSTALL_DATA]/mkspecs/features
equals(QT_MAJOR_VERSION, 4) {
    pc.files = timed.pc
    prf.files = timed.prf
}
equals(QT_MAJOR_VERSION, 5) {
    pc.files = timed-qt5.pc
    prf.files = timed-qt5.prf
}

INSTALLS = target devheaders prf pc

OTHER_FILES += *.pc *.prf

QMAKE_CXXFLAGS  += -Wall
