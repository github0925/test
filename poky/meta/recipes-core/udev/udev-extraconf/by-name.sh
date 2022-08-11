#!/bin/sh
#
# Called from udev
#
# Attempt to add by-name link of block devices

lk_name="`basename "$PARTNAME"`"

if [ "$ACTION" = "add"  ] && [ -n "$PARTNAME"  ]; then
	mkdir -p "/dev/block/by-name/"
	ln -s $DEVNAME /dev/block/by-name/$lk_name
fi

if [ "$ACTION" = "remove"  ] && [ -n "$PARTNAME"  ]; then
        rm /dev/block/by-name/$lk_name
fi
