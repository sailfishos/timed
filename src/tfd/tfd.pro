QT -= gui

TEMPLATE = app
TARGET = tfd

VERSION = $$(TIMED_VERSION)

INCLUDEPATH += ../h

LIBS += -lrt

SOURCES = tfd.cpp

INSTALLS += target
target.path = $$(DESTDIR)/usr/bin

QMAKE_CXXFLAGS  += -Wno-psabi
QMAKE_CXXFLAGS  += -Wall -Werror

