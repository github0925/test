#!/bin/sh
#
# Semidrive Yocto Project Build Environment Setup Script
#
# Copyright (C) 2020 Semidrive Semiconductor
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

. meta-semidrive/scripts/setup-utils.sh

CWD=`pwd`
PROGNAME="setup-environment"
exit_message ()
{
   echo "To return to this build environment later please run:"
   echo "    source setup-environment <build_dir>"

}

usage()
{
    echo -e "\nUsage: source sd-setup-release.sh
    Optional parameters: [-b build-dir] [-h]"
echo "
    * [-b build-dir]: Build directory, if unspecified script uses 'build' as output directory
    * [-h]: help
"
}


clean_up()
{

    unset CWD BUILD_DIR SMDVDISTRO BUILD_DIR
    unset sd_setup_help sd_setup_error sd_setup_flag
    unset usage clean_up
    unset ARM_DIR META_FSL_BSP_RELEASE
    exit_message clean_up
}

# get command line options
OLD_OPTIND=$OPTIND
unset SMDVDISTRO
unset BUILD_DIR

while getopts "k:r:t:b:e:gh" sd_setup_flag
do
    case $sd_setup_flag in
        b) BUILD_DIR="$OPTARG";
           echo -e "\n Build directory is " $BUILD_DIR
           ;;
        h) sd_setup_help='true';
           ;;
        \?) sd_setup_error='true';
           ;;
    esac
done
shift $((OPTIND-1))
if [ $# -ne 0 ]; then
    sd_setup_error=true
    echo -e "Invalid command line ending: '$@'"
fi
OPTIND=$OLD_OPTIND
if test $sd_setup_help; then
    usage && clean_up && return 1
elif test $sd_setup_error; then
    clean_up && return 1
fi


if [ -z "$DISTRO" ]; then
    if [ -z "$SMDVDISTRO" ]; then
        SMDVDISTRO='minimal-console'
    fi
else
    SMDVDISTRO="$DISTRO"
fi

if [ -z "$BUILD_DIR" ]; then
    BUILD_DIR='build'
fi

if [ -z "$BL_TYPE" ]; then
    echo setting to default bootloader type: lk
    BL_TYPE='lk'
fi

if [ -z "$MACHINE" ]; then
    echo setting to default machine
    MACHINE='x9h_evb'
fi
echo machine=$MACHINE distro=$SMDVDISTRO BUILD_DIR=$BUILD_DIR
case $MACHINE in
x9*)
    case $SMDVDISTRO in
	*console)
        : ok
        ;;
    esac
    ;;
g9*)
    case $SMDVDISTRO in
    gateway*)
		: ok
		;;
    *)
        echo -e "\n ERROR - G9 must use gateway distro."
        echo -e "\n Set DISTRO=gateway in command line."
        echo -e "\n"
        return 1
        ;;
    esac
    ;;
*)
    : ok
    ;;
esac

# copy new EULA into community so setup uses latest Semidrive EULA
cp meta-semidrive/EULA.txt meta-semidrive/EULA

# Set up the basic yocto environment
if [ -z "$DISTRO" ]; then
   DISTRO=$SMDVDISTRO MACHINE=$MACHINE . ./$PROGNAME $BUILD_DIR
else
   MACHINE=$MACHINE . ./$PROGNAME $BUILD_DIR
fi

# Point to the current directory since the last command changed the directory to $BUILD_DIR
BUILD_DIR=.

if [ ! -e $BUILD_DIR/conf/local.conf ]; then
    echo -e "\n ERROR - No build directory is set yet. Run the 'setup-environment' script before running this script to create " $BUILD_DIR
    echo -e "\n"
    return 1
fi

# On the first script run, backup the local.conf file
# Consecutive runs, it restores the backup and changes are appended on this one.
if [ ! -e $BUILD_DIR/conf/local.conf.org ]; then
    cp $BUILD_DIR/conf/local.conf $BUILD_DIR/conf/local.conf.org
else
    cp $BUILD_DIR/conf/local.conf.org $BUILD_DIR/conf/local.conf
fi


if [ ! -e $BUILD_DIR/conf/bblayers.conf.org ]; then
    cp $BUILD_DIR/conf/bblayers.conf $BUILD_DIR/conf/bblayers.conf.org
else
    cp $BUILD_DIR/conf/bblayers.conf.org $BUILD_DIR/conf/bblayers.conf
fi


echo "BOARD_VERSION ?= \"${BOARD_VERSION}\"" >> $BUILD_DIR/conf/local.conf
echo "" >> $BUILD_DIR/conf/bblayers.conf
echo "# Semidrive Yocto Project Release layers" >> $BUILD_DIR/conf/bblayers.conf

echo "" >> $BUILD_DIR/conf/bblayers.conf
echo "BBLAYERS += \" \${BSPDIR}/meta-openembedded/meta-networking \"" >> $BUILD_DIR/conf/bblayers.conf
echo "BBLAYERS += \" \${BSPDIR}/meta-openembedded/meta-python \"" >> $BUILD_DIR/conf/bblayers.conf
echo "BBLAYERS += \" \${BSPDIR}/meta-openembedded/meta-filesystems \"" >> $BUILD_DIR/conf/bblayers.conf
echo "BBLAYERS += \" \${BSPDIR}/meta-qt5 \"" >> $BUILD_DIR/conf/bblayers.conf

echo BSPDIR=$BSPDIR
echo BUILD_DIR=$BUILD_DIR
export YOCTO_MACHINE=$MACHINE
export YOCTO_DISTRO=$SMDVDISTRO
export YOCTO_BL_TYPE=$BL_TYPE

# Support integrating community meta-semidrive
if [ -d ../meta-semidrive ]; then
    echo meta-semidrive directory found
    # Change settings according to environment
fi

cd  $BUILD_DIR
clean_up
unset SMDVDISTRO
