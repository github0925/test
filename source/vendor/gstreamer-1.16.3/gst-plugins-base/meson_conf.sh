#!/bin/sh

export SYSROOT=$SDK/sysroot
export TOOLCHAIN=$SDK/bin
export PKG_CONFIG_LIBDIR=$SYSROOT/usr/lib/pkgconfig:$SYSROOT/usr/share/pkgconfig
export PATH=$PATH:$TOOLCHAIN


EXTRA_OEMESON="\
    -Dgtk_doc=disabled \
    -Dexamples=disabled \
    -Ddbghelp=disabled \
    -Dnls=enabled \
    -Dgl-graphene=disabled \
    -Dgl_api=gles2 \
    -Dgl_platform=egl \
    -Dgl_winsys=wayland \
    -Dfbdev=enabled \
    -Dkms=enabled \
    -Dcdparanoia=disabled \
    -Dgl-jpeg=enabled \
    -Dopus=disabled \
    -Dtremor=disabled \
    -Dlibvisual=disabled \
    -Dx11=disabled \
    -Dxvideo=disabled \
    -Dxshm=disabled
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
