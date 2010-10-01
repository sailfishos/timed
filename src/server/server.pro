QT -= gui
QT += dbus

TEMPLATE = app
TARGET = timed

VERSION = 2.18

INCLUDEPATH += ../h

QMAKE_LIBDIR_FLAGS += -L../lib -L../voland
LIBS += -ltimed -ltimed-voland

HEADERS += adaptor.h timed.h states.h automata.h singleshot.h pinguin.h unix-signal.h onitz.h
SOURCES += main.cpp timed.cpp states.cpp automata.cpp timeutil.cpp event.cpp misc.cpp settings.cpp unix-signal.cpp onitz.cpp

SOURCES += credentials.cpp aegis.cpp
HEADERS += credentials.h

SOURCES += olson.cpp tz.cpp
HEADERS += tz.h

CONFIG += link_pkgconfig iodata
PKGCONFIG += contextprovider-1.0 libpcrecpp

target.path = $$(DESTDIR)/usr/bin

xml.files  = com.nokia.time.context
xml.path = $$(DESTDIR)/usr/share/contextkit/providers

typeinfo.files = queue.type timed-rc.type settings.type customization.type tzdata.type timed-cust-rc.type
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
}
