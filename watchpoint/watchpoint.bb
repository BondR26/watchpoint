SUMMARY = "External Linux kernel module"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/GPL-2.0;md5=801f80980d171dd6425610833a22dbe6"

inherit module

SRC_URI = "file://watchpoint.c \
           file://Makefile"

PV = "1.0"

S = "${WORKDIR}"

# Provide the kernel module name
RPROVIDES_${PN} = "kernel-module-watchpoint"

# Ensure appropriate dependencies and paths are set up for cross-compiling
EXTRA_OEMAKE = "'KERNEL_SRC=${STAGING_KERNEL_DIR}' 'ARCH=${ARCH}' 'CROSS_COMPILE=${TARGET_PREFIX}'"

# Install the kernel module in the correct location
FILES_${PN} = "${base_libdir}/modules/${KERNEL_VERSION}/extra/watchpoint.ko"

