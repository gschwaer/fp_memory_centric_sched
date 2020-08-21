Fixed Priority Memory Centric Scheduler: Erika on Jailhouse (Jetson TX2)
========================================================================

Copyright Notice
----------------

* [Jailhouse](https://github.com/siemens/jailhouse): All files in the folder `jailhouse-master_dprem` are licensed according to the specified license (see files `README.md`, `LICENSING.md`, `COPYING`, and related). Credits for the cache coloring implementation go to the [High-Performance Real-Time Laboratory (HiPeRT Lab) at the University of Modena](https://git.hipert.unimore.it/rtes/jailhouse) and the [HERCULES Innovation Action of the EU project Horizon 2020](http://hercules2020.eu/).
* [Erika](http://www.erika-enterprise.com/): All files in the folder `ee_files` are licensed according to the specified license (see files `README.md`, `LICENSE.TXT`, `THIRDPARTY.TXT`, and related, and [this website](http://www.erika-enterprise.com/index.php/erika3/licensing.html))
* [TACLe-Bench](http://www.tacle.eu/index.php/activities/taclebench): All files in the folder `benchmarking/sha` are licensed according to the license specified in the file. For more details see the [TACLe-Bench repository](https://github.com/tacle/tacle-bench/tree/master/bench/kernel/sha).
* Rest: For all files of this repository, without the subdirectories `ee_files` and `jailhouse-master_dprem`, which do not specify a different license in the file itself, the license will default to BSD Zero Clause License (see `DefaultLicense.txt`)
* For any file, containing license information on its own, this specified license takes precedence.

If there are any issues where a file in this repository violates the license terms of a respective property right owner, please write an issue and the file will be labeled correctly or removed.


Prerequisites on Host
---------------------

There is a docker configuration in `docker/` that can be used. To use it you folder structure should look like this:
```
somewhere
├── docker
└── code
    └── workspace
        └── Erika_DPREM
```
Download the file [`eclipse-rtdruid3-photon-linux-gtk-x86_64_20190524_gh65.tar.gz` (Eclipse Photon, Linux 64bit)](https://www.erika-enterprise.com/index.php/download/erika-v3-download.html), put it in the docker folder and use the `build.sh` to create a docker container that you can run with the `launch.sh`.


Prerequisites on TX2
--------------------

Jailhouse needs to be installed just as described in <https://github.com/evidence/linux-jailhouse-jetson>.
Compiling, installing and starting jailhouse are also described there. Additionally there are some helpful scripts for Jailhouse developement: `make.sh` and `recompile_restart.sh`.

In the folder `benchmarking/` are some scripts as well as Erika binaries (sub directories).


UART
----

One UART is used by Jailhouse inbuild printk function __and__ the Erika calls to printk.

To use the second UART (UART-C) ref. to `benchmarking/enable_uart_c.sh`. To monitor UART-C use minicom -D /dev/ttyUSB1 (or whatever your USB to serial adapter is detected as). Use minicom on you host machine to not slow down the TX2.


Erika
-----

* Every inmate gets its cache partition from its A57 core ID (in the range of 0-3 on TX2).
* To compile Erika applications for Jailhouse in Eclipse you need some files from the Jailhouse compilation. So after compiling Jailhouse copy the jailhouse folder back from the TX2 to the Host in place of the existing folder (copy_jailhouse_to_tx2.sh / copy_jailhouse_from_tx2.sh). The Erika Makefiles will then find the necessary libraries in the jailhouse folder.
* Setup of dependencies see below


Jailhouse
---------

* There are two versions of memory arbitration implemented: TDMA and fixed priority. You can switch from one to the other in the file `include/jailhouse/config.h` (+ recompile & restart).
* There is a define in the `fixed_priority.c` code which suspends linux cores in case the memory is in use (untested!). It should be easily adaptable for tdma as well.
* Jailhouse compilation might fail while it is running.


Test Runs
---------

Example for running benchmarks on all three cores:
1. `./prepare_linux.sh`
1. `sudo ./jetson_clocks.sh`
1. `./enable_uart_c.sh`
1. `./start_jailhouse.sh`
1. `./add_test_cells.sh`
1. `./run_all_erika_inmate.sh`
1. `./halt_linux` (will halt linux for 6 minutes)
1. [on host] open UART-C using minicom, leave without resetting (Ctrl+A,Q) then use `timeout 6m cat /dev/ttyUSB1 > capture.csv`
1. [on host] alternatively use `minicom -D /dev/ttyUSB1 -C capture.csv`

Configs used:

* Root cell: configs/arm64/jetson-tx2-colored.cell
* Test cell0: configs/arm64/jetson-tx2-gschwaer-testing-p0.cell
* Test cell1: configs/arm64/jetson-tx2-gschwaer-testing-p1.cell
* Test cell2: configs/arm64/jetson-tx2-gschwaer-testing-p2.cell


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

* Go to Entries > GNU C > CDT User Settings Entries:
* Add the following as Include Directory, File System Path:
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


Bugs and Quirks
---------------

It happens every now and then, that ...
* the USB serial to UART dies -> reboot
* the Erika inmate just stops executing -> reboot
* same for Linux -> hard reset
