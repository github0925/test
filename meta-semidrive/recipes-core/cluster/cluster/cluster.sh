#! /bin/sh
### BEGIN INIT INFO
# Provides:          clusterd
# Required-Start:
# Required-Stop:
# Default-Start:     S
# Default-Stop:      2 3 4 5
# Short-Description: One of the first scripts to be executed. Starts or stops
#
### END INIT INFO

DAEMON=/usr/bin/cluster
ACTION="$1"

case "$ACTION" in
  start)
	echo -n "Starting Cluster App"
	start-stop-daemon -S -b -q -x $DAEMON
	echo "done."
	;;

  stop)
	echo -n "Stopping Cluster App"
	start-stop-daemon -K -x $DAEMON
	echo "."
	;;

  restart)
	echo -n "Restarting Cluster App"
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