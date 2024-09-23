QT -= gui
QT += dbus

TEMPLATE = app

TARGET = timed

VERSION = $$(TIMED_VERSION)

INCLUDEPATH += ../h

QMAKE_LIBDIR_FLAGS += -L../lib -L../voland
LIBS += -ltimed-qt$${QT_MAJOR_VERSION} -ltimed-voland-qt$${QT_MAJOR_VERSION} -liodata-qt$${QT_MAJOR_VERSION}

IODATA_TYPES = queue.type config.type settings.type customization.type tzdata.type

CONFIG(dsme_dbus_if) {
    SOURCES += dsme-mode.cpp
    HEADERS += dsme-mode.h interfaces.h
    DEFINES += HAVE_DSME
}

CONFIG(ofono) {
    DEFINES += OFONO
}

HEADERS += \
    tzdata.h \
    cellular.h \
    settings.h \
    csd.h \
    adaptor.h \
    timed.h \
    state.h \
    cluster.h \
    machine.h \
    singleshot.h \
    pinguin.h \
    unix-signal.h \
    networktime.h \
    networktimewatcher.h \
    networkoperator.h \
    networkregistrationwatcher.h \
    networktimeinfo.h \
    ofonomodemmanager.h \
    modemwatcher.h \
    ofonoconstants.h \
    ntpcontroller.h

SOURCES += \
    tzdata.cpp \
    cellular.cpp \
    csd.cpp \
    cluster.cpp \
    machine.cpp \
    state.cpp \
    main.cpp \
    timed.cpp \
    timeutil.cpp \
    event.cpp \
    misc.cpp \
    settings.cpp \
    pinguin.cpp \
    unix-signal.cpp \
    networktime.cpp \
    networktimewatcher.cpp \
    networkoperator.cpp \
    networkregistrationwatcher.cpp \
    networktimeinfo.cpp \
    ofonomodemmanager.cpp \
    modemwatcher.cpp \
    ofonoconstants.cpp \
    ntpcontroller.cpp

SOURCES += credentials.cpp
HEADERS += credentials.h

SOURCES += olson.cpp tz.cpp
HEADERS += tz.h

SOURCES += notification.cpp
HEADERS += notification.h

CONFIG += link_pkgconfig iodata-qt$${QT_MAJOR_VERSION}
PKGCONFIG += libpcrecpp libsystemd
PKGCONFIG += sailfishaccesscontrol

CONFIG(dsme_dbus_if) {
    PKGCONFIG += dsme_dbus_if
}

target.path = $$(DESTDIR)/usr/bin

timedrc.path  = $$(DESTDIR)/etc
timedrc.files = timed.rc

dbusconf.path  = $$(DESTDIR)/etc/dbus-1/system.d
dbusconf.files = timed.conf

systemd.path = $$(DESTDIR)/usr/lib/systemd/user
systemd.files = timed.service

oneshot.path = $$(DESTDIR)/usr/lib/oneshot.d
oneshot.files = setcaps-timed.sh

INSTALLS += target timedrc dbusconf systemd oneshot

QMAKE_CXXFLAGS  += -Wall

OTHER_FILES += *.sh
