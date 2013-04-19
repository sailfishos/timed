TEMPLATE = app
TARGET = ut_networktime

QT += testlib dbus
QT -= gui

HEADERS += ut_networktime.h \
           ../../src/server/networktimeinfo.h \
           ../../src/server/networktime.h \
           ../../src/server/networkoperator.h \
           ../../src/server/networkregistrationwatcher.h \
           ../../src/server/ofonomodemmanager.h \
           ../../src/server/networktimewatcher.h \
           ../../src/server/ofonoconstants.h \
           ../../src/server/modemwatcher.h \
            ../../src/server/ofonoconstants.h \
           fakeofono/fakeofono.h \
           fakeofono/networkregistrationinterface.h \
           fakeofono/modeminterface.h \
           fakeofono/networktimeinterface.h

SOURCES += ut_networktime.cpp \
           ../../src/server/networktimeinfo.cpp \
           ../../src/server/networktime.cpp \
           ../../src/server/networkoperator.cpp \
           ../../src/server/networkregistrationwatcher.cpp \
           ../../src/server/ofonomodemmanager.cpp \
           ../../src/server/networktimewatcher.cpp \
           ../../src/server/modemwatcher.cpp \
           fakeofono/fakeofono.cpp \
           fakeofono/networkregistrationinterface.cpp \
           fakeofono/modeminterface.cpp \
           fakeofono/networktimeinterface.cpp

INCLUDEPATH += ../../src/server fakeofono ../../src/common

OTHER_FILES += fakeofono/org.fakeofono.conf

equals(QT_MAJOR_VERSION, 4): target.path = $$(DESTDIR)/opt/tests/timed-tests
equals(QT_MAJOR_VERSION, 5): target.path = $$(DESTDIR)/opt/tests/timed-qt5-tests
dbusconf.files = fakeofono/org.fakeofono.conf
dbusconf.path  = $$(DESTDIR)/etc/dbus-1/system.d

INSTALLS += target dbusconf
