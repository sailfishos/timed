#!/bin/sh -e

# This script is setting system time to the value given by RTC (hwclock)
# and then it checks, if the time is rolled back from the last run,
# in that case the saved time is being restored

# see http://busybox.net/downloads/BusyBox.html for date/hwclock options

SAVED_TIME_FILE=/var/cache/timed/utc.time
VERBOSE="yes" ; SILENT="no"

need="yes" ; failed="no"

[ "$need" = "yes" ] && saved_time=$(head -n 1 "$SAVED_TIME_FILE") || need="no"
[ "$VERBOSE" != "yes" ] || echo "$0: saved_time=$saved_time"

[ "$need" = "yes" ] && saved_value=$(date +%s -u -d "$saved_time") || need="no"
[ "$VERBOSE" != "yes" ] || echo "$0: saved_value=$saved_value"

# first set the system time by hwclock
hwclock -s -u

[ "$need" = "yes" ] && system_value=$(date -u +%s) || need="no"
[ "$VERBOSE" != "yes" ] || echo "$0: system_value=$system_value"

[ "$need" = "yes" ] && [ "$system_value" -lt "$saved_value" ] || need="no"

if [ "$need" = "yes" ] ; then
  date -u -s "$saved_time" && hwclock -u -w || failed="yes"
fi

[ "$VERBOSE" != "yes" ] || echo "$0: need=$need failed=$failed"

if [ "$SILENT" != "yes" ] ; then
  echo "Current time settings (UTC):"
  echo -n "Real time clock : " ; hwclock -u -r
  echo -n "System time     : " ; date -u
fi

