# AM263x example
This example shows a basic ethercat example for the TI AM263x Launchpad.

# Build instructions:
### Install prerequisites:
`sudo apt install cmake libncurses5`

### Download compiler from [here](https://www.ti.com/tool/download/ARM-CGT-CLANG/4.0.0.LTS). Then run
```
./ti_cgt_armllvm_4.0.0.LTS_linux-x64_installer.bin --prefix ~/ti/
```

### Download industrial communications SDK from [here](https://www.ti.com/tool/download/INDUSTRIAL-COMMUNICATIONS-SDK-AM263X/09.01.00.03). Then run
```
ind_comms_sdk_am263x_09_02_00_13-linux-x64-installer.run --prefix ~/ti/
```

### Build example in VSCode
1. Open SOES_am263x.code-workspace in VSCode
2. Run build task using ctrl + shift + B
    * This might require modfying some tool paths in CMakeLists.txt

### Build example outside of VSCode
1. `cmake -S . -B build`
2. `cmake --build build`

## Debugging
### Install XDS110 drivers:
I just install TI CCS. maybe there's a better way.
1. Download latest version of ccs
2. Install
3. `cd ~/ti/ccs****/ccs/install_scripts`
4. `sudo ./install_drivers.sh'
### Build openocd from source
Follow instructions in [openocd readme](https://sourceforge.net/p/openocd/code/ci/master/tree/).

As of right now the version in my package manager doesn't have the required ti_k3 target script. Since you're reading this in the future, maybe it's included for you and you can skip this step.

### Install arm-none-eabi-gcc
Required for gdb. You might need to change the path in the workspace config to the install location.

### Debug using VSCode
1. Set bootmode switches, top to bottom, to right, left, right, right on the launchpad
2. Open SOES_am263x.code-workspace in VSCode
3. Install `marus25.cortex-debug` VSCode extension
4. Run and debug the "Debug with OpenOCD" target

### Run example master
```
python3 -m venv .env
source .env/bin/activate
pip install pysoem
# sudo is required to open raw sockets. There are ways to specifically allow 
# just raw socket permissions, but they seem somewhat system dependant
sudo $(which python) test.py
```