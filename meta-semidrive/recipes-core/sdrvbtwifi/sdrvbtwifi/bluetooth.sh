echo "config for bt"
/usr/bin/brcm_patchram_plus_latest --no2bytes --tosleep 50000 --baudrate 115200 --use_baudrate_for_download --patchram /usr/lib/CYW89359B1_002.002.014.0153.0349.hcd /dev/ttyS1 && hciattach /dev/ttyS1 any 115200 flow && hciconfig hci0 up &
