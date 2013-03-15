QT 	-= gui
QT 	+= dbus

TEMPLATE = app

TARGET   = timedclient
SOURCES += timedclient.cpp
LIBS    += -ltimed
