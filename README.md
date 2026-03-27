# linux-rpi-i2c
# My I2C Kernel Module
# i2c device: PCF8574T I/O

Custom Linux kernel module for Raspberry Pi (I2C).

## Features
- Sysfs interface: /sys/kernel/my_i2c_module/data
- Read/write over I2C (SMBus)
- Built-in kernel support

## Usage
# i2c address 0x20

echo 0x01 | sudo tee /sys/kernel/my_i2c_module/data
cat /sys/kernel/my_i2c_module/data

## Build

1. Clone Raspberry Pi kernel:
sudo apt install git
sudo apt install bc bison flex libssl-dev make
git clone --depth=1 https://github.com/raspberrypi/linux linux-rpi-i2c
cd linux-rpi-i2c

2. Copy module:
drivers/i2c/my_i2c_module.c

3. Add to the end Kconfig and Makefile
Examples:
Kconfig
config MY_I2C_MODULE
        tristate "My I2C sysfs I/O module"
        default y
        help
          A simple I2C driver exposing one sysfs file:
          /sys/kernel/my_i2c_module/data
          Supports read/write for testing.

Makefile
obj-$(CONFIG_MY_I2C_MODULE)     += my_i2c_module.o


4. Build kernel
export ARCH=arm64
KERNEL=kernel_2712
make bcm2712_defconfig
make menuconfig     # Device Drivers → I2C → My I2C Module [*]

# Run the following commands to build a 64-bit kernel:
time make -j4 Image.gz modules dtbs

# Install the kernel modules onto the boot media:
sudo make -j4 modules_install

# To install the 64-bit kernel:
sudo cp /boot/firmware/$KERNEL.img /boot/firmware/$KERNEL-backup.img
sudo cp arch/arm64/boot/Image.gz /boot/firmware/$KERNEL.img
sudo cp arch/arm64/boot/dts/broadcom/*.dtb /boot/firmware/
sudo cp arch/arm64/boot/dts/overlays/*.dtb* /boot/firmware/overlays/
sudo cp arch/arm64/boot/dts/overlays/README /boot/firmware/overlays/

sudo reboot
