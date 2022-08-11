#! /bin/sh
### BEGIN INIT INFO
# Provides:          d9demo
# Required-Start:
# Required-Stop:
# Default-Start:     S
# Default-Stop:      2 3 4 5
# Short-Description: One of the first scripts to be executed. Starts or stops
#
### END INIT INFO

DAEMON=/usr/bin/d9demo
ACTION="$1"

case "$ACTION" in
  start)
	echo -n "Starting d9demo App"
	start-stop-daemon -S -b -q -x $DAEMON
	echo "done."
	;;

  stop)
	echo -n "Stopping d9demo App"
	start-stop-daemon -K -x $DAEMON
	echo "."
	;;

  restart)
	echo -n "Restarting d9demo App"
	start-stop-daemon -K -x $DAEMON
	sleep 2
	start-stop-daemon -S -b -q -x $DAEMON
	echo "."
	;;
  *)
	echo "Usage $0 $1 {start/stop/restart}"
	exit 1
esac

exit 0
