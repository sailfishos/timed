TEMPLATE = app
TARGET = tst_events

QT += testlib dbus
QT -= gui

INCLUDEPATH += ../../src/h

QMAKE_LIBDIR_FLAGS += -L../../src/lib -L../../src/voland
LIBS += -ltimed-qt5 -ltimed-voland-qt5

HEADERS += tst_events.h \
           simplevolandadaptor.h

SOURCES += tst_events.cpp \
           simplevolandadaptor.cpp

target.path = $$(DESTDIR)/opt/tests/timed-qt5-tests
INSTALLS += target
