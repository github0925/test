#!/bin/bash
ALLSRCFILE="$1"
TOP="$2"
CFLAGS_DEFS="$3"
CFLAGS_INC="$4"
LDFLAGS="$5"
CIAR_PROG_ENTRY="$6"
IAR_OUT="$7"
ICFTARGET="$8"
IEXE_PLACE="$9"
CIAR_LINKER=""
CIAR_DEBUGGER=""
CIAR_FLASHLOADER=""
#CLinkAdditionalLibs="hal/disp_hal/sd_disp_hal/lib/libdisp_link.a"

fRenamedSrc=""
fAsmSrc=""

getAsmSrc(){
    local parasrc="$1"
    for src in ${parasrc}; do
        postfix=${src##*.}
        if [ ${postfix} == "S" ]; then
            src=$(dirname $src)"/toolchain/iar/"$(basename $src)
        fi
        fAsmSrc="$fAsmSrc $src"
    done
}

# duplicate file in different folder ,failed to generate *pbi in same obj folder
renamesrc(){
    local tmpsrc
    local tmpfile
    local result
    local alla
    local parasrc="$1"
    for src in ${parasrc}; do
        tmpsrc=$(basename $src)
        if [[ $tmpfile =~ $tmpsrc ]]; then
            arr=($tmpfile)
            for ele in ${arr[*]}; do
                if [ $tmpsrc == $ele ]; then
                #if [ ${#tmpsrc} == ${#ele} ]; then
                dirsrc=$(dirname $src)
             lstdir=${dirsrc##*/}
             tmpsrc=$lstdir"_"$tmpsrc
             cp $src $dirsrc"/"$tmpsrc
             src=$dirsrc"/"$tmpsrc
             fi
                done
        fi
        tmpfile="$tmpfile $tmpsrc"
        fRenamedSrc="$fRenamedSrc $src"
    done
}

parse3rdsrc(){
    local para="$1"
    local thrdsrc
    local tmpsrc

    thrdsrc=${para#3rd/}
    pos=`expr index $thrdsrc /`
    tmpsrc=${thrdsrc:0:pos-1}
    case $tmpsrc in
        "ComputerLibrary") ComputerLibrary_y="$ComputerLibrary_y $src"
        ;;
        "erpc") erpc_y="$erpc_y $para"
        ;;
        "fatfs") fatfs_y="$fatfs_y $para"
        ;;
        "libmetal") libmetal_y="$libmetal_y $para"
        ;;
        "littlevgl") littlevgl_y="$littlevgl_y $para"
        ;;
        "rpmsg-lite") rpmsg_lite_y="$rpmsg_lite_y $para"
        ;;
    esac
}

parsesrc() {
    local pos
    local tmpsrc
    local para="$1"
    for src in ${para}; do
        pos=`expr index $src /`
        if [ "${pos}" == "1" ]; then
            freertos_y="$freertos_y $src"
            continue
        fi
        tmpsrc=${src:0:pos-1}
        case $tmpsrc in
            "target") target_y="$target_y $src"
            ;;
            "application") application_y="$application_y $src"
            ;;
            "arch") arch_y="$arch_y $src"
            ;;
            "chipcfg") chipcfg_y="$chipcfg_y $src"
            ;;
            "chipdev") chipdev_y="$chipdev_y $src"
            ;;
            "exdev") exdev_y="$exdev_y $src"
            ;;
            "hal") hal_y="$hal_y $src"
            ;;
            "framework") framework_y="$framework_y $src"
            ;;
            "kernel") kernel_y="$kernel_y $src"
            ;;
            "lib") lib_y="$lib_y $src"
            ;;
            "platform") platform_y="$platform_y $src"
            ;;
             "top") top_y="$top_y $src"
            ;;
             "3rd") parse3rdsrc "$src"
            ;;
        esac

    done
}

handleLib(){
    local flag="$1"
    local path
    local lib

    for item in ${flag}; do
        if [[ ${item} == -L* ]]; then
            path=${item:2}
        elif [[ ${item} == -l* ]]; then
            lib=$path"/lib"${item:2}".a"
            lib_y="$lib_y $lib"
        fi
    done
}

# Get Execu place
getExecuPlace(){

    IEXEVAR="$1"

    if [ -n "$IEXEVAR" ]; then
        IVARIANT=$IEXEVAR
        touch .env-IEXE.sh
        echo $IVARIANT     >> .env-IEXE.sh
    fi
}

getScript(){
    local iexe="$1"
    if [ ${iexe} = "iram" ];then
        CIAR_LINKER="ide/IAR/linkcfg/iram.icf"
        CIAR_DEBUGGER="ide/IAR/linkcfg/iram.mac"
    elif [ ${iexe} = "norflash" ];then
        CIAR_LINKER="ide/IAR/linkcfg/ospi.icf"
        CIAR_DEBUGGER="ide/IAR/linkcfg/ospi.mac"
    else
        CIAR_LINKER="target/$ICFTARGET/safety/IAR/ddr.icf"
        CIAR_DEBUGGER="target/$ICFTARGET/safety/IAR/ddr.mac"
        CIAR_FLASHLOADER="target/$ICFTARGET/safety/IAR/flashloader/iar_flashboardcfg.board"
    fi
}

