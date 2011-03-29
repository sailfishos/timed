CRAZY_TIMEOUT="--reply-timeout=240000"
BUS="--system"
OPTIONS="$BUS $CRAZY_TIMEOUT --print-reply --type=method_call"

FILE="/home/user/TIMED-BACKUP"

echo "$*" >> $FILE

exec dbus-send $OPTIONS --dest=com.nokia.timed.backup /com/nokia/timed/backup com.nokia.backupclient.backupStarts >> $FILE 2>&1
