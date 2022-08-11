echo "running sdcam"
sleep 5

if [[ -f  /dev/video-evs0 ]];then
	csi-test 0 -evs0 &
else
	csi-test-gl 0 1 &
	vdsp-test 0 -dvr1 1920 1080&
fi

