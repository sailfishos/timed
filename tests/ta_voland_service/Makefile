# ----------------------------------------------------------- -*- mode: sh -*-
# DIRECTORY CONFIGURATION
# ----------------------------------------------------------------------------

# Dummy installation root, real target set by package build env
DESTDIR ?= /tmp/install-test

# Standard directories, package build env can override these
_PREFIX         ?= /usr#                         # /usr
_INCLUDEDIR     ?= $(_PREFIX)/include#           # /usr/include
_EXEC_PREFIX    ?= $(_PREFIX)#                   # /usr
_BINDIR         ?= $(_EXEC_PREFIX)/bin#          # /usr/bin
_SBINDIR        ?= $(_EXEC_PREFIX)/sbin#         # /usr/sbin
_LIBEXECDIR     ?= $(_EXEC_PREFIX)/libexec#      # /usr/libexec
_LIBDIR         ?= $(_EXEC_PREFIX)/lib#          # /usr/lib
_SYSCONFDIR     ?= /etc#                         # /etc
_DATADIR        ?= $(_PREFIX)/share#             # /usr/share
_MANDIR         ?= $(_DATADIR)/man#              # /usr/share/man
_INFODIR        ?= $(_DATADIR)/info#             # /usr/share/info
_DEFAULTDOCDIR  ?= $(_DATADIR)/doc#              # /usr/share/doc
_LOCALSTATEDIR  ?= /var#                         # /var
_UNITDIR        ?= /lib/systemd/system#          # /lib/systemd/system

# Directories used below
BINDIR   := $(_BINDIR)
DBUSDIR  := $(_SYSCONFDIR)/dbus-1

# ----------------------------------------------------------------------------
# List of targets to build
# ----------------------------------------------------------------------------

TARGETS += ta_voland_service
TARGETS += alarms_unfreeze

#TARGETS += systembus.so
#TARGETS += alarmui.png

# ----------------------------------------------------------------------------
# Top level targets
# ----------------------------------------------------------------------------

.PHONY: build install clean distclean mostlyclean

build:: $(TARGETS)

install:: build

clean:: mostlyclean
	$(RM) $(TARGETS)

distclean:: clean

mostlyclean::
	$(RM) *.o *~

# ----------------------------------------------------------------------------
# Default flags
# ----------------------------------------------------------------------------

CPPFLAGS += -D_GNU_SOURCE
CPPFLAGS += -D_FILE_OFFSET_BITS=64

COMMON   += -Wall
COMMON   += -Os
COMMON   += -g

CFLAGS   += $(COMMON)
CFLAGS   += -std=c99

CXXFLAGS += $(COMMON)

LDFLAGS  += -g

LDLIBS   += -Wl,--as-needed

# ----------------------------------------------------------------------------
# Flags from pkg-config
# ----------------------------------------------------------------------------

PKG_NAMES += glib-2.0
PKG_NAMES += dbus-glib-1
PKG_NAMES += dbus-1

maintenance  = normalize clean distclean mostlyclean
intersection = $(strip $(foreach w,$1, $(filter $w,$2)))
ifneq ($(MAKECMDGOALS),)
ifeq ($(call intersection,$(maintenance),$(MAKECMDGOALS)),$(MAKECMDGOALS))
# build goals given, all of them are "maintenance only" -> skip pkg-config
PKG_CONFIG   ?= true
endif
endif

ifneq ($(strip $(PKG_NAMES)),)
PKG_CONFIG   ?= pkg-config
PKG_CFLAGS   := $(shell $(PKG_CONFIG) --cflags $(PKG_NAMES))
PKG_LDLIBS   := $(shell $(PKG_CONFIG) --libs   $(PKG_NAMES))
PKG_CPPFLAGS := $(filter -D%,$(PKG_CFLAGS)) $(filter -I%,$(PKG_CFLAGS))
PKG_CFLAGS   := $(filter-out -I%, $(filter-out -D%, $(PKG_CFLAGS)))
endif

CPPFLAGS += $(PKG_CPPFLAGS)
CFLAGS   += $(PKG_CFLAGS)
LDLIBS   += $(PKG_LDLIBS)

# ----------------------------------------------------------------------------
# Implicit rules
# ----------------------------------------------------------------------------

%.so : %.pic.o
	$(CC) -o $@ -shared $^ $(LDFLAGS) $(LDLIBS)

%.pic.o : %.c
	$(CC) -o $@ -c -fPIC  $^ $(CPPFLAGS) $(CFLAGS)

%.o : %.c
	$(CC) -c -o $@ $< $(CPPFLAGS) $(CFLAGS)

%   : %.o
	$(CC) -o $@ $^ $(LDFLAGS) $(LDLIBS)

%.png : %.dot
	dot -Tpng $< -o $@

%.proto : %.c
	cproto $(CPPFLAGS) $<

% : %.sh
	cp $< $@

# ----------------------------------------------------------------------------
# Explicit dependencies
# ----------------------------------------------------------------------------

ta_voland_service : ta_voland_service.o\
	arr.o\
	attr.o\
	button.o\
	event.o\
	mainloop.o\
	stab.o\
	ta_voland.o\
	voland.o\
	xdbus.o\
	xtimed.o\

systembus.so : LDLIBS += -ldl

install:: ta_voland_service alarms_unfreeze
	install -d -m755 $(DESTDIR)$(BINDIR)/
	install -m755 ta_voland_service $(DESTDIR)$(BINDIR)/
	install -m755 alarms_unfreeze $(DESTDIR)$(BINDIR)/

	install -d -m755 $(DESTDIR)$(DBUSDIR)/system.d/
	install -m755 com.nokia.voland.conf $(DESTDIR)$(DBUSDIR)/system.d/
	install -m755 com.nokia.ta_voland.conf $(DESTDIR)$(DBUSDIR)/system.d/

# ----------------------------------------------------------------------------
# Source code normalization
# ----------------------------------------------------------------------------

.PHONY: normalize
normalize::
	normalize_whitespace -M Makefile
	normalize_whitespace -a $(wildcard *.[ch] *.cc *.cpp)
	normalize_whitespace -a $(wildcard *.conf *.py *.dot)
	normalize_whitespace -a $(wildcard *.sh README)

# ----------------------------------------------------------------------------
# Update ctags
# ----------------------------------------------------------------------------

.PHONY: tags
tags::
	ctags *.[ch]

distclean::
	$(RM) tags

# ----------------------------------------------------------------------------
# Header dependency scanning
# ----------------------------------------------------------------------------

.PHONY: depend
depend:
	gcc -MM $(CPPFLAGS) *.c | ./depend_filter.py >  .depend

ifneq ($(wildcard .depend),)
ifneq ($(MAKECMDGOALS),depend)
include .depend
endif
endif
