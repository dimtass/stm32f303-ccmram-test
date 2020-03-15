STM32F303 CCMRAM test
----

[![dimtass](https://circleci.com/gh/dimtass/stm32f303-ccmram-test.svg?style=svg)](https://circleci.com/gh/dimtass/stm32f303-ccmram-test)

> Note: This code is from this blog post [here](https://www.stupid-projects.com/using-ccm-on-stm32f303cc/).

This project is a template project for evaluating and testing the
Core-Coupled Memory (CCM) of the STM32F303CC. This memory is 8KB
in size and it's the fastest memory in the chip (faster than SRAM).
You can map the functions you need in that area by using a compiler
attribute like this.

```cpp
__attribute__((section(".ccmram"))) void function();
```

> Note: I've used [this](https://www.st.com/content/ccc/resource/technical/document/application_note/bb/09/ca/83/14/e9/44/c5/DM00083249.pdf/files/DM00083249.pdf/jcr:content/translations/en.DM00083249.pdf)
application note for enabling the CCM RAM feature.

In this project I'm using the [LZ4](https://github.com/lz4/lz4)
which is a fast compression library. The code for the lib is located in
`source/libs/lz4/`. I've mapped the `LZ4_compress_generic()` function
in the CCMRAM which is located in `source/libs/lz4/src/lz4.c`.

Furthermore, in order to support the CCM RAM a modification is needed
in the default startup `source/libs/cmsis/device/startup_stm32f30x.s`
file and the linker script `source/config/LinkerScripts/STM32F303xC/STM32F303VC_FLASH.ld`.
In this project the modifications are already done, therefore you can
use those files to other projects in order to enable the CCM RAM.

Finally, you also need to provide the path of the toolchain to
use in the `CMAKE_TOOLCHAIN` if it's different from the default one.
By default is pointing to `/opt/toolchains/gcc-arm-none-eabi-9-2019-q4-major`.

## Cloning the code
Because this repo has dependencies on other submodules, in order to
fetch the repo use the following command:

```sh
git clone --recursive -j8 https://dimtass@bitbucket.org/dimtass/stm32f103-cmake-template.git
```

## Build the firmware

There are many flags that you can use for building the firmware. This is the format
of the buld command:
```sh
USE_OVERCLOCKING=<ON/OFF> USE_BLOCK_SIZE=<N> USE_BLOCK_COUNT=<N> USE_CCM=<ON/OFF> USE_SRAM=<ON/OFF> USE_FLASH=<ON/OFF> ./build.sh
```

* `USE_OVERCLOCKING`, ON: enable overclocking at 128MHz, OFF: 72MHz
* `USE_BLOCK_SIZE`, number of bytes used as the block size. Default: 8, which means 8K
* `USE_BLOCK_COUNT`, number of blocks used for the compression. Default: 512.
* `USE_CCM`, ON: move compression function to CCMRAM
* `USE_SRAM`, ON: move compression function to SRAM
* `USE_FLASH`, ON: move compression function to FLASH

Notes:
* Only one of the `USE_CCM`, `USE_SRAM`, `USE_FLASH` can be `ON`.
* The processed size will be `USE_BLOCK_SIZE`*`USE_BLOCK_COUNT`
* The default processed size is 4MB
* The `USE_BLOCK_SIZE` can not be larger than 20KB

These are some examples:

```sh
USE_OVERCLOCKING=OFF USE_BLOCK_SIZE=8 USE_BLOCK_COUNT=512 \
USE_CCM=OFF USE_SRAM=OFF USE_FLASH=ON ./build.sh

USE_OVERCLOCKING=OFF USE_BLOCK_SIZE=16 USE_BLOCK_COUNT=512 \
USE_CCM=OFF USE_SRAM=OFF USE_FLASH=ON ./build.sh

USE_OVERCLOCKING=ON USE_BLOCK_SIZE=8 USE_BLOCK_COUNT=512 \
USE_CCM=OFF USE_SRAM=OFF USE_FLASH=ON ./build.sh

USE_OVERCLOCKING=ON USE_BLOCK_SIZE=16 USE_BLOCK_COUNT=512 \
USE_CCM=OFF USE_SRAM=OFF USE_FLASH=ON ./build.sh

USE_OVERCLOCKING=OFF USE_BLOCK_SIZE=8 USE_BLOCK_COUNT=512 \
USE_CCM=OFF USE_SRAM=ON USE_FLASH=OFF ./build.sh

USE_OVERCLOCKING=OFF USE_BLOCK_SIZE=16 USE_BLOCK_COUNT=512 \
USE_CCM=OFF USE_SRAM=ON USE_FLASH=OFF ./build.sh

USE_OVERCLOCKING=ON USE_BLOCK_SIZE=8 USE_BLOCK_COUNT=512 \
USE_CCM=OFF USE_SRAM=ON USE_FLASH=OFF ./build.sh

USE_OVERCLOCKING=ON USE_BLOCK_SIZE=16 USE_BLOCK_COUNT=512 \
USE_CCM=OFF USE_SRAM=ON USE_FLASH=OFF ./build.sh

USE_OVERCLOCKING=OFF USE_BLOCK_SIZE=8 USE_BLOCK_COUNT=512 \
USE_CCM=ON USE_SRAM=OFF USE_FLASH=OFF ./build.sh

USE_OVERCLOCKING=OFF USE_BLOCK_SIZE=16 USE_BLOCK_COUNT=512 \
USE_CCM=ON USE_SRAM=OFF USE_FLASH=OFF ./build.sh

USE_OVERCLOCKING=ON USE_BLOCK_SIZE=8 USE_BLOCK_COUNT=512 \
USE_CCM=ON USE_SRAM=OFF USE_FLASH=OFF ./build.sh

USE_OVERCLOCKING=ON USE_BLOCK_SIZE=16 USE_BLOCK_COUNT=512 \
USE_CCM=ON USE_SRAM=OFF USE_FLASH=OFF
```


To build the firmware without using CCM then:
```sh
CLEANBUILD=true USE_CCM=OFF ./build.sh
```

To build the firmware using CCM then:
```sh
CLEANBUILD=true USE_CCM=ON ./build.sh
```

## Flashing the firmware
To flash the firmware in Linux you need the [texane/stlink](https://github.com/texane/stlink) tool.
Then you can use the flash script like this:
```sh
./flash.sh
```

Otherwise you can build the firmware and then use any programmer you like. The
`elf`, `hex` and `bin` firmwares are located in the build folder
```
./build-stm32/src/stm32f303xc-ccm-test.bin
./build-stm32/src/stm32f303xc-ccm-test.hex
./build-stm32/src/stm32f303xc-ccm-test.elf
```

## FW details
* `CMSIS version`: 4.2.0
* `StdPeriph Library version`: 1.2.3