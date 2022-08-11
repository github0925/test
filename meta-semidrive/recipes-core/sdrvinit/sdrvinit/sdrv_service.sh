#!/bin/sh

echo "This is a init script for semidrive SoC"

if [[ -f  /usr/bin/board_diag ]];then
    /usr/bin/board_diag &
fi

if [[ -f  /usr/bin/slt ]];then
    /usr/bin/slt &
fi

if [[ -f  /usr/bin/tee-supplicant ]];then
    /usr/bin/tee-supplicant &
fi

# resize ext2/ext4 part
if ! test -d "/dev/block/by-name/"
then
	echo "no part found"
	exit 0
else
	resize2fs /dev/block/by-name/userdata
fi

mount -t ext4 /dev/block/by-name/userdata /data
