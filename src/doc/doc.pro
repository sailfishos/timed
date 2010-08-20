TEMPLATE = lib

# ascii2mandocbook.commands = python2.5 /usr/bin/asciidoc -d manpage -b ${QMAKE_FILE_IN}
ascii2mandocbook.input = ASCIIMAN
ascii2mandocbook.output = ${QMAKE_FILE_BASE}.mandocbook
ascii2mandocbook.variable_out = MANDOCBOOK
# ascii2mandocbook.commands = python2.5 /usr/bin/asciidoc -d manpage -b docbook -o ${QMAKE_FILE_OUT} ${QMAKE_FILE_IN}
ascii2mandocbook.commands = SBOX_REDIRECT_IGNORE=/usr/bin/python asciidoc -d manpage -b docbook -o ${QMAKE_FILE_OUT} ${QMAKE_FILE_IN}

mandocbook2man.commands = xsltproc --nonet -o ${QMAKE_FILE_OUT} /etc/asciidoc/docbook-xsl/manpage.xsl ${QMAKE_FILE_IN}
mandocbook2man.input = MANDOCBOOK
mandocbook2man.output = ${QMAKE_FILE_BASE}
mandocbook2man.variable_out = MANPAGES

man2dummy.commands = touch ${QMAKE_FILE_OUT}
man2dummy.input = MANPAGES
man2dummy.output = ${QMAKE_FILE_IN}.c++
man2dummy.variable_out = SOURCES

ASCIIMAN = libtimed.3.txt libtimed-voland.3.txt timed.8.txt
QMAKE_EXTRA_COMPILERS = ascii2mandocbook mandocbook2man man2dummy

man3.files = libtimed.3 libtimed-voland.3
man8.files = timed.8

man3.path = $$(DESTDIR)/usr/share/man/man3
man8.path = $$(DESTDIR)/usr/share/man/man8

INSTALLS += man3 man8
