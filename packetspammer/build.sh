#!/bin/bash

FLAG_MENUCONFIG=false
FLAG_KERNELCONFIG=false

PROJECT_ROOT="$( cd "$(dirname "$0")" ; pwd -P )"

# Get number of available system processors (includes hyper-threading processors)
PROCESSORS=`grep -c ^processor /proc/cpuinfo`
(( THREADS = $PROCESSORS + 2 ))

clean() {
    rm -rf "$PROJECT_ROOT/build_dir"
    rm -rf "$PROJECT_ROOT/result"
}

usage() {
    if [ $# = 2 ]; then
        local flag=$1;
        local msg=$2;
        printf "\t$flag\t\t$msg\n";
    elif [ $# = 3 ]; then
        local flag=$1;
        local arg=$2;
        local msg=$3;
        printf "\t$flag $arg\t$msg\n";
    fi
}

print_usage() {
    echo "Usage: $0 [-c] [-k] [-t THREADS]"
    usage '-c' 'Enable advanced OpenWRT config (i.e. menuconfig).'
    usage '-k' 'Enable OpenWRT kernel config (i.e. kernel_menuconfig).'
    usage '-t' 'THREADS' 'Number of threads to use when compiling OpenWRT. Default: # of processors + 2.'
    usage '-d' 'Clean the project (removes build_dir and result). Does not proceed with build.'
}

# Loop to get options / print usage
while getopts 'ckt:d' flag; do
  case "${flag}" in
    c) FLAG_MENUCONFIG=true ;;
    k) FLAG_KERNELCONFIG=true ;;
    t) THREADS="${OPTARG}" ;;
    d) clean
       exit 0 ;;
    *) print_usage
       exit 1 ;;
  esac
done

# prepare to compile packetspammer for the AR9331
cd "$PROJECT_ROOT"
cp -r "packetspammer" "build_dir"
CC="$BUILD_DIR/toolchain/OpenWrt-Toolchain-ar71xx-for-mips_34kc-gcc-4.8-linaro_uClibc-0.9.33.2/toolchain-mips_34kc_gcc-4.8-linaro_uClibc-0.9.33.2/bin/mips-openwrt-linux-gcc"

# compile packetspammer
LPCAP="$PROJECT_ROOT/OpenWRT-14.07-JS9331/build_dir/target-mips_34kc_uClibc-0.9.33.2/libpcap-1.5.3/"
LPTHREAD="$PROJECT_ROOT/OpenWRT-14.07-JS9331/build_dir/toolchain-mips_34kc_gcc-4.8-linaro_uClibc-0.9.33.2/uClibc-0.9.33.2/lib/libpthread.so"
LIBS="-L$LPCAP -L$LPTHREAD"
cd "$BUILD_DIR/packetspammer"
$CC -Wall radiotap.c packetspammer.c -o packetspammer $LIBS -lpcap -ldl -lpthread -std=gnu99

# collect build results
cd "$PROJECT_ROOT"
mkdir "result"

mkdir "result/firmware"
cp "$PROJECT_ROOT/OpenWRT-14.07-JS9331/bin/ar71xx/openwrt-ar71xx-generic-tl-wr720n-v3-squashfs-factory.bin" "result/firmware"
cp "$PROJECT_ROOT/OpenWRT-14.07-JS9331/bin/ar71xx/openwrt-ar71xx-generic-tl-wr720n-v3-squashfs-sysupgrade.bin" "result/firmware"

mkdir "result/modules"
cp "$BUILD_DIR/ath/ath9k/"*.ko "result/modules"

mkdir "result/utilities"
cp "$BUILD_DIR/packetspammer/packetspammer" "result/utilities"