equals(QT_MAJOR_VERSION, 4): PACKAGENAME = timed
equals(QT_MAJOR_VERSION, 5): PACKAGENAME = timed-qt5

TEMPLATE = subdirs

SUBDIRS = ut_networktime tst_events

tests_xml.target = tests.xml
tests_xml.depends = $$PWD/tests.xml.in
tests_xml.commands = sed -e "s:@PACKAGENAME@:$${PACKAGENAME}:g" $< > $@

QMAKE_EXTRA_TARGETS = tests_xml
QMAKE_CLEAN += $$tests_xml.target
PRE_TARGETDEPS += $$tests_xml.target

test_definition.depends = tests_xml
test_definition.path = $$(DESTDIR)/opt/tests/$${PACKAGENAME}-tests/test-definition
test_definition.files = $$tests_xml.target
test_definition.CONFIG += no_check_exist

INSTALLS += test_definition
