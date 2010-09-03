QT -= gui
QT += dbus

TEMPLATE = app
TARGET = ticker

VERSION = 2.15

INCLUDEPATH += ../h

QMAKE_LIBDIR_FLAGS += -L../lib
LIBS += -ltimed

SOURCES = ticker.cpp
HEADERS = ticker.h

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
}
