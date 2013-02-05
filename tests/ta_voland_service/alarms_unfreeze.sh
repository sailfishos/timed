#! /bin/sh

unfreeze_to_ACTDEAD()
{
  dbus-send --system --dest=com.nokia.time \
  /com/nokia/startup/signal \
  com.nokia.startup.signal.init_done int32:5
}

unfreeze_to_USER()
{
  dbus-send --system --dest=com.nokia.time \
    /com/nokia/startup/signal \
    com.nokia.startup.signal.desktop_visible
}

case $1 in
  ACT*DEAD)
    unfreeze_to_ACTDEAD
    ;;
  *)
    unfreeze_to_USER
    ;;
esac
