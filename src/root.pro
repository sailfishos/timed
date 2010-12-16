TEMPLATE = subdirs
SUBDIRS = lib voland doc server simple-client fake-dialog-ui ticker logging-test ntp
SUBDIRS += memory
# SUBDIRS += arch
CONFIG += ordered

INSTALLS += tests

tests.path = $$(DESTDIR)/usr/share/timed-tests
tests.files = tests.xml
