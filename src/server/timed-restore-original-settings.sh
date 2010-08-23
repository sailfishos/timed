#!/bin/sh

dbus-send --session --print-reply --dest=com.nokia.time /com/nokia/time com.nokia.time.halt string:restore-original-settings

# vim:tw=0:
