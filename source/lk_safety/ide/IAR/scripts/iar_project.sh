#!/bin/bash
# Generate IAR projects for examples for the available variants

set -e

### Arguments parsing
DIR="$1"
TOP="$2"
IBINNAME="$3"
WORKSPACE_TEMPLATE="$4"
PROJECT_TEMPLATE="$5"
DEBUG_TEMPLATE="$6"
IAR_INFO_CENTER_STARTUP_TEMPLATE="$7"
IAR_INFO_CENTER_EXAMPLE_TEMPLATE="$8"
IAR_INFO_CENTER_EWINFO_TEMPLATE="$9"
IAR_OUT="${10}"

### Global variable

tpl-get-execu(){
    local tmp_exe="$1"
    ITARGET=$(cat ${tmp_exe})
    AVAILABLE_VARIANTS=${ITARGET}
    rm -f "$DIR/${tmp_exe}"
}

tpl-split() {
    local template_src="$1"
    local template="$2"
    local nod="$3"
    local head
    local tail

    echo "SPLIT $template_src"

    head=$(sed -n "/<${nod}>/=" "${template_src}" | sed -n '1p')
    tail=$(sed -n "/<\/${nod}>/=" "${template_src}" | sed -n '$p')

    # head: from start of file to <configuration> (excluded)
    sed -n "1,`expr ${head} - 1`p" "${template_src}" > "$IAR_OUT/${template}.head"
    # body: from <configuration> to </configuration> (both included)
    sed -n "${head},${tail}p" "${template_src}" > "$IAR_OUT/${template}.body"
    # tail: from </configuration> (excluded) to end of file
    sed -n "`expr ${tail} + 1`,\$p" "${template_src}" > "$IAR_OUT/${template}.tail"
}

