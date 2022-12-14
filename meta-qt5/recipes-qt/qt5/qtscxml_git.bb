LICENSE = "LGPL-3.0 & BSD & GPL-3.0 & The-Qt-Company-GPL-Exception-1.0 | The-Qt-Company-Commercial"
LIC_FILES_CHKSUM = " \
    file://LICENSE.LGPL3;md5=e6a600fd5e1d9cbde2d983680233ad02 \
    file://LICENSE.GPL3-EXCEPT;md5=763d8c535a234d9a3fb682c7ecb6c073 \
    file://LICENSE.FDL;md5=6d9f2a9af4c8b8c3c769f6cc1b6aaf7e \
"

require qt5.inc
require qt5-git.inc

DEPENDS += "qtbase qtdeclarative qtxmlpatterns qtscxml-native"

SRCREV = "f416efc6e2ea2fff4357cd87f4458e009ac5d595"

# Patches from https://github.com/meta-qt5/qtscxml/commits/b5.12
# 5.12.meta-qt5.2
SRC_URI += "file://0001-Use-external-host-bin-path-for-cmake-file.patch"

do_install_append_class-nativesdk() {
    # qml files not needed in nativesdk
    rm -rf ${D}${OE_QMAKE_PATH_QML}
}

BBCLASSEXTEND =+ "native nativesdk"
