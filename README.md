# UCSMA Rate Control Protocol

This repo contains the rate control protocol for ucsma. It MUST be used in conjunction with the UCSMA unlocking protocol (kernel modules).

## Experiment Results
Below are the experiment results achieved through testing the UCSMA rate control protocol (along with the UCSMA unlocking protocol) on the three node topology.

For each unlocking period T (T = 20,000/60,000/10,000/150,000/200,000 microseconds), we plotted 3 graphs:

i. The dot plot shows the packet transmission timestamps for each node ie. a dot indicates a packet was sent from the node, and each blue vertical line corresponds to an unlock signal; the x-axis corresponds to time in microseconds.

ii. The bar chart shows the average throughput of each node in kbps (calculated over a total of 5000 packets sent from all nodes; each packet is roughly 1kb).

iii. The line graph shows the buffer size at each node upon receiving the unlock signal ie. updates every T microseconds; the x-axis corresponds to time in microseconds, and the y-axis corresponds to the number of packets in the buffer.

### A. T = 20,000 microseconds
#### Graph A.i
![Graph A.i](./Results/dot%20T=20,000.png)
#### Graph A.ii
![Graph A.ii](./Results/avgthruput%20T=20,000.png)
#### Graph A.iii
![Graph A.iii](./Results/buffsize%20T=20,000.png)

### B. T = 60,000 microseconds
#### Graph B.i
![Graph B.i](./Results/dot%20T=60,000.png)
#### Graph B.ii
![Graph B.ii](./Results/avgthruput%20T=60,000.png)
#### Graph B.iii
![Graph B.iii](./Results/buffsize%20T=60,000.png)

### C. T = 100,000 microseconds
#### Graph C.i
![Graph C.i](./Results/dot%20T=100,000.png)
#### Graph C.ii
![Graph C.ii](./Results/avgthruput%20T=100,000.png)
#### Graph C.iii
![Graph C.iii](./Results/buffsize%20T=100,000.png)

### D. T = 150,000 microseconds
#### Graph D.i
![Graph D.i](./Results/dot%20T=150,000.png)
#### Graph D.ii
![Graph D.ii](./Results/avgthruput%20T=150,000.png)
#### Graph D.iii
![Graph D.iii](./Results/buffsize%20T=150,000.png)

### E. T = 200,000 microseconds
#### Graph E.i
![Graph E.i](./Results/dot%20T=200,000.png)
#### Graph E.ii
![Graph E.ii](./Results/avgthruput%20T=200,000.png)
#### Graph E.iii
![Graph E.iii](./Results/buffsize%20T=200,000.png)

From the graphs above, we can notice the trade off clearly. 

