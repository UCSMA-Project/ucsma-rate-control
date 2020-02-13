PROJECT_ROOT=/home/ubuntu/399/new_ucsma/ucsma
BUILD_DIR=/home/ubuntu/399/new_ucsma/ucsma/build_dir
make -C "$PROJECT_ROOT/OpenWRT-14.07-JS9331/build_dir/target-mips_34kc_uClibc-0.9.33.2/linux-ar71xx_generic/linux-3.10.49/" \
     ARCH=mips \
     CROSS_COMPILE="$BUILD_DIR/toolchain/OpenWrt-Toolchain-ar71xx-for-mips_34kc-gcc-4.8-linaro_uClibc-0.9.33.2/toolchain-mips_34kc_gcc-4.8-linaro_uClibc-0.9.33.2/bin//mips-openwrt-linux-" \
     M="." \
     modules