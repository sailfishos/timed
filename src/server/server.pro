QT -= gui
QT += dbus

TEMPLATE = app

equals(QT_MAJOR_VERSION, 4): TARGET = timed
equals(QT_MAJOR_VERSION, 5): TARGET = timed-qt5

VERSION = $$(TIMED_VERSION)

INCLUDEPATH += ../h

QMAKE_LIBDIR_FLAGS += -L../lib -L../voland
equals(QT_MAJOR_VERSION, 4): LIBS += -ltimed -ltimed-voland
equals(QT_MAJOR_VERSION, 5): LIBS += -ltimed-qt5 -ltimed-voland-qt5

IODATA_TYPES = queue.type config.type settings.type customization.type tzdata.type

CONFIG(dsme_dbus_if) {
    SOURCES += dsme-mode.cpp
    HEADERS += dsme-mode.h interfaces.h
    DEFINES += HAVE_DSME
}

HEADERS += peer.h \
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
    onitz.h \
    networktime.h \
    networktimewatcher.h \
    networkoperator.h \
    networkregistrationwatcher.h \
    networktimeinfo.h \
    ofonomodemmanager.h \
    modemwatcher.h \
    ofonoconstants.h \
    ntpcontroller.h

SOURCES += peer.cpp \
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
    onitz.cpp \
    networktime.cpp \
    networktimewatcher.cpp \
    networkoperator.cpp \
    networkregistrationwatcher.cpp \
    networktimeinfo.cpp \
    ofonomodemmanager.cpp \
    modemwatcher.cpp \
    ofonoconstants.cpp \
    ntpcontroller.cpp

SOURCES += credentials.cpp aegis.cpp
HEADERS += credentials.h

SOURCES += olson.cpp tz.cpp
HEADERS += tz.h

SOURCES += backup.cpp
HEADERS += backup.h

SOURCES += notification.cpp
HEADERS += notification.h

CONFIG += link_pkgconfig
PKGCONFIG += libpcrecpp libsystemd-daemon
equals(QT_MAJOR_VERSION, 4) {
    CONFIG += iodata
    PKGCONFIG += contextprovider-1.0
}
equals(QT_MAJOR_VERSION, 5) {
    QMAKE_CXXFLAGS += -std=c++0x # required by statefs-qt5
    CONFIG += iodata-qt5
    PKGCONFIG += statefs-qt5
}


CONFIG(dsme_dbus_if) {
    PKGCONFIG += dsme_dbus_if
}

target.path = $$(DESTDIR)/usr/bin

xml.files  = com.nokia.time.context
xml.path = $$(DESTDIR)/usr/share/contextkit/providers

equals(QT_MAJOR_VERSION, 4) {
    timedrc.files = timed.rc
    dbusconf.files = timed.conf
    systemd.files = timed.service
    oneshot.files = setcaps-timed.sh
}
equals(QT_MAJOR_VERSION, 5) {
    timedrc.files = timed-qt5.rc
    dbusconf.files = timed-qt5.conf
    systemd.files = timed-qt5.service
    oneshot.files = setcaps-timed-qt5.sh
    statefs.files = timed-statefs.conf
    statefs.path = /etc
    INSTALLS += statefs
}
timedrc.path  = $$(DESTDIR)/etc
dbusconf.path  = $$(DESTDIR)/etc/dbus-1/system.d
systemd.path = $$(DESTDIR)/usr/lib/systemd/user
oneshot.path = $$(DESTDIR)/usr/lib/oneshot.d

INSTALLS += target xml timedrc dbusconf systemd oneshot

QMAKE_CXXFLAGS  += -Wall

OTHER_FILES += *.sh
