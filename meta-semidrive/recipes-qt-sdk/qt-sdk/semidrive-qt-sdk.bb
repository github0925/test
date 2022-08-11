SUMMARY = "qt sdk"
LICENSE = "MIT"

#IMAGE_INSTALL_append = "pvr-lib qtbase \
# qtcanvas3d qtconnectivity qtdatavis3d \
# qt3d qtdeclarative qtgraphicaleffects qtimageformats \
# qtlocation qtmqtt qtmultimedia qtnetworkauth qtopcua \
# qtquickcontrols qtquickcontrols2 qtremoteobjects qtscxml \
# qtsensors qtserialbus qtserialport qtsvg qtsystems qttools \
# qttranslations qtvirtualkeyboard qtwebchannel qtwebengine qtwebglplugin \
# qtwebsockets qtwebview qtxmlpatterns \
#"

#IMAGE_INSTALL_append = "  gstreamer \
# alsa-utils  pulseaudio  packagegroup-core-eclipse-debug packagegroup-core-ssh-openssh \
# udev cpio sysvinit" 


#PACKAGES =. " packagegroup-core-ssh-openssh gstreamer \
# alsa-utils  pulseaudio  packagegroup-core-eclipse-debug packagegroup-core-ssh-openssh "


inherit core-image
inherit populate_sdk_qt5