In those throughput bar plots, we cannot obsever the throughput fairness very well(but it has already been improved a little already compared to the result in George's report) when T is small and the throughput fairness is improved by increase the value of T. 

In addition, the dot plots is the packets recieved timeline recorded by Raspberry Pi and we can notice when T is small, left and right node can send packets when it should be the mid node turn to send. However, by increasing the value of T, we can observe the result that mid node and left or/and right nodes are sending packets turn by turn.

Besides that, the buffsize graphs above can explain what happened in the buffer when left or/and right node recieve an unlock signal. When left or/and right nodes recieve an unlock signal, the mid node will send all packets in its buffer while left and right nodes cannot send packets. On the other hand, when left or/and right nodes send packets, the packets in the mid node will not be sended but are accumulated in the buffer until left or/and right nodes send all packets in the buffer. Therefore, if T is small, then the nodes will not have enough time in the buffer while all packets can be sended with a larger T.

T is the unlocking period. Theoretically speaking, a larger T will cause a larger delay. Nevertheless, a larger T can improve the throughput fairness which is what we want. 

## UCSMA Main Repo
The following sections detail how to setup the required items in the main repository, which can be found here: https://github.com/UCSMA-Project/ucsma

### Dependencies:
1) A GNU/Linux system
2) All [prerequisite packages](https://wiki.openwrt.org/doc/howto/buildroot.exigence#table_of_known_prerequisites_and_their_corresponding_packages) for the OpenWRT build system must be installed.

### Building Everything
Simply running build.sh will proceed with building all of the components necessary for the experiment.
Build products will be located in the result folder after the build process completes.

### AR9331 SoC Login/Setup
- If you have flashed the chip, make sure to first disable the firewall (otherwise `ssh` will not work): `/etc/init.d/firewall disable`
- To `ssh` into the chip, run `ssh root@<ip_addr>` with password `root` (scp works the same way)
- Connecting via serial port is also possible with serial speed `115200`

### How to compile and use built modules
As mentioned above, running `build.sh` will build all of the components necessary for the experiment. Once built, these components can be found in the `/result` directory

#### Flashing the OS
The `result` directory contains two `OpenWRT` (OS) binary files, where one of which is a `sysupgrade`. To flash the system, use the non-`sysupgrade` (`factory`) version, otherwise files will not be overwritten. On the chip, run `sysupgrade <bin name>`.

#### Copying modules
Use `scp` to copy `packetspammer` and `unlock.ko` to each of the AR9331 boards. It may be helpful to use a script together with `ssh-pass` to pass in the password directly through the script

### Setting up the experiment with three node topology
#### On the AR9331 Boards
1. Copy the relevant files to the AR9331 boards.
2. 

#### On the Raspberry Pi
1. Copy the `gpio_timeline` repository into the Raspberry Pi
2. Run `rpi-update` and update the entire operating system to the newest version (make sure this is done properly as it ensures the kernel headers match.
3. Run `sudo apt-get install raspberrypi-kernel-headers`.
4. `cd` into the repository and run make.
5. To install the `gpio_timeline` kernel module, run `sudo insmod gpio_timeline.ko human_readable_output=<1 or 0> max_log_count=<# logs>`.
    The Raspberry Pi will then log for the `max_log_count` number of events and flush logs into the kernel ring buffer. It will no longer log after that so a `rmmod` and a subsequent `insmod` is required
6. To view logs, run `sudo dmesg`.
7. To remove the kernel module, run `sudo rmmod gpio_timeline`

#### Physically
1. Place three nodes 2.5 meters apart each, in the same orientation, and then connect them to power.
2. Make sure antennas are firmly attached to each of the three boards.
3. Connect the three nodes to a router using the WAN ethernet port. (This is to allow us to `ssh` into the boards)
4. (TODO: Add pin assignment instructions)

### How to use packetspammer and horst on the AR9331
#### Packetspammer
Packetspammer is a command line executable (for the AR9331 chip) that broadcasts packets as fast as it can on the specified monitor. On the AR9331 SoC, run command `./packetspammer -d <delay> <monitor id>` to begin sending packets

#### Horst
Horst is a network monitoring tool/sniffer that allows you to see the channel usage and received packets. On the AR9331 SoC, run command `horst -i <monitor id>` to open the sniffer - we recommend using a separate terminal to do this and to avoid doing it via a serial connection

### Frequently Encountered Issues
#### build.sh make/world error
If you see an error similar to this (when building in verbose mode):
```
Unescaped left brace in regex is illegal here in regex; marked by <-- HERE in m/\${ <-- HERE ([^ \t=:+{}]+)}/ at ./bin/automake.tmp line 3938.
Makefile:50: recipe for target '/openwrt-master/build_dir/host/automake-1.15/.configured' failed
make[3]: *** [openwrt-master/build_dir/host/automake-1.15/.configured] Error 255
make[3]: Leaving directory '/openwrt-master/tools/automake'
tools/Makefile:134: recipe for target 'tools/automake/compile' failed
make[2]: *** [tools/automake/compile] Error 2
make[2]: Leaving directory '/openwrt-master'tools/Makefile:133: recipe for target '/openwrt-master/staging_dir/target-x86_64_musl1.1.14/stamp/.tools_install_yynyynynynyyyyyyynyyynyyyyyyyyynyyyyynyyynynnyyynnnyy' failed
make[1]: *** [/openwrt-master/staging_dir/target-x86_64_musl1.1.14/stamp/.tools_install_yynyynynynyyyyyyynyyynyyyyyyyyynyyyyynyyynynnyyynnnyy] Error 2
make[1]: Leaving directory '/openwrt-master'/openwrt-master/include/toplevel.mk:192: recipe for target 'world' failed
make: *** [world] Error 2
```
Simply go into the `./bin/automake*` files and edit the unescaped curly braces on the lines specified. This is an error caused by a deprecated perl version on Ubuntu 18.04.

#### Packetspammer recorded throughput extremely high
This is an issue with packetspammer incrementing the throughput whenever the `ath9k`'s driver entry point is called, regardless of the buffer being actually sent. For example, if the software buffer is full, the function would immediately return with an error, but packetspammer would not detect this. For precise throughput, use the Raspberry Pi along with the `gpio_timeline.ko` and `unlock.ko` kernel modules in order to monitor throughput.

#### All nodes are able to see each other despite being a sufficient physical distance apart
Check the `TX_Power` and `noise_floor` settings in the monitor.

#### Can't find the `ath9k` directory used for building the network driver
It's `OpenWRT-14.07-JS9331/build_dir/target-mips_34kc_uClibc-0.9.33.2/linux-ar71xx_generic/compat-wireless-2014-05-22/drivers/net/wireless/ath/ath9k/`

#### Can't find the compiled kernel modules
It's `OpenWRT-14.07-JS9331/staging_dir/target-mips_34kc_uClibc-0.9.33.2/root-ar71xx/lib/modules/3.10.49`

#### I just want compile the network driver
Within the `OpenWRT` directory, run `make package/compile`. Then the result can be found in the directory above.

## Topology Setup
### Install ath9k module
1. Copy the relevant files to the AR9331 boards.
2. Install ath9k module with command `insmod ath9k`. (default model) Uninstall ath9k module with command `rmmod ath9k`.
3. Set noise floor and txpower by running `init.sh` script sometimes the noise floor need to be set angin manully after running `init.sh` script. Left and Right nodes with noise floor = -56 and txpower = 100 while the mid node with noise floor = -95 and txpower = 2000.
4. Run packetspammer on three boards with command e.g. `./packetspammer -d0 mon0`. 
5. Run horst on three boards with command e.g. `horst -i mon0` (horst can be helpful when adjusting the topology.)
6. Observe the throughput of each device to make sure the topology holds.

### Install unlock module and rate control module
1. Copy the relevant files to the AR9331 boards.
2. Run packetspammer before install unlock module by command e.g. `./packetspammer -d0 mon0` to send a packet first.
3. Install unlock module with command `insmod unlock.ko`.
4. Install buffer_number module with command `insmod buffer_number.ko`.
5. Set parameters for unlock module. (T and Delta) The default values of these two parameters are (T = 20000 microsecond, Delta = 100 microsecond) with command `echo $1 > /sys/module/unlock/parameters/T` and `echo $1 > /sys/module/unlock/parameters/Delta`.
6. Run packetspammer with command e.g. `./packetspammer -d0 mon0` and observe the result with Raspberry Pi or by the printed throughput by packetspammer.

### Unlock Wire Connection
#### Raspberry Pi pin Assignment
![Raspberry Pi pin Assignment](./images/rasp_pi_assign.jpg)

#### Unlock Wire Connection
![Unlock Wire Connection](./images/unlock.jpg)

## How to Compile
Below are instructions to compile each of the components, which must be done separately. A script to automate this process will probably save a lot of time in the long run and could be something to look into doing in the future.

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

# Blueprint of Wireless Unlocking
Current implementation of unlocking uses wires to pass the unlocking signal. One of the next steps of this project is to implement the unlocking wirelessly.

By the time of writing this document, the equipment to implement wireless unlocking has been bought but the project was interrupted by the COVID-19 pandemic as the university was physically shut down.

Here we provide a blueprint of how the wireless unlocking should be implemented. The unlocking signal produced by the chips should be passed to a module which will rely the signal wirelessly.

The wireless module we bought is E10-433MD-SMA which is essentially a modulation of RF chip SI4463. Detailed documentation including how to programming this chip, provided by the manufacture can be found at http://www.ebyte.com/product-view-news.aspx?id=59. And the data sheet for SI4463 can be found at https://www.silabs.com/documents/public/data-sheets/Si4464-63-61-60.pdf

The wireless module must be driven by a micro-controller, the MCU(micro-controller unit) we chose is Arduino Uno Rev3, which is a popular and widely-used open-source MCU. The documentation on how to programming it can be found at arduino.cc.

The chip communicate with Arduino via Serial Peripheral Interfect(SPI), the datasheet of the wireless module has instructions on how to connect the wireless module to Ardunio.

The workflow of sending an unlocking signal should be: chip produces a unlocking signal -> Arduino detects the unlocking signal -> Arduino drives the wireless module to rely the signal wirelessly.

The workflow of receiving an unlocking signal should be: wireless module detects a signal -> Arduino detects the unlocking signal and generate a hardware interrupt for the chip -> OS of the chip handle the interrupt.

Notice that SI4463 should be put on direct mode, which means that the wireless module sending data synchronously and does not conduct any packet handle. This can minimize the latency as unlocking is a latency-sensitive job.