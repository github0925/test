#!/bin/sh

export SYSROOT=$SDK/sysroot
export TOOLCHAIN=$SDK/bin
export PKG_CONFIG_LIBDIR=$SYSROOT/usr/lib/pkgconfig:$SYSROOT/usr/share/pkgconfig
export PATH=$PATH:$TOOLCHAIN

EXTRA_OEMESON="\
    -Dexamples=disabled \
    -Dnls=enabled \
    -Dfbdev=enabled \
    -Dshm=enabled \
    -Dkms=enabled \
    -Dwayland=enabled \
    -Dgl=enabled \
    -Dipcpipeline=enabled \
    -Dshm=enabled \
    -Dbluez=disabled \
    -Dvulkan=disabled \
    -Dalios=enabled \
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
