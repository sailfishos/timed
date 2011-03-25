#!/bin/sh

CRAZY_TIMEOUT="--reply-timeout=240000"
BUS="--system"
OPTIONS="$BUS $CRAZY_TIMEOUT --print-reply --type=method_call"

exec dbus-send --dest=com.nokia.timed.backup /com/nokia/timed/backup com.nokia.backupclient.restoreFinished