#add freertos include
addRtosInc(){
    path1="../FreeRTOS/Source/include"
    path2="../FreeRTOS/Source/portable/GCC/ARM_CR5"
    CFLAGS_INC="$CFLAGS_INC $path1 $path2 ."
}

create(){
    addRtosInc
    getExecuPlace "${IEXE_PLACE}"
    getScript "${IVARIANT}"
    rm -f $IAR_OUT/.env-$IVARIANT.sh
    handleLib "$LDFLAGS"
    echo GEN Environment $IAR_OUT/.env-$IVARIANT.sh
    touch $IAR_OUT/.env-$IVARIANT.sh
    echo TOP=\"$TOP\"                                       >> $IAR_OUT/.env-$IVARIANT.sh
    echo CFLAGS_DEFS=\'$CFLAGS_DEFS\'                       >> $IAR_OUT/.env-$IVARIANT.sh
    echo CFLAGS_INC=\"$CFLAGS_INC\"                         >> $IAR_OUT/.env-$IVARIANT.sh
    echo iar_linker_script_y=\"$CIAR_LINKER\"               >> $IAR_OUT/.env-$IVARIANT.sh
    echo iar_debug_script_y=\"$CIAR_DEBUGGER\"              >> $IAR_OUT/.env-$IVARIANT.sh
    echo iar_flashloader_script_y=\"$CIAR_FLASHLOADER\"     >> $IAR_OUT/.env-$IVARIANT.sh
    #echo iar_LinkAdditionalLibs_y=\"$CLinkAdditionalLibs\"  >> $IAR_OUT/.env-$IVARIANT.sh
    echo iar_program_entry_y=\"$CIAR_PROG_ENTRY\"           >> $IAR_OUT/.env-$IVARIANT.sh
    echo application_y=\"$application_y\"                   >> $IAR_OUT/.env-$IVARIANT.sh
    echo arch_y=\"$arch_y\"                                 >> $IAR_OUT/.env-$IVARIANT.sh
    echo chipcfg_y=\"$chipcfg_y\"                           >> $IAR_OUT/.env-$IVARIANT.sh
    echo chipdev_y=\"$chipdev_y\"                           >> $IAR_OUT/.env-$IVARIANT.sh
    echo hal_y=\"$hal_y\"                                   >> $IAR_OUT/.env-$IVARIANT.sh
    echo exdev_y=\"$exdev_y\"                               >> $IAR_OUT/.env-$IVARIANT.sh
    echo framework_y=\"$framework_y\"                       >> $IAR_OUT/.env-$IVARIANT.sh
    echo kernel_y=\"$kernel_y\"                             >> $IAR_OUT/.env-$IVARIANT.sh
    echo platform_y=\"$platform_y\"                         >> $IAR_OUT/.env-$IVARIANT.sh
    echo target_y=\"$target_y\"                             >> $IAR_OUT/.env-$IVARIANT.sh
    echo lib_y=\"$lib_y\"                                   >> $IAR_OUT/.env-$IVARIANT.sh
    echo freertos_y=\"$freertos_y\"                         >> $IAR_OUT/.env-$IVARIANT.sh
    echo top_y=\"$top_y\"                                   >> $IAR_OUT/.env-$IVARIANT.sh
    if [ -n "$ComputerLibrary_y" ]; then
        echo ComputerLibrary_y=\"$ComputerLibrary_y\"       >> $IAR_OUT/.env-$IVARIANT.sh
    fi
    if [ -n "$erpc_y" ]; then
        echo erpc_y=\"$erpc_y\"                             >> $IAR_OUT/.env-$IVARIANT.sh
    fi
    if [ -n "$fatfs_y" ]; then
        echo fatfs_y=\"$fatfs_y\"                           >> $IAR_OUT/.env-$IVARIANT.sh
    fi
    if [ -n "$libmetal_y" ]; then
        echo libmetal_y=\"$libmetal_y\"                     >> $IAR_OUT/.env-$IVARIANT.sh
    fi
    if [ -n "$littlevgl_y" ]; then
        echo littlevgl_y=\"$littlevgl_y\"                   >> $IAR_OUT/.env-$IVARIANT.sh
    fi
    if [ -n "$rpmsg_lite_y" ]; then
        echo rpmsg_lite_y=\"$rpmsg_lite_y\"                 >> $IAR_OUT/.env-$IVARIANT.sh
    fi
}

#Replace GNU ASM with ARM ASM
getAsmSrc "$ALLSRCFILE"

#Rename duplicated file
renamesrc "$fAsmSrc"
parsesrc "$fRenamedSrc"
create

