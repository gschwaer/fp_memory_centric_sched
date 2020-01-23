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
Example for one test cell:
1. `start_jailhouse.sh`
1. `start_uart_c.sh`
1. `inmate/gschwaer-testing/add_test_cell0.sh`
1. `inmate/gschwaer-testing/start_erika_inmate0.sh erika_inmate_dprem.bin`
