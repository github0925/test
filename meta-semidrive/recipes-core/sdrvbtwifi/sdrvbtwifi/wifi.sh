echo "config for wifi"
#insmod brcmutil
#insmod brcmfmac
ifconfig wlan0 up;
wpa_passphrase mi9 12345678 > /data/wpa_supplicant.conf
wpa_supplicant -i wlan0 -c /etc/wpa_supplicant.conf -B
udhcpc -i wlan0

