QT -= gui
QT += dbus

CONFIG += qmlog
CONFIG += link_pkgconfig

PKGCONFIG += contextprovider-1.0 libpcrecpp dsme_dbus_if

TEMPLATE = app
TARGET = memory

VERSION = $$(TIMED_VERSION)

INCLUDEPATH += ../h

QMAKE_LIBDIR_FLAGS += -L../lib
LIBS += -ltimed

SOURCES = memory.cpp
HEADERS = memory.h

INSTALLS += target
target.path = $$(DESTDIR)/usr/bin

CONFIG(MEEGO) \
{
  message("MEEGO flag is set")
  DEFINES += __MEEGO__
} \
else \
{
  CONFIG += cellular-qt
  QMAKE_CXXFLAGS  += -Wall -Wno-psabi
}
