require conf/distro/poky.conf

DISTRO_VERSION = "2.5"

DISTRO = "sdrv-base"
DISTRO_NAME = "Semidrive Distro Base"
SDK_VENDOR = "-sdrvsdk"

MAINTAINER = "Semidrive Software <sw@semidrive.com>"
TARGET_VENDOR = "-sdrv"
DISTROOVERRIDES = "sdrv"

# This function changes the default tune for machines which are based on armv7a
# to use common tune value
def arm_tune_handler(d):
    features = d.getVar('TUNE_FEATURES', True).split()
    if 'armv7a' in features or 'armv7ve' in features:
        tune = 'armv7athf'
        if 'bigendian' in features:
            tune += 'b'
        if 'vfpv3' in features:
            tune += '-vfpv3'
        if 'vfpv3d16' in features:
            tune += '-vfpv3d16'
        if 'neon' in features:
            tune += '-neon'
        if 'vfpv4' in features:
            tune += '-vfpv4'
    else:
        tune = d.getVar('DEFAULTTUNE', True)
    return tune

DEFAULTTUNE_smdv := "${@arm_tune_handler(d)}"

DISTRO_ARM_INSTRUCTION ?= "thumb"
DISTRO_ARM_INSTRUCTION_armv5 ?= "arm"
ARM_INSTRUCTION_SET_smdv ??= "${DISTRO_ARM_INSTRUCTION}"

# Log information on images and packages
INHERIT += "buildhistory"
BUILDHISTORY_COMMIT = "1"

# Use bluez5 as default.
#DISTRO_FEATURES_append = " bluez5"

# The following set the providers to components supported by Semidrive
# Use Kernel, U-Boot and Gstreamer 1.0 providers
PREFERRED_PROVIDER_virtual/bootloader_x9 = "lk-boot-semidrive"
PREFERRED_PROVIDER_virtual/bootloader_g9 = "lk-boot-semidrive"

#PREFERRED_PROVIDER_virtual/kernel_x9 = "linux-semidrive"
#PREFERRED_PROVIDER_virtual/kernel_g9 = "linux-semidrive"

