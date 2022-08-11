#!/bin/bash

usage()
{
    echo -e "\nUsage: release_cut.sh
    Optional parameters: [-d yocto-root-dir] [-h]"
echo "
    * [-d yocto-root-dir]:   Yocto top directory, default:$YOCTO_TOP
    * [-c]:                  chip:X9,G9,D9 
    * [-h]:                  This help message
"
}

clean_up()
{
    unset release_cut_help release_cut_error make_pac_flag
    unset usage clean_up
}

# get command line options
OLD_OPTIND=$OPTIND

YOCTO_TOP=`realpath ../..`

CHIP_NAME=D9
while getopts "d:h:c" make_pac_flag
do
    case $make_pac_flag in
        d) YOCTO_TOP="$OPTARG";
           ;;
        c) CHIP_NAME="$OPTARG";
           ;;
        h) release_cut_help='true';
           ;;
        \?) release_cut_error='true';
           ;;
    esac
done

shift $((OPTIND-1))
if [ $# -ne 0 ]; then
    echo -e "Invalid command line ending: '$@'"
fi
OPTIND=$OLD_OPTIND
if test $release_cut_help; then
    usage && clean_up && exit 1
elif test $release_cut_error; then
    clean_up && exit 1
fi

cd ${YOCTO_TOP}

echo " $CHIP_NAME "

#Step 1,Remove .repo
echo 'Remove .repo dir...'
find . -type d -name ".repo" -exec rm -rf {} \;

cd ${YOCTO_TOP}
#Step 2,Remove .git
#echo 'Remove .git dir...'
#find . -type d -name ".git" -exec rm -rf {} \;

#Step 3,Remote lk_customize
cd ${YOCTO_TOP}/source/lk_customize
if [ "$CHIP_NAME" = X9 ]; then
RemoveFiles="
    chipcfg:epu_fpga
    chipcfg:x9_fpga
    chipcfg:x9_plus
    chipcfg:sdpe
    chipcfg:scripts
    chipcfg:*.xlsx
    target:reference_g9_sdpe
    target:reference_g9_ap
    target:reference_g9_safety
    target:reference_g9_secure
    target:fpga_ecockpit
    target:fpga_epu
    target:fpga_secure
    project:epu.mk
    project:qnx_ipl.mk
    project:sdpe-g9x.mk
    project:fpga_*.mk
    project:*_plus.mk
    platform:epu_fpga
    platform:x9_fpga
    platform:x9_plus
    platform:x9_sdpe
    application:qnx_ipl
    application:firewall
    application:mp
    application:sdpe
    "
elif [ "$CHIP_NAME" = D9 ]; then
echo "lk_cust D9------------"
RemoveFiles="
    chipcfg:epu_fpga
    chipcfg:x9_fpga
    chipcfg:x9_plus
    chipcfg:x9_*
    chipcfg:v9*
    chipcfg:g9*
    chipcfg:bf200
    chipcfg:sdpe
    chipcfg:scripts
    chipcfg:*.xlsx
    target:reference_g9_sdpe
    target:reference_g9_ap
    target:reference_g9_safety
    target:reference_g9_secure
    target:reference_v9*
    target:reference_x9*
    target:reference_g9*
    target:x9h_icl02
    target:evb
    target:bf200
    target:fpga
    target:ii4_g9
    target:ms_x9
    target:fpga_ecockpit
    target:fpga_epu
    target:fpga_secure
    project:epu.mk
    project:qnx_ipl.mk
    project:sdpe-g9x.mk
    project:fpga_*.mk
    project:*_plus.mk
    project:*v9*.mk
    project:*g9*.mk
    project:*x9*.mk
    project:*bf200*.mk
    platform:epu_fpga
    platform:x9_fpga
    platform:x9_plus
    platform:x9_sdpe
    platform:mp
    application:qnx_ipl
    application:firewall
    application:mp
    application:sdpe
    "
else
RemoveFiles="
    chipcfg:epu_fpga
    chipcfg:x9_fpga
    chipcfg:x9_plus
    chipcfg:sdpe
    chipcfg:scripts
    chipcfg:*.xlsx
    target:reference_g9_sdpe
    target:fpga_ecockpit
    target:fpga_epu
    target:fpga_secure
    project:epu.mk
    project:qnx_ipl.mk
    project:sdpe-g9x.mk
    project:fpga_*.mk
    project:*_plus.mk
    platform:epu_fpga
    platform:x9_fpga
    platform:x9_plus
    platform:x9_sdpe
    application:qnx_ipl
    application:firewall
    application:mp
    application:sdpe
    "

fi
echo $RemoveFiles
for f in $RemoveFiles
do
echo "Process "$f

prodir=`echo ${f} | cut -d: -f1`
profile=`echo ${f} | cut -d: -f2`

echo "prodir:${prodir}   profile:${profile}"
#find ${prodir} -name "${profile}" -exec rm -rf {} \;
find ${prodir} -name "${profile}" | xargs rm -rf
#find ${prodir} -name "${profile}"
done


cd ${YOCTO_TOP}/source/lk_safety
if [ "$CHIP_NAME" = X9 ]; then
RemoveFiles="
    chipdev:firewall
    chipcfg:epu_fpga 
    chipcfg:x9_fpga
    chipcfg:sdpe
    chipcfg:scripts
    chipcfg:*.xlsx
    target:reference_g9_sdpe
    target:fpga_ecockpit
    target:fpga_epu
    target:fpga_secure
    project:epu.mk
    platform:epu_fpga
    platform:sdpe
    platform:x9_fpga
    "
elif  [ "$CHIP_NAME" = D9 ]; then
RemoveFiles="
    chipdev:firewall
    chipcfg:epu_fpga 
    chipcfg:x9_fpga
    chipcfg:sdpe
    chipcfg:scripts
    chipcfg:*.xlsx
    target:reference_g9_sdpe
    target:fpga_ecockpit
    target:fpga_epu
    target:fpga_secure
    project:epu.mk
    project:*v9*.mk
    project:*g9*.mk
    project:*x9*.mk
    project:autosar.mk
    project:autoas.mk
    project:*bf200*.mk
    platform:epu_fpga
    platform:sdpe
    platform:x9_fpga
    "
else
RemoveFiles="
    chipdev:firewall
    chipcfg:epu_fpga
    chipcfg:x9_fpga
    chipcfg:sdpe
    chipcfg:scripts
    chipcfg:*.xlsx
    target:reference_g9_sdpe
    target:fpga_ecockpit
    target:fpga_epu
    target:fpga_secure
    project:epu.mk
    platform:epu_fpga
    platform:sdpe
    platform:x9_fpga
    platform:x9_plus
    "
fi
for f in $RemoveFiles
do
echo "Process "$f

prodir=`echo ${f} | cut -d: -f1`
profile=`echo ${f} | cut -d: -f2`

echo "prodir:${prodir}   profile:${profile}"
#find ${prodir} -name "${profile}" -exec rm -rf {} \;
find ${prodir} -name "${profile}" | xargs rm -rf
#find ${prodir} -name "${profile}"
done



cd ${YOCTO_TOP}/meta-semidrive/conf/
if [ "$CHIP_NAME" = X9 ]; then
RemoveFiles="
    "
elif  [ "$CHIP_NAME" = D9 ]; then
RemoveFiles="
    machine:g9*
    machine:v9*
    machine:x9*
    machine:bf200.conf
    "
else
RemoveFiles="
    "
fi
for f in $RemoveFiles
do
echo "Process "$f

prodir=`echo ${f} | cut -d: -f1`
profile=`echo ${f} | cut -d: -f2`

echo "prodir:${prodir}   profile:${profile}"
#find ${prodir} -name "${profile}" -exec rm -rf {} \;
find ${prodir} -name "${profile}" | xargs rm -rf
#find ${prodir} -name "${profile}"
done