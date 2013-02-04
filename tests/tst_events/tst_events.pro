TEMPLATE = app
TARGET = tst_events

QT += testlib dbus

CONFIG += timed

INCLUDEPATH += ../../src/h

QMAKE_LIBDIR_FLAGS += -L../../src/lib -L../../src/voland
LIBS += -ltimed -ltimed-voland

HEADERS += tst_events.h \
           simplevolandadaptor.h

SOURCES += tst_events.cpp \
           simplevolandadaptor.cpp

target.path = $$(DESTDIR)/opt/tests/timed-tests
INSTALLS += target
