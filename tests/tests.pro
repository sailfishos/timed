TEMPLATE = subdirs

SUBDIRS = ut_networktime tst_events

test_definition.path = $$(DESTDIR)/opt/tests/timed-tests/test-definition
test_definition.files = tests.xml

INSTALLS += test_definition
