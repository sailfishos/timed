QT -= gui
QT -= dbus network

TEMPLATE = app
TARGET = libtz-example

VERSION = $$(TIMED_VERSION)

INCLUDEPATH += ../h .

QMAKE_LIBDIR_FLAGS += -L.
LIBS += -ltz
CONFIG += qmlog

SOURCES += example.cpp
HEADERS +=

target.path = $$(DESTDIR)/usr/bin

INSTALLS += target

CONFIG(MEEGO) \
{
  message("MEEGO flag is set")
  DEFINES += __MEEGO__
} \
else \
{
  message("MEEGO flag is not set, assuming HARMATTAN")
  CONFIG  += cellular-qt
  LIBS    += -lcreds
  DEFINES += __HARMATTAN__
  QMAKE_CXXFLAGS  += -Wall -Wno-psabi
}

QMAKE_CXXFLAGS  += -Wall -Werror

