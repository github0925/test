#!/bin/bash

# Create a FAT12/16/32 disk image from the contents of a directory.
#
# This tool requires the following to be available on the host system:
#
#
# - dosfstools(mkfs.vfat)
# - mtools(mmd, mcopy)
#


set -e

VERSION=1.0

SECTOR_SIZE=512
FAT_SIZE=16
LOGICAL_SECTOR_SIZE=512
NEW_DISKLABEL=o
NEW_PARTITION=n
PRIMARY=p
FIRST=1
OFFSET_8MB=16384
SET_PARTITION_TYPE=t
WIN95_FAT32=b
WRITE=w
VALID_SECSIZES=( 512 1024 2048 4096 8912 16384 32768 )

usage() {
  echo "Usage: $(basename $0) [-h -f -S SECSIZE] OUTPUT SIZE SOURCE"
  echo
  echo "Arguments:"
  echo "  OUTPUT      name of the image file"
  echo "  SIZE        size of the FAT12/16/32 partition in MiB (1024 based)"
  echo "  SOURCE      source directory"
  echo
  echo "Options:"
  echo "  -S SECSIZE  logical sector size (default: 512)"
  echo "  -F fat-size fat size (suport 12/16/32, default: 16)"
  echo "  -f          overwrite existing image file (if any)"
  echo "  -h          show this message and exit"
  echo
  echo "Valid values for SECSIZE are: 512, 1024, 2048, 4096, 8912, 16384, and "
  echo "32768. Using values other than those will result in an error."
  echo
  echo "dir2fat32 v$VERSION"
  echo "Copyright 2020 Nanjing Semidrive Technology Ltd."
  echo "Some rights reserved."
  echo
}

test_secsize() {
  secsize=$1
  for s in ${VALID_SECSIZES[@]}; do
    if [ "$s" == $secsize ]; then
      return 1
    fi
  done
  return 0
}

relpath() {
  full=$1
  if [ "$full" == "$SOURCE" ]; then
    echo ""
  else
    base=${SOURCE%%/}/
    echo "${full##$base}"
  fi
}

disksize() {
  echo $(expr $SIZE)
}

filesize() {
  path=$1
  echo $(expr $(stat -c%s "$path") / 1024 / 1024) MiB
}

mkpartition() {
  fallocate -l ${SIZE}M "$PARTITION"
  mkfs.fat -F$FAT_SIZE -S"$LOGICAL_SECTOR_SIZE" -s 16 "$PARTITION" >/dev/null
}

copyfiles() {
  find "$SOURCE" -type d | while read dir; do
    target=$(relpath "$dir")
    [ -z "$target" ] && continue
    echo "  Creating $target"
    mmd -i "$PARTITION" "::$target"
  done
  find $SOURCE -type f | while read file; do
    target=$(relpath "$file")
    echo "  Copying $target"
    mcopy -i "$PARTITION" "$file" "::$target"
  done
}

# Parse options
while getopts "hfF:S:" opt; do
	echo "opt" $opt 
  case "$opt" in
    h)
      usage
      exit 0
      ;;
    f)
      FORCE=1
      ;;
    F)
     FAT_SIZE=$OPTARG
      ;;
    S)
      LOGICAL_SECTOR_SIZE=$OPTARG
      ;;
    *)
      echo "Unrecognized option $opt"
      exit
  esac
done

# Parse remaining positional arguments
OUTPUT=${@:$OPTIND:1}
SIZE=${@:$OPTIND+1:1}
SOURCE=${@:$OPTIND+2:1}

if test_secsize "$LOGICAL_SECTOR_SIZE"; then
  echo "ERROR: Invalid logical sector size."
  usage
  exit 0
fi

if [ -z "$OUTPUT" ] || [ -z "$SIZE" ] || [ -z "$SOURCE" ]; then
  echo "ERROR: Missing required arguments, please see usage instructions"
  usage
  exit 0
fi
echo "force" $FORCE 
[ $FORCE ] && (rm -f $OUTPUT 2>/dev/null || true)

if [ -e "$OUTPUT" ]; then
  echo "ERROR: $OUTPUT already exists. Aborting."
  exit 1
fi

DISK_SIZE=$(disksize $SIZE)
#PARTITION=${OUTPUT}.partition
PARTITION=${OUTPUT}

echo "=============================================="
echo "Output file:      $OUTPUT"
echo "Image size:       $DISK_SIZE MiB"
echo "Sector size:      $LOGICAL_SECTOR_SIZE B"
echo "Source dir:       $SOURCE"
echo "=============================================="

echo "===> Creating FAT$FAT_SIZE partition image"
mkpartition
echo "===> Copying files"
copyfiles
echo "===> DONE"
