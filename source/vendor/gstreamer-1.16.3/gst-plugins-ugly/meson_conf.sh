#!/bin/sh

export SYSROOT=$SDK/aarch64-sdrv-linux
export TOOLCHAIN=$SDK/x86_64-sdrvsdk-linux/usr/bin/aarch64-sdrv-linux
export PKG_CONFIG_LIBDIR=$SYSROOT/usr/lib/pkgconfig:$SYSROOT/usr/share/pkgconfig
export PATH=$PATH:$TOOLCHAIN

EXTRA_OEMESON="\
"

if [ -f $ROOTINI ]; then
    rm $ROOTINI
fi

touch $ROOTINI
echo '[binaries]' > $ROOTINI
echo "sys_root = '$SYSROOT'" >> $ROOTINI
echo '' >> $ROOTINI
echo '[properties]' >> $ROOTINI
echo "sys_root = '$SYSROOT'" >> $ROOTINI

meson setup --prefix=/usr --buildtype plain --libdir lib --cross-file $ROOTINI --cross-file cross.ini $EXTRA_OEMESON build

if [ -f build/build.ninja ]; then
    ninja -C build
    cd build
    export DESTDIR=$SYSROOT
    meson install
else
    echo 'ERROR: no build file found!'
fi
