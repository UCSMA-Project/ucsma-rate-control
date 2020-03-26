# UCSMA Rate Control Protocol

This repo contains the rate control protocol for ucsma. It MUST be used in conjunction with the UCSMA unlocking protocol (kernel modules).

## Experiment Results
Below are the experiment results achieved through testing the UCSMA rate control protocol (along with the UCSMA unlocking protocol) on the three node topology:

## How to Compile
TODO: Write a script to automate this process

Below are instructions to compile each of the components, which must be done separately.

### Packetspammer
To compile packetspammer, first copy all contents of the `packetspammer` directory within the `ucsma-rate-control` repo to `ucsma/packetspammer/`.

Next, run the following commands:
```
PROJECT_ROOT="./ucsma"
BUILD_DIR="$PROJECT_ROOT/build_dir"

# compile packetspammer                                                                                                                                                             
LPCAP="$PROJECT_ROOT/OpenWRT-14.07-JS9331/build_dir/target-mips_34kc_uClibc-0.9.33.2/libpcap-1.5.3/"
LPTHREAD="$PROJECT_ROOT/OpenWRT-14.07-JS9331/build_dir/toolchain-mips_34kc_gcc-4.8-linaro_uClibc-0.9.33.2/uClibc-0.9.33.2/lib/libpthread.so"
LIBS="-L$LPCAP -L$LPTHREAD"

ucsma/build_dir/toolchain/OpenWrt-Toolchain-ar71xx-for-mips_34kc-gcc-4.8-linaro_uClibc-0.9.33.2/toolchain-mips_34kc_gcc-4.8-linaro_uClibc-0.9.33.2/bin/mips-openwrt-linux-gcc -Wall radiotap.c packetspammer.c -o packetspammer $LIBS -lpcap -ldl -lpthread -std=gnu99
```
And the result `packetspammer` can then be found in your current working directory. Note that you may need to alter some of the paths in the above code snippet to get it to work.

### ath9k.ko
To compile the `ath9k.ko` kernel module (network driver) that supports the rate control protocol, copy all `.c` and `.h` files from the `ath9k` direction in the `ucsma-rate-control` repo to `ucsma/OpenWRT-14.07-JS9331/build_dir/target-mips_34kc_uClibc-0.9.33.2/linux-ar71xx_generic/compat-wireless-2014-05-22/drivers/net/wireless/ath/ath9k` (from the main repo), which should be present after running `./build.sh`.

Then, `cd` to `ucsma/OpenWRT-14.07-JS9331` and run `make package/compile`.

Finally, you should be able to find the compiled `ath9k.ko` kernel module in `ucsma/OpenWRT-14.07-JS9331/staging_dir/target-mips_34kc_uClibc-0.9.33.2/root-ar71xx/lib/modules/3.10.49/ath9k.ko`.

### unlock.ko
To compile the `unlock.ko` kernel module (to send unlock signals), copy all files from the `kernel-module` directory of the `ucsma-rate-control` repo into the `ucsma/build_dir/ath/ath9k/` directory (which should be present if you have successfully run `build.sh` at least once) and run `build.sh`.

The resulting `unlock.ko` should be present at `ucsma/result/modules/unlock.ko`.
