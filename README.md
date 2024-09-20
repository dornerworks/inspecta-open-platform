# Inspecta Open Platform
The open platform implementation for INSPECTA. 

## Setup

### Dependencies

* Make
* dtc (Device Tree Compiler)
* Clang, LLD, and `llvm-ar`
* QEMU (for simulating the example)
* Microkit SDK

On Ubuntu/Debian:

```sh
sudo apt update && sudo apt install -y make clang lld llvm qemu-system-arm device-tree-compiler
```


#### Optional

* bear (For creating a compile_commands.json for Clang LSP)

```sh
sudo apt install -y bear
```

### Acquiring the SDK
Get the microkit sdk with SMC support enabled for the ZCU102. It is best to do this in a directory outside the repo.

```sh
wget https://github.com/dornerworks/microkit/releases/download/inspecta-v0.5/microkit-sdk-1.4.1.tar.gz
tar -xvzf microkit-sdk-1.4.1.tar.gz
# This variable needs to be set anytime a new terminal is opened
export MICROKIT_SDK=$(pwd)/microkit-sdk-1.4.1
```

## Building

```sh
make BOARD=zcu102 MICROKIT_SDK=/path/to/sdk
```

Suggestion: Setup a `.env` file with:
```
export BOARD=zcu102
export MICROKIT_SDK=/path/to/sdk
```

Then when you open a new terminal, you just need to run
```sh
source .env
```

Then to build, you just run
```sh
make
```

### Creating `compile_commands.json` for clang LSP
When building prepend the build command with `bear`. This must be done when the build is actually doing something, so if you have already built, then do a `make clean` first.

```sh
bear -- make BOARD=zcu102 MICROKIT_SDK=/path/to/sdk
```

Then open your editor in the root directory of the repo.

## Booting
Directions for booting with either tftp or mmc are provided.

### Getting the bootloader
Booting virtualized microkit systems on the ZCU102 is an odd edgecase. This build system creates a binary, `loader.img`, which requires the `go` command to run once it is loaded. The `go` command for U-Boot on the ZCU102 drops to EL1, so a modified U-Boot is required. You can get the required U-Boot from here: [https://github.com/dornerworks/meta-inspecta-sut/releases/download/v0.1.0-baseline/BOOT.BIN](https://github.com/dornerworks/meta-inspecta-sut/releases/download/v0.1.0-baseline/BOOT.BIN)

### Setup SD Card
This boot binary needs to be copied to a boot partition on a micro SD card that is setup following these [directions](https://xilinx-wiki.atlassian.net/wiki/spaces/A/pages/18842385/How+to+format+SD+card+for+SD+boot).

Follow these [directions](https://xilinx-wiki.atlassian.net/wiki/spaces/A/pages/18841858/Board+bring+up+using+pre-built+images#Boardbringupusingpre-builtimages-ZCU102) to configure the board to boot from the SD card.

### Booting with TFTP
Copy `build/loader.img` to the tftp root directory. Place the SD card into the ZCU102 development board. Boot the board and run the following command in U-Boot:
```
dhcp; tftpboot 0x40000000 loader.img; go 0x40000000
```

### Booting with MMC
Copy the loader image onto a boot partition of a micro SD card. Place the SD card into the ZCU102 development board. Boot the board and run the following command in U-Boot:
```
fatload mmc 0 0x40000000 loader.img; go 0x40000000
```
