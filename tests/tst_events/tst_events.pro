TEMPLATE = app
TARGET = tst_events

QT += testlib dbus
QT -= gui

INCLUDEPATH += ../../src/h

QMAKE_LIBDIR_FLAGS += -L../../src/lib -L../../src/voland
equals(QT_MAJOR_VERSION, 4): LIBS += -ltimed -ltimed-voland
equals(QT_MAJOR_VERSION, 5): LIBS += -ltimed-qt5 -ltimed-voland-qt5

HEADERS += tst_events.h \
           simplevolandadaptor.h

SOURCES += tst_events.cpp \
           simplevolandadaptor.cpp

equals(QT_MAJOR_VERSION, 4): target.path = $$(DESTDIR)/opt/tests/timed-tests
equals(QT_MAJOR_VERSION, 5): target.path = $$(DESTDIR)/opt/tests/timed-qt5-tests
INSTALLS += target
