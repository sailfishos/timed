#!/bin/sh -e

python2.5 /usr/bin/asciidoc -d manpage -b docbook libtimed.3.txt
xsltproc --nonet /etc/asciidoc/docbook-xsl/manpage.xsl libtimed.3.xml


python2.5 /usr/bin/asciidoc -d manpage -b docbook libtimed-voland.3.txt
xsltproc --nonet /etc/asciidoc/docbook-xsl/manpage.xsl libtimed-voland.3.xml
