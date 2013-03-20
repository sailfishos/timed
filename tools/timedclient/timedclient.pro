QT 	-= gui
QT 	+= dbus

TEMPLATE = app

TARGET   = timedclient
SOURCES += timedclient.cpp

INCLUDEPATH += ../../src/h
QMAKE_LIBDIR_FLAGS += -L../../src/lib

LIBS    += -ltimed

INSTALLS += target
target.path = $$(DESTDIR)/usr/bin
