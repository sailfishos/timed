#!/bin/sh
if [ "$MIC_RUN" != "" ]; then
	echo "setcaps-timed-qt5.sh - returning FAIL to postpone oneshot to first boot"
	exit 1
fi

setcap cap_sys_time+ep /usr/bin/timed-qt5
