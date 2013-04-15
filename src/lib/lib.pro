TEMPLATE = lib

QT += dbus
QT -= gui

TARGET = timed
VERSION = 0.$$(TIMED_VERSION)

HEADERS = interface.h
SOURCES = interface.cpp event-io.cpp event-pimple.cpp exception.cpp nanotime.cpp imagetype.cpp aliases.cpp

SOURCES += wall-settings.cpp wall-info.cpp qmacro.cpp

INCLUDEPATH += ../h
LIBS += -lrt

target.path = /usr/lib

devheaders.files += hidden/exception exception.h qmacro.h
devheaders.files += interface interface.h
devheaders.files += event event-declarations.h
devheaders.files += wallclock wall-declarations.h
devheaders.path  = /usr/include/timed

prf.files = timed.prf
prf.path = /usr/share/qt4/mkspecs/features

pc.files = timed.pc
pc.path = /usr/lib/pkgconfig

INSTALLS = target devheaders prf pc

CONFIG(MEEGO) \
{
  message("MEEGO flag is set")
  DEFINES += __MEEGO__
} \
else \
{
  message("MEEGO flag is not set, assuming HARMATTAN")
  DEFINES += __HARMATTAN__
  LIBS += -lsysinfo
  QMAKE_CXXFLAGS  += -Wall -Wno-psabi
}
QMAKE_CXXFLAGS  += -Wall
