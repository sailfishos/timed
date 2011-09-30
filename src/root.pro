TEMPLATE = subdirs
SUBDIRS = lib voland server simple-client fake-dialog-ui ticker logging-test ntp tfd
SUBDIRS += memory
# SUBDIRS += arch
CONFIG += ordered

INSTALLS += tests

tests.path = $$(DESTDIR)/usr/share/timed-tests
tests.files = tests.xml
