TEMPLATE = lib

QT += dbus
QT -= gui

TARGET = timed
VERSION = 0.2.30

CONFIG += qmlog

HEADERS = interface.h
SOURCES = interface.cpp event-io.cpp event-pimple.cpp exception.cpp nanotime.cpp log.cpp imagetype.cpp aliases.cpp

SOURCES += wall-settings.cpp wall-info.cpp

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

INSTALLS = target devheaders prf

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
