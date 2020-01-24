DPREM Erika on Jailhouse for Jetson TX2
=======================================

Prerequisites on Host
---------------------
There is a docker configuration in `docker.zip` that can be used. Download the file
[`eclipse-rtdruid3-photon-linux-gtk-x86_64_20190524_gh65.tar.gz` (Eclipse Photon, Linux 64bit)](https://www.erika-enterprise.com/index.php/download/erika-v3-download.html) and use the `build.sh` to create a docker container that you can run with the `launch.sh`.

Prerequisites on TX2
--------------------
Jailhouse needs to be installed just as described in <https://github.com/evidence/linux-jailhouse-jetson>.
Compiling, installing and starting jailhouse are also described there. Additionally there are some scripts in `jailhouse-master_dprem` like `make.sh`, `recompile_restart.sh` and `start_jailhouse.sh`.

In the folder `inmates/gschwaer-testing` are some scripts as well as Erika binaries.

Config
* Root cell: configs/arm64/jetson-tx2-colored.cell
* Test cell0: configs/arm64/jetson-tx2-gschwaer-testing-p0.cell
* Test cell1: configs/arm64/jetson-tx2-gschwaer-testing-p1.cell
* Test cell2: configs/arm64/jetson-tx2-gschwaer-testing-p2.cell

UART
----
To use the second UART (UART-C) ref. to `start_uart_c.sh`. To monitor UARTS use minicom -D /dev/ttyUSB0 (or whatever your USB to serial adapter is detected as).

Erika
-----
* Every inmate needs to be compiled, saved and started separately because they have different cache partitions (see `config.h`).
* To compile Erika applications for Jailhouse in Eclipse you need some files from the Jailhouse compilation. So after compiling Jailhouse copy the jailhouse folder back from the TX2 to the Host and replace the existing folder. The Erika Makefiles will then find the necessary libraries in the jailhouse folder.
* Setup of dependencies see below

Jailhouse
---------
* There are two versions of memory arbitration implemented: TDMA and fixed priority. You can switch from one to the other in the file `include/jailhouse/config.h`.
* There is a define in the `fixed_priority.c` code which suspends linux cores in case the memory is in use. It should be easily adaptable for tdma as well.

Bugs and Quirks
---------------
It happens every now and then, that ...
* the USB serial to UART dies -> reboot
* the Erika inmate just stops executing -> reboot
* same for Linux -> hard reset

Test Run
--------
Example for sorting benchmark on one test cell:
1. `sudo ./jetson_clocks.sh`
1. `./enable_uart_c.sh`
1. `./start_jailhouse.sh`
1. `./add_test_cell.sh 0`
1. `./run_erika_inmate.sh 0 sorting/erika_inmate.bin`
1. [on host] `minicom -D /dev/ttyUSB0 -C sorting_raw_data.txt`


Erika Dependencies Setup
------------------------
For the following you can also specify workspace paths if the files are in your workspace.
Project Properties > Oil > Erika Files Location:
* Manual `<path in your FS>/Erika_DPREM/ee_files`
Project Properties > Oil > Generator properties:
* Enable project specific settings
* AArch64 - Compiler prefix: `aarch64-linux-gnu-`
* AArch64 - Jailhouse dir: `<path in your FS>/jailhouse-master`
* AArch64 - Jailhouse version: `0.9.1`
Project Properties > C/C++ General > Preprocessor Include Paths, Macros etc.:
Go to Entries > GNU C > CDT User Settings Entries:
Add the following as Include Directory, File System Path:
* `/usr/lib/gcc-cross/aarch64-linux-gnu/7/include`
* `<path in your FS>/jailhouse-master/hypervisor`
* `<path in your FS>/jailhouse-master/hypervisor/include`
* `<path in your FS>/jailhouse-master/hypervisor/arch/arm64/include`
* `<path in your FS>/jailhouse-master/hypervisor/arch/arm-common/include`
* `<path in your FS>/jailhouse-master/include`
* `<path in your FS>/jailhouse-master/include/arch/arm64`
* `<path in your FS>/jailhouse-master/include/inmates/lib`
* `<path in your FS>/jailhouse-master/include/inmates/lib/arm64/include`
* `<path in your FS>/jailhouse-master/include/inmates/lib/arm-common/include`
