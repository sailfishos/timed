TEMPLATE = lib

QT += dbus
QT -= gui

TARGET = timed-qt$${QT_MAJOR_VERSION}

VERSION = 0.$$(TIMED_VERSION)

HEADERS = interface.h
SOURCES = interface.cpp event-io.cpp event-pimple.cpp exception.cpp nanotime.cpp aliases.cpp

SOURCES += wall-settings.cpp wall-info.cpp qmacro.cpp

LIBS += -lrt

target.path = $$[QT_INSTALL_LIBS]

devheaders.files += hidden/exception exception.h qmacro.h
devheaders.files += interface interface.h
devheaders.files += event event-declarations.h
devheaders.files += wallclock wall-declarations.h
devheaders.path = /usr/include/timed-qt$${QT_MAJOR_VERSION}
devheaders.path  = /usr/include/$$TARGET

pc.path = $$[QT_INSTALL_LIBS]/pkgconfig
pc.files = timed-qt$${QT_MAJOR_VERSION}.pc

prf.path =  $$[QT_INSTALL_DATA]/mkspecs/features
prf.files = timed-qt$${QT_MAJOR_VERSION}.prf

INSTALLS = target devheaders prf pc

OTHER_FILES += *.pc *.prf

QMAKE_CXXFLAGS  += -Wall
