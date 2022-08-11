#!/bin/sh

export SYSROOT=$SDK/sysroot
export TOOLCHAIN=$SDK/bin
export PKG_CONFIG_LIBDIR=$SYSROOT/usr/lib/pkgconfig:$SYSROOT/usr/share/pkgconfig
export PATH=$PATH:$TOOLCHAIN

EXTRA_OEMESON="\
    -Dexamples=disabled \
    -Dnls=enabled \
    -Daalib=disabled \
    -Ddirectsound=disabled \
    -Ddv=disabled \
    -Dlibcaca=disabled \
    -Doss4=disabled \
    -Dosxaudio=disabled \
    -Dosxvideo=disabled \
    -Dshout2=disabled \
    -Dtwolame=disabled \
    -Dwaveform=disabled \
    -Dbz2=enabled \
    -Ddv1394=disabled \
    -Dgtk3=disabled \
    -Djack=disabled \
    -Dv4l2-libv4l2=disabled \
    -Dqt5=disabled \
    -Dv4l2=enabled \
    -Dv4l2-probe=true \
    -Dvpx=disabled \
    -Dwavpack=disabled \
    -Dximagesrc=disabled \
    -Dximagesrc-xshm=disabled \
    -Dximagesrc-xfixes=disabled \
    -Dximagesrc-xdamage=disabled
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
else
    echo 'ERROR: no build file found!'
fi
