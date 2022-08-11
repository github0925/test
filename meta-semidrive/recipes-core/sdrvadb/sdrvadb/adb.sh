echo "config for adb"
mount -t configfs none /config
mkdir /config/usb_gadget/g1

echo 0x18d1 > /config/usb_gadget/g1/idVendor
echo 0x4ee7 > /config/usb_gadget/g1/idProduct
mkdir /config/usb_gadget/g1/strings/0x409

echo 0123459876 > /config/usb_gadget/g1/strings/0x409/serialnumber
echo semidriver > /config/usb_gadget/g1/strings/0x409/manufacturer
echo x9 > /config/usb_gadget/g1/strings/0x409/product

mkdir /config/usb_gadget/g1/configs/b.1
mkdir /config/usb_gadget/g1/configs/b.1/strings/0x409
echo "adb" > /config/usb_gadget/g1/configs/b.1/strings/0x409/configuration
echo 120 > /config/usb_gadget/g1/configs/b.1/MaxPower

mkdir /config/usb_gadget/g1/functions/ffs.adb
ln -s /config/usb_gadget/g1/functions/ffs.adb /config/usb_gadget/g1/configs/b.1/ffs.adb

mkdir /dev/usb-ffs
mkdir /dev/usb-ffs/adb
mount -o uid=2000,gid=2000 -t functionfs adb /dev/usb-ffs/adb

chmod 666 /config/usb_gadget/g1/UDC

echo "sleep for run adbd"
/usr/bin/adbd &
echo "adbd done"
sleep 1
echo "31220000.dwc3" > /config/usb_gadget/g1/UDC

