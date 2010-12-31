QT -= gui
QT += dbus

TEMPLATE = app
TARGET = timed

VERSION = $$(TIMED_VERSION)

INCLUDEPATH += ../h

QMAKE_LIBDIR_FLAGS += -L../lib -L../voland
LIBS += -ltimed -ltimed-voland
CONFIG += qmlog

HEADERS += interfaces.h adaptor.h timed.h state.h cluster.h machine.h singleshot.h pinguin.h unix-signal.h onitz.h
SOURCES += cluster.cpp machine.cpp state.cpp main.cpp timed.cpp timeutil.cpp event.cpp misc.cpp settings.cpp unix-signal.cpp onitz.cpp

SOURCES += credentials.cpp aegis.cpp
HEADERS += credentials.h

SOURCES += olson.cpp tz.cpp
HEADERS += tz.h

SOURCES += backup.cpp
HEADERS += backup.h

CONFIG += link_pkgconfig iodata
PKGCONFIG += contextprovider-1.0 libpcrecpp dsme_dbus_if

target.path = $$(DESTDIR)/usr/bin

xml.files  = com.nokia.time.context
xml.path = $$(DESTDIR)/usr/share/contextkit/providers

typeinfo.files = queue.type config.type settings.type customization.type tzdata.type timed-cust-rc.type
typeinfo.path = $$(DESTDIR)/usr/share/timed/typeinfo

backupconf.files = timedbackup.conf
backupconf.path = $$(DESTDIR)/usr/share/backup-framework/applications

cud.files = timed-clear-device.sh
cud.path = $$(DESTDIR)/etc/osso-cud-scripts

rfs.files = timed-restore-original-settings.sh
rfs.path = $$(DESTDIR)/etc/osso-rfs-scripts

INSTALLS += target xml typeinfo backupconf cud rfs

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
