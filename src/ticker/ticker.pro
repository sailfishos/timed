QT -= gui
QT += dbus

TEMPLATE = app
TARGET = ticker

VERSION = 2.20

INCLUDEPATH += ../h

QMAKE_LIBDIR_FLAGS += -L../lib
LIBS += -ltimed

SOURCES = ticker.cpp
HEADERS = ticker.h

INSTALLS += target qmtime-users
target.path = $$(DESTDIR)/usr/bin

qmtime-users.files = qmtime-users.sh
qmtime-users.path = $$(DESTDIR)/usr/bin

CONFIG(MEEGO) \
{
  message("MEEGO flag is set")
  DEFINES += __MEEGO__
} \
else \
{
  CONFIG += cellular-qt
}
