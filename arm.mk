#cross compile для arm
#root
OPENWRT = /home/prog/openwrt/lede-all/2019-openwrt-all/openwrt-ipq4xxx
#где лежат бинарники компилятора(gcc, ldd)
#PATH := $(PATH):$(STAGING_DIR)/bin
#указываем архитекруту
export ARCH = arm
export STAGING_DIR = $(OPENWRT)/staging_dir/toolchain-arm_cortex-a7+neon-vfpv4_gcc-7.4.0_musl_eabi
export CROSS_COMPILE = $(STAGING_DIR)/bin/arm-openwrt-linux-

#дальше все как обычно для модуля
#путь к исходнику ядра
KERNEL_DIR = ${OPENWRT}/build_dir/target-arm_cortex-a7+neon-vfpv4_musl_eabi/linux-ipq40xx/linux-4.19.23
