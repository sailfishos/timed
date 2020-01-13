QT 	-= gui
QT 	+= dbus

TEMPLATE = app

TARGET   = timedclient-qt5
SOURCES += timedclient.cpp

INCLUDEPATH += ../../src/h
QMAKE_LIBDIR_FLAGS += -L../../src/lib

LIBS += -ltimed-qt5

INSTALLS += target
target.path = $$(DESTDIR)/usr/bin
