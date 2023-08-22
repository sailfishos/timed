QT 	-= gui
QT 	+= dbus

TEMPLATE = app

TARGET   = timedclient-qt$${QT_MAJOR_VERSION}
SOURCES += timedclient.cpp

INCLUDEPATH += ../../src/h
QMAKE_LIBDIR_FLAGS += -L../../src/lib

LIBS += -ltimed-qt$${QT_MAJOR_VERSION}

INSTALLS += target
target.path = $$(DESTDIR)/usr/bin
