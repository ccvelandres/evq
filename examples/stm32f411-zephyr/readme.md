# Example -- Blackpill STM32F411

This example uses the Blackpill STM32F11 development board.

![](board.jpg)
Photo from https://www.dfrobot.com

## Pinout

![](board_pinout.png)
Photo from https://www.adafruit.com

# Requirements

## Zephyr

Follow the [Getting Started](https://docs.zephyrproject.org/latest/develop/getting_started/index.html) guide here to install some required dependencies.

```
# Setup venv
sudo apt install python3-virtualenv
python3 -m venv $HOME/.zephyrenv
source $HOME/.zephyrenv/bin/activate
pip3 install west

# Making a workspace -- skip if you already got this set-up
west init $HOME/zephyrproject
cd $HOME/zephyrproject
west update
west zephyr-export

export GNUARMEMB_TOOLCHAIN_PATH=/usr
export ZEPHYR_TOOLCHAIN_VARIANT=gnuarmemb
export ZEPHYR_BASE=$HOME/zephyrproject/zephyr
cmake -Bbuild
cmake --build build

```

# Building

## MSYS2

Under UCRT64, install the required packages: `git make mingw-w64-ucrt-x86_64-arm-none-eabi-toolchain mingw-w64-ucrt-x86_64-cmake`.

```
cmake --preset=debug
cmake --build build --parallel
```

# Flashing

## DFU

For Blackpill STM32F11 using the stm32 bootloader, install `dfu-util` and boot the board to dfu mode by holding `BOOT0` and pressing `NRST`.

## Black Magic Probe
