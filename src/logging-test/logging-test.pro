QT -= gui
QT += dbus

TEMPLATE = app
TARGET = logging-test.launch

SOURCES = logging-test.cpp

INSTALLS += target script

target.path = $$(DESTDIR)/usr/bin
script.path = $$(DESTDIR)/usr/bin
script.files = logging-test

QMAKE_CXXFLAGS  += -Wall -Wno-psabi

QMAKE_CXXFLAGS += -fPIC # -fvisibility=hidden -fvisibility-inlines-hidden
QMAKE_LFLAGS += -pie -rdynamic
