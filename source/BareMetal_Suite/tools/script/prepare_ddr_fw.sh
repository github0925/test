#!/bin/bash

TOPDIR=$(pwd)

CFGs=" lpddr4x lpddr4 ddr4 ddr3 "

rm -rf build/ddr_fw
mkdir build/ddr_fw
chmod +w build/ddr_fw

make clean
make CFG=kunlun_sec D=1 elf
if [ $? -ne 0 ]; then
    echo "$0: !! Opps, build_kunlun_sec fail !!"
    exit 1
fi

for p in $CFGs
do

arm-none-eabi-objcopy -O binary ./driver/ddr/dw_umctl2/fw/${p}_pmu_train_1d_string.o -j .rodata.${p}_pmu_train_1d_string  build/ddr_fw/${p}_pmu_train_1d_string.bin.rodata
arm-none-eabi-objcopy -O binary ./driver/ddr/dw_umctl2/fw/${p}_pmu_train_1d_string.o -j .rodata build/ddr_fw/${p}_pmu_train_1d_string.bin.string

if [ "$p" = "ddr3" ]; then
	${TOPDIR}/tools/bin_packer/bin_packer if=${TOPDIR}/driver/ddr/dw_umctl2/fw/fw_release/${p}/1d/${p}_pmu_train_imem.bin if=${TOPDIR}/driver/ddr/dw_umctl2/fw/fw_release/${p}/1d/${p}_pmu_train_dmem.bin if=${TOPDIR}/build/ddr_fw/${p}_pmu_train_1d_string.bin.rodata if=${TOPDIR}/build/ddr_fw/${p}_pmu_train_1d_string.bin.string of=${TOPDIR}/build/ddr_fw/${p}_training_fw.pac
else
	arm-none-eabi-objcopy -O binary ./driver/ddr/dw_umctl2/fw/${p}_pmu_train_2d_string.o -j .rodata.${p}_pmu_train_2d_string  build/ddr_fw/${p}_pmu_train_2d_string.bin.rodata
	arm-none-eabi-objcopy -O binary ./driver/ddr/dw_umctl2/fw/${p}_pmu_train_2d_string.o -j .rodata build/ddr_fw/${p}_pmu_train_2d_string.bin.string

	${TOPDIR}/tools/bin_packer/bin_packer if=${TOPDIR}/driver/ddr/dw_umctl2/fw/fw_release/${p}/1d/${p}_pmu_train_imem.bin if=${TOPDIR}/driver/ddr/dw_umctl2/fw/fw_release/${p}/1d/${p}_pmu_train_dmem.bin if=${TOPDIR}/driver/ddr/dw_umctl2/fw/fw_release/${p}/2d/${p}_2d_pmu_train_imem.bin if=${TOPDIR}/driver/ddr/dw_umctl2/fw/fw_release/${p}/2d/${p}_2d_pmu_train_dmem.bin if=${TOPDIR}/build/ddr_fw/${p}_pmu_train_1d_string.bin.rodata if=${TOPDIR}/build/ddr_fw/${p}_pmu_train_1d_string.bin.string  if=${TOPDIR}/build/ddr_fw/${p}_pmu_train_2d_string.bin.rodata if=${TOPDIR}/build/ddr_fw/${p}_pmu_train_2d_string.bin.string of=${TOPDIR}/build/ddr_fw/${p}_training_fw.pac
fi

rm build/ddr_fw/*.string build/ddr_fw/*.rodata
done
