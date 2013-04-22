QT 	-= gui
QT 	+= dbus

TEMPLATE = app

equals(QT_MAJOR_VERSION, 4): TARGET   = timedclient
equals(QT_MAJOR_VERSION, 5): TARGET   = timedclient-qt5
SOURCES += timedclient.cpp

INCLUDEPATH += ../../src/h
QMAKE_LIBDIR_FLAGS += -L../../src/lib

equals(QT_MAJOR_VERSION, 4): LIBS += -ltimed
equals(QT_MAJOR_VERSION, 5): LIBS += -ltimed-qt5

INSTALLS += target
target.path = $$(DESTDIR)/usr/bin
