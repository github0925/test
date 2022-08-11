#!/bin/bash

function get_image_max_size()
{
    BPT_FILE=$1
    PARTITION_NAME=$2
    TYPE=$3
    MAX_SIZE=0

    if [[ $# != 3 ]];then
        echo "get_image_max_size error param:$#" > /dev/stderr
        return -1
    fi

    IMAGE_SIZE=`bpttool query_partition --input ${BPT_FILE} --label ${PARTITION_NAME} --type size`
    IMAGE_SIZE=`avbtool add_hash_footer --calc_max_image_size --partition_size ${IMAGE_SIZE}`
    MAX_SIZE=${IMAGE_SIZE}
    echo ${MAX_SIZE}
}

function get_partition_size()
{
    PARTITION_NAME=$1
    BPT_FILE=$2
    if [ $# != 2 ];then
        echo "get_partition_size error param:$#" > /dev/stderr
        return -1
    fi

    IMAGE_SIZE=`bpttool query_partition --input ${BPT_FILE} --label ${PARTITION_NAME} --type size`
    echo ${IMAGE_SIZE}
}

function image_add_hash_footer_no_bpt()
{
    IMAGE=$1
    PARTITION_NAME=$2
    PARTITION_SIZE=$3
    HASH_ALG=$4
    VBMETA_SIGNED_ARGS=$5
    if [ $# != 5 ];then
        echo "image_add_hash_footer_no_bpt error param:$#" > /dev/stderr
        return -1
    fi

    #avbtool erase_footer --image ${IMAGE}
    avbtool add_hash_footer --partition_name  ${PARTITION_NAME} --partition_size ${PARTITION_SIZE} --image ${IMAGE}  --hash_algorithm $HASH_ALG $VBMETA_SIGNED_ARGS

    DESCIPTOR=" --include_descriptors_from_image ${IMAGE} "
    echo ${DESCIPTOR}
}

function image_add_hash_footer()
{
    IMAGE=$1
    PARTITION_NAME=$2
    BPT_FILE=$3
    HASH_ALG=$4
    if [ $# != 4 ];then
        echo "image_add_hash_footer error param:$#" > /dev/stderr
        return -1
    fi

    VBMETA_BLOB_DIR=`dirname ${IMAGE}`
    VBMETA_BLOB_BASE=`basename ${IMAGE}`
    VBMETA_BLOB=${VBMETA_BLOB_DIR}/${VBMETA_BLOB_BASE}.vbmeta.blob

    PARTITION_SIZE=`bpttool query_partition --input ${BPT_FILE} --label ${PARTITION_NAME} --type size`
    #avbtool erase_footer --compact_footer --image ${IMAGE}
    avbtool add_hash_footer --partition_name  ${PARTITION_NAME} \
                                   --partition_size ${PARTITION_SIZE} \
                                   --image ${IMAGE} \
                                   --compact_footer \
                                   --output_vbmeta_image ${VBMETA_BLOB} \
                                   --do_not_append_vbmeta_image \
                                   --hash_algorithm $HASH_ALG

    DESCIPTOR=" --include_descriptors_from_image ${VBMETA_BLOB} "
    echo ${DESCIPTOR}
}

function images_add_footer()
{
    BPT_ORIG_FILE=$1
    ITEMS=$2
    IMAGE=
    PARTITION_NAME=
    HASH_ALG=
    FOOTER_TYPE=
    INFOS=
    VBMTEA_DESCIPTOR=
    for item in ${ITEMS};do
        INFOS=(`echo $item | tr ':' ' '`)
        IMAGE=${INFOS[0]}
        PARTITION_NAME=${INFOS[1]}
        HASH_ALG=${INFOS[2]}
        FOOTER_TYPE=${INFOS[3]}
        if [[ x"$FOOTER_TYPE" == x"hashtree" ]];then
            VBMTEA_DESCIPTOR+=" $(image_add_hashtree_footer "${IMAGE}" "${PARTITION_NAME}" "${BPT_ORIG_FILE}" "${HASH_ALG}")"
        elif [[ x"$FOOTER_TYPE" == x"hash" ]];then
            VBMTEA_DESCIPTOR+=" $(image_add_hash_footer "${IMAGE}" "${PARTITION_NAME}" "${BPT_ORIG_FILE}" "${HASH_ALG}")"
        else
            echo "unsupported hash algorithm"
            exit 1
        fi
    done
    echo ${VBMTEA_DESCIPTOR}
}

function image_add_hashtree_footer()
{
    IMAGE=$1
    PARTITION_NAME=$2
    BPT_FILE=$3
    HASH_ALG=$4
    FILE_TYPE=
    if [ $# != 4 ];then
        echo "image_add_hashtree_footer error param:$#" > /dev/stderr
        return -1
    fi

    IMAGE=$(realpath $IMAGE)
    FILE_TYPE=$(file ${IMAGE})
    PARTITION_SIZE=`bpttool query_partition --input ${BPT_FILE} --label ${PARTITION_NAME} --type size`
    IMAGE_MAX_SIZE=`avbtool add_hashtree_footer --calc_max_image_size --partition_size ${PARTITION_SIZE}`
    if [[ "${FILE_TYPE}" =~ "ext4 filesystem" ]];then
        IMAGE_MAX_SIZE=$((IMAGE_MAX_SIZE/1048576))
        resize2fs ${IMAGE} "${IMAGE_MAX_SIZE}M" > /dev/null
    fi

    #avbtool erase_footer --image ${IMAGE}
    avbtool add_hashtree_footer --partition_name  ${PARTITION_NAME} --partition_size ${PARTITION_SIZE} --image ${IMAGE}  --hash_algorithm $HASH_ALG
    if [[ $? -ne 0 ]];then
        echo "[ERROR] fail to add hashtree for ${IMAGE}" > /dev/stderr
        exit 1
    fi
    DESCIPTOR=" --include_descriptors_from_image ${IMAGE} "
    echo ${DESCIPTOR}
}

function make_vbmeta_image()
{
    DESCIPTOR=$1
    VBMETA_SIGNED_ARGS=$2
    VBMETA_FILE=$3

    if [ $# != 3 ];then
        echo "make_vbmeta_image error param:$#" > /dev/stderr
        return -1
    fi

    avbtool make_vbmeta_image --output ${VBMETA_FILE} ${VBMETA_SIGNED_ARGS} ${DESCIPTOR}
    return $?
}