helper-use-windows-path() {
    local str=${1//\/\//\/}
    echo ${str//\//\\\\}
}

tpl-finalize() {
    local tpl="$1"

    sed -i -e "s/\(__REPLACE_DEP_LIST__\|__REPLACE_PROJECT_FILES__\)//g" "$tpl"
}

find-source() {
    local dir="$1"
    local obj="$2"
    local path=${obj//.o/.c}
    if [ ! -f "$dir$path" ]; then
        path=${obj//.o/.s}

        if [ ! -f "$dir/$path" ]; then

            echo "Source file for $obj not found!" 1>&2
            exit 3
        fi
    fi

}

tpl-set-prj-files() {
    local tpl="$1"
    local section="$2"; shift; shift
    local input="$*"
    local temp

    local tmpxml=`mktemp -p "$IAR_OUT"`
    (
        for path in ${input}; do
            temp=$(find-source "$DIR" "$path")
            local win_path=$(helper-use-windows-path "$path")
            #echo -e "  <file><name>\$PROJ_DIR\$\\\\..\\\\..\\\\$win_path</name></file>"
            echo -e "  <file><name>\$PROJ_DIR\$\\\\..\\\\..\\\\$win_path</name></file>"
        done
    ) > "$tmpxml"

    sed -i -e "/__REPLACE_PROJECT_FILES__/r $tmpxml" "$tpl"
    rm -f "$tmpxml" 2>&1 > /dev/null
}

tpl-set-deps() {
    local tpl="$1"
    local section="$2"; shift; shift

    local input=$*
    local empty_group=true
    local tmpxml=`mktemp -p "$IAR_OUT"`
    local temp

    # Read file and fill input array
    (

        for path in ${input}; do
            if [ "${section}"  != "freertos" ]; then

                temp=$(find-source "$DIR" "$path")

            else
                if [ ! -d "$DIR/../freertos/FreeRTOS" ]; then
                    tmpsrc=${path#*../../}
                    path="../FreeRTOS/"$tmpsrc
                else
                    tmpsrc=${path#*../}
                    path="../"$tmpsrc
                fi
            fi


            if [ "$empty_group" = true ]
            then
                empty_group=false
                echo -e "  <group>\n    <name>$section</name>"
            fi

            local win_path=$(helper-use-windows-path "$path")
            echo -e "    <file><name>\$PROJ_DIR\$\\\\..\\\\..\\\\..\\\\$win_path</name></file>"
        done

        if [ "$empty_group" = false ]
        then
            echo -e "  </group>"
        fi
    ) > "$tmpxml"

    sed -i -e "/__REPLACE_DEP_LIST__/r $tmpxml" "$tpl"

    rm -f "$tmpxml" 2>&1 > /dev/null
}

tpl-set-version() {
    local tpl="$1"
    sed -i -e "s/__REPLACE_VERSION__/$VERSION/g" "$tpl"
}

tpl-finalize() {
    local tpl="$1"

    sed -i "s/\(__REPLACE_DEP_LIST__\|__REPLACE_PROJECT_FILES__\)//g" "$tpl"
}

tpl-set-defines() {
    local tpl="$1"
    local tmpxml=`mktemp -p "$IAR_OUT"`

    for flag in $CFLAGS_DEFS; do
        flag=${flag//:/ }
        echo "          <state>${flag//-D/}</state>"
    done > "$tmpxml"

    sed -i -e "/__REPLACE_DEFINES__/r $tmpxml" "$tpl"
    sed -i -e "s/__REPLACE_DEFINES__//g" "$tpl"

    rm -f "$tmpxml" 2>&1 > /dev/null
}

tpl-set-includes() {
    local tpl="$1"
    local tmpxml=`mktemp -p "$IAR_OUT"`

    (
        for include in $CFLAGS_INC; do
            local inc=$(helper-use-windows-path "${include//-I/}")
            echo -e "          <state>\$PROJ_DIR\$\\\\..\\\\..\\\\..\\\\$inc</state>"
        done
        echo -e "          <state>\$PROJ_DIR\$</state>"
    ) > "$tmpxml"

    sed -i -e "/__REPLACE_INCLUDES__/r $tmpxml" "$tpl"
    sed -i -e "s/__REPLACE_INCLUDES__//g" "$tpl"

    rm -f "$tmpxml" 2>&1 > /dev/null
}

tpl-set-preincludes() {
    local tpl="$1"
    local tmpxml=`mktemp -p "$IAR_OUT"`

    echo -e "          <state>\$PROJ_DIR\$\\\\iar_config.h</state>" > "$tmpxml"

    sed -i -e "/__REPLACE_PREINCLUDES__/r $tmpxml" "$tpl"
    sed -i -e "s/__REPLACE_PREINCLUDES__//g" "$tpl"

    rm -f "$tmpxml" 2>&1 > /dev/null
}

tpl-set-linker-script() {
    local tpl="$1"
    local linker_script="$2"
    if [ ! -f $linker_script ]; then
        echo "File $linker_script not found!" 1>&2
        exit 3
    fi

    local win_path=$(helper-use-windows-path "$linker_script")
    sed -i -e "s%__REPLACE_LINK_SCRIPT__%\$PROJ_DIR\$\\\\..\\\\..\\\\..\\\\$win_path%g" "$tpl"
}

tpl-set-link-lib() {
    local tpl="$1"
    local ilinklib="$2"

    sed -i -e "s%__REPLACE_IlinkAdditionalLibs__%\$PROJ_DIR\$\\\\..\\\\..\\\\..\\\\$2%g" "$tpl"
}
tpl-set-program-entry() {
    local tpl="$1"
    local entry="$2"

    sed -i -e "s%__REPLACE_ENTRY__%$entry%g" "$tpl"
}

tpl-set-binary-name() {
    local tpl="$1"
    local binary_name="$2"

    if [ -z $binary_name ]; then
        echo binary_name not defined! 1>&2
        exit 3
    fi

    sed -i -e "s/__REPLACE_BIN_NAME__/$binary_name/g" "$tpl"
}

tpl-set-configuration() {
    local tpl="$1"
    local target=$2
    local variant=$3

    echo "SET target=$target, variant=$variant"
    sed -i "s/__REPLACE_CONF__/$variant/g" "$tpl"
    sed -i "s/__REPLACE_TARGET__/$target/g" "$tpl"
}

tpl-set-chip() {
    local tpl="$1"
    local chip=

    for flag in $CFLAGS_DEFS; do
        if [ $flag == "ARM_CPU_cortex-r5=1" ]; then
            chip="Cortex-R5"
        fi
    done

    echo "SET CHIP=$chip"
    sed -i "s/__REPLACE_CHIP__/$chip/g" "$tpl"
}

generate-bodies-ewd() {
    local file="$1"
    local variant="$2"
    local tpl="$IAR_OUT/${file}_$variant.ewd"

    echo "GEN temporary file ${file}_$variant.ewd"

    cat "$IAR_OUT/iar_debug.body" > "$tpl"
    local win_path=$(helper-use-windows-path "$iar_debug_script_y")
    sed -i -e "s%__REPLACE_MACFILE__%\$PROJ_DIR\$\\\\..\\\\..\\\\..\\\\$win_path%g" "$tpl"
    sed -i -e "s%//%/%g" "$tpl"

    if [ -e "$iar_flashloader_script_y" ]; then
        sed -i -e "s%__REPLACE_FLASH_LOADER__%1%g" "$tpl"
        local win_path=$(helper-use-windows-path "$iar_flashloader_script_y")
        sed -i -e "s%__REPLACE_FLASH_LOADER_V3__%\$PROJ_DIR\$\\\\..\\\\..\\\\..\\\\$win_path%g" "$tpl"
    fi

    tpl-set-configuration "$tpl" $IBINNAME $variant
}

generate-ewd() {
    local file="$1"
    local tpl="$IAR_OUT/${file}_$ITARGET.ewd"
    if [ -f "$tpl" ]; then
        rm -f $tpl
    fi

    tpl-split "$DEBUG_TEMPLATE" iar_debug configuration

    rm -f "$IAR_OUT/$file.ewd.bodies"
    for variant in $AVAILABLE_VARIANTS; do
        if [ -f "$IAR_OUT/.env-$variant.sh" ]; then
            . "$IAR_OUT/.env-$variant.sh"
            generate-bodies-ewd "$file" $variant
        fi
    done

    # Remove temprary files
    rm -f "$IAR_OUT/$file.ewd.bodies"
    touch "$IAR_OUT/$file.ewd.bodies"
    for variant in $AVAILABLE_VARIANTS; do
        cat "$IAR_OUT/${file}_$variant.ewd" >> "$IAR_OUT/$file.ewd.bodies"
        rm -f "$IAR_OUT/${file}_$variant.ewd"
    done

    tpl-set-chip "$IAR_OUT/$file.ewd.bodies"

    echo "GEN ${file}_$ITARGET.ewd"
    cat "$IAR_OUT/iar_debug.head" > "$tpl"
    rm -f "$IAR_OUT/iar_debug.head"
    cat "$IAR_OUT/$file.ewd.bodies" >> "$tpl"
    rm -f "iar_debug.body"
    rm -f "$IAR_OUT/$file.ewd.bodies"
    cat "$IAR_OUT/iar_debug.tail" >> "$tpl"
    rm -f "$IAR_OUT/iar_debug.tail"
}

generate-bodies-ewp() {
    local file="$1"
    local variant="$2"
    local tpl="$IAR_OUT/${file}_$variant.ewp"

    echo "GEN temporary file ${file}_$variant.ewp"
    cat "$IAR_OUT/iar_project.body" > "$tpl"
    #tpl-set-defines       "$tpl"
    tpl-set-includes      "$tpl"
    tpl-set-preincludes   "$tpl"
    tpl-set-linker-script "$tpl" "$iar_linker_script_y"
    tpl-set-program-entry "$tpl" "$iar_program_entry_y"
    tpl-set-binary-name   "$tpl" "$file"
    tpl-set-configuration "$tpl" $IBINNAME $variant
    tpl-set-link-lib      "$tpl" "$iar_LinkAdditionalLibs_y"
}

generate-ewp() {
    local file="$1"
    local tpl="$IAR_OUT/${file}_${ITARGET}.ewp"
    if [ -f ${tpl} ]; then
        rm -f ${tpl}
    fi
    tpl-split "$PROJECT_TEMPLATE" iar_project configuration

    for variant in $AVAILABLE_VARIANTS; do
        echo "GEN ${file}_$variant.ewp"
        if [ -f "$IAR_OUT/.env-$variant.sh" ]; then
            . "$IAR_OUT/.env-$variant.sh"
            generate-bodies-ewp "$file" $variant
        fi
    done
    # Remove temporary files after merging them
    rm -f "$IAR_OUT/$file.ewp.bodies"
    touch "$IAR_OUT/$file.ewp.bodies"
    for variant in $AVAILABLE_VARIANTS; do
        cat "$IAR_OUT/${file}_$variant.ewp" >> "$IAR_OUT/$file.ewp.bodies"
        rm -f "$IAR_OUT/${file}_$variant.ewp"
    done

    echo "GEN ${file}_$ITARGET.ewp"
    cat "$IAR_OUT/iar_project.head" >> "$tpl"
    rm -f "$IAR_OUT/iar_project.head"
    cat "$IAR_OUT/$file.ewp.bodies" >> "$tpl"
    rm -f "$IAR_OUT/iar_project.body"
    rm -f "$IAR_OUT/$file.ewp.bodies"
    cat "$IAR_OUT/iar_project.tail" >> "$tpl"
    rm -f "$IAR_OUT/iar_project.tail"
    tpl-set-deps      "$tpl" "top"      "$top_y"
    tpl-set-deps      "$tpl" "arch"         "$arch_y"
    tpl-set-deps      "$tpl" "chipcfg"          "$chipcfg_y"
    tpl-set-deps      "$tpl" "chipdev"       "$chipdev_y"
    tpl-set-deps      "$tpl" "lib"      "$lib_y"
    #tpl-set-deps      "$tpl" "fatfs"      "$fatfs_y"
    tpl-set-deps      "$tpl" "freertos"        "$freertos_y"
    tpl-set-deps      "$tpl" "application"        "$application_y"
    tpl-set-deps      "$tpl" "exdev"          "$exdev_y"
    tpl-set-deps      "$tpl" "framework"           "$framework_y"
    tpl-set-deps      "$tpl" "hal"           "$hal_y"
    tpl-set-deps      "$tpl" "kernel"      "$kernel_y"
    tpl-set-deps      "$tpl" "platform"           "$platform_y"

    tpl-set-deps      "$tpl" "target"      "$target_y"
    if [ -n "$fatfs_y" ] ;then
        tpl-set-deps      "$tpl" "fatfs"      "$fatfs_y"
    fi
    if [ -n "$erpc_y" ] ;then
        tpl-set-deps      "$tpl" "erpc"      "$erpc_y"
    fi
    if [ -n "$libmetal_y" ] ;then
        tpl-set-deps      "$tpl" "libmetal"      "$libmetal_y"
    fi
    if [ -n "$littlevgl_y" ] ;then
        tpl-set-deps      "$tpl" "littlevgl"      "$littlevgl_y"
    fi
    if [ -n "$rpmsg_lite_y" ] ;then
        tpl-set-deps      "$tpl" "rpmsg-lite"      "$rpmsg_lite_y"
    fi

    tpl-set-chip      "$tpl"
    #tpl-set-prj-files "$tpl" "project_files" "$top_y"
    tpl-finalize      "$tpl"
}


generate-bodies-eww() {
    local file="$1"
    local target="$2"
    local tpl="$IAR_OUT/iar_workspace_$target.eww"

    echo "GEN temporary file iar_workspace_$target.eww"
    cat "$file" > "$tpl"
}

generate-eww() {
    local file=$1
    local tpl="$IAR_OUT/${file}.eww"
    if [ -f $tpl ]; then
        rm -f $tpl
    fi
    if [ -f "$IAR_OUT/${file}.eww" ]; then
       echo "UPD ${file}.eww"
       tpl-split "$WORKSPACE_TEMPLATE" iar_workspace project
       generate-bodies-eww "$IAR_OUT/iar_workspace.body" $ITARGET
       rm -f "$IAR_OUT/iar_workspace.head"
       rm -f "$IAR_OUT/iar_workspace.body"
       rm -f "$IAR_OUT/iar_workspace.tail"

       tpl-split "$tpl" iar_workspace project
       cat "$IAR_OUT/iar_workspace.head" > "$tpl"
       rm -f "$IAR_OUT/iar_workspace.head"
       cat "$IAR_OUT/iar_workspace.body" >> "$tpl"
       sed -i "s%__REPLACE_WORKSPACE_FILE__%\$WS_DIR\$/${file}_$ITARGET.ewp%g" "$IAR_OUT/iar_workspace_$ITARGET.eww"
       cat "$IAR_OUT/iar_workspace_$ITARGET.eww" >> "$tpl"
       rm -f "$IAR_OUT/iar_workspace.body"
       rm -f "$IAR_OUT/iar_workspace_$ITARGET.eww"
       cat "$IAR_OUT/iar_workspace.tail" >> "$tpl"
       rm -f "$IAR_OUT/iar_workspace.tail"

    else
       echo "GEN ${file}_$ITARGET.eww"
       sed -e "s%__REPLACE_WORKSPACE_FILE__%\$WS_DIR\$/${file}_$ITARGET.ewp%g" < "$WORKSPACE_TEMPLATE" |
       sed -e "s%//%/%g" > "$IAR_OUT/${file}.eww"
    fi
}

generate-infocenter() {
    local tpl="$DIR/top/main.c"
    #local tp2="$IAR_OUT/main.purpose"
    local main_purpose=`mktemp -p "$IAR_OUT"`
    local main_desc=`mktemp -p "$IAR_OUT"`
    #local tp3="$IAR_OUT/main.desc"
    local tp4="$IAR_OUT/StartupScreen.ewsample"
    local tp5="$IAR_OUT/ExampleInfo.ENU.xml"
    local tp6="$IAR_OUT/ewinfo.ENU.html"

    if [ ! -f "$tp4" -a -e "$tpl" ]; then
        cat "$IAR_INFO_CENTER_STARTUP_TEMPLATE" > "$tp4"
        sed -n '/section Purpose/,/section Requirements$/p' $tpl | grep -Ev '(section Purpose|section Requirements$)' | cut -f 1,2 >$main_purpose
        sed -i '/^$/d' $main_purpose
        sed -i 's/\ \*//g' $main_purpose

        sed -n '/section Description/,/section Usage$/p' $tpl | grep -Ev '(section Description|section Usage$)' | cut -f 1,2 >$main_desc
        sed -i '/^$/d' $main_desc
        sed -i 's/\ \*//g' $main_desc
        cat "$IAR_INFO_CENTER_EXAMPLE_TEMPLATE" > "$tp5"

        sed -i "s/__REPLACE_EXAMPLE_NAME__/$BINNAME/g" "$tp5"
        sed -i "/<iar_description>/r $main_purpose" "$tp5"
        cat "$IAR_INFO_CENTER_EWINFO_TEMPLATE" > "$tp6"
        sed -i "/Description:/r $main_desc" "$tp6"

    fi
    rm -f "$main_purpose"
    rm -f "$main_desc"
}
tpl-get-execu ".env-IEXE.sh"
generate-ewp $IBINNAME
generate-eww $IBINNAME
generate-ewd $IBINNAME

generate-infocenter

for variant in $AVAILABLE_VARIANTS; do
    rm -f "$IAR_OUT/.env-$variant.sh"
done

exit 0
