#!/bin/sh

REMOTE_AP_IP=172.20.1.3
# Check the connect status of internet
function check_ip_status()
{
    ping -c 1 -W 1 $1 &> /dev/null
    if [ $? -eq 0 ];then
        return 0
    else
        return -1
    fi
}

while true
do

check_ip_status ${REMOTE_AP_IP}

if [ $? -eq 0 ];then
	mount.nfs -o nolock,vers=2 ${REMOTE_AP_IP}:/data /mnt/data
	exit 0
fi

done

