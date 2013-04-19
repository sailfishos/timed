TEMPLATE = subdirs

SUBDIRS = ut_networktime tst_events

equals(QT_MAJOR_VERSION, 4) {
    test_definition.path = $$(DESTDIR)/opt/tests/timed-tests/test-definition
    test_definition.files = test-definition/tests.xml
}
equals(QT_MAJOR_VERSION, 5) {
    test_definition.path = $$(DESTDIR)/opt/tests/timed-qt5-tests/test-definition
    test_definition.files = test-definition-qt5/tests.xml
}

OTHER_FILES += test-definition/*.xml test-definition-qt5/*.xml

INSTALLS += test_definition
