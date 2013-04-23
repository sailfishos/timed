TEMPLATE = lib

QT -= dbus
QT -= gui

TARGET = tz
VERSION = 0.$$(TIMED_VERSION)

CONFIG += qmlog

HEADERS =
SOURCES = api.cpp timezone.cpp

INCLUDEPATH +=
LIBS +=

target.path = /usr/lib

devheaders.files += tz api.h
devheaders.path  = /usr/include/tz

# prf.files = tz.prf
# prf.path = /usr/share/qt4/mkspecs/features

# pc.files = tz.pc
# pc.path = /usr/lib/pkgconfig

INSTALLS = target # devheaders prf pc

CONFIG(MEEGO) \
{
  message("MEEGO flag is set")
  DEFINES += __MEEGO__
} \
else \
{
  message("MEEGO flag is not set, assuming HARMATTAN")
  DEFINES += __HARMATTAN__
  # LIBS += -lsysinfo
  QMAKE_CXXFLAGS  += -Wall -Wno-psabi
}
QMAKE_CXXFLAGS  += -Wall -Werror

