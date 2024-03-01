# fsbl Build Process

## Compile

- CC      plat/cv180x/bl2/bl2_main.c

  build/cv1800b_milkv_duo_sd/bl2/bl2_main.o
  build/cv1800b_milkv_duo_sd/bl2/bl2_main.d

- AS      lib/cpu/riscv/bl2_entrypoint.S

  build/cv1800b_milkv_duo_sd/bl2/bl2_entrypoint.o
  build/cv1800b_milkv_duo_sd/bl2/bl2_entrypoint.d

- PP      plat/cv180x/bl2/bl2.ld.S

  build/cv1800b_milkv_duo_sd/bl2/bl2.ld
  build/cv1800b_milkv_duo_sd/bl2/bl2.ld.d

- PP      make_helpers/get_macros.ld.S

  build/cv1800b_milkv_duo_sd/blmacros/blmacros.ld
  build/cv1800b_milkv_duo_sd/blmacros/blmacros.ld.d

- LD      build/cv1800b_milkv_duo_sd/blmacros/blmacros.elf

  build/cv1800b_milkv_duo_sd/blmacros/blmacros.ld
  build/cv1800b_milkv_duo_sd/blmacros/build_message.o
  plat/cv180x/bl2_objs/cv1800b_milkv_duo_sd/bl2/*.o

  ---

  build/cv1800b_milkv_duo_sd/blmacros/blmacros.elf
  build/cv1800b_milkv_duo_sd/blmacros/blmacros.map

- LD      build/cv1800b_milkv_duo_sd/bl2/bl2.elf

  build/cv1800b_milkv_duo_sd/bl2/bl2.ld
  build/cv1800b_milkv_duo_sd/bl2/build_message.o
  plat/cv180x/bl2_objs/cv1800b_milkv_duo_sd/bl2/*.o
  build/cv1800b_milkv_duo_sd/bl2/bl2_main.o
  build/cv1800b_milkv_duo_sd/bl2/bl2_entrypoint.o

  ---

  build/cv1800b_milkv_duo_sd/bl2/bl2.elf
  build/cv1800b_milkv_duo_sd/bl2/bl2.map

- BIN     build/cv1800b_milkv_duo_sd/blmacros.bin

  build/cv1800b_milkv_duo_sd/blmacros/blmacros.elf

  ---

  build/cv1800b_milkv_duo_sd/blmacros.bin

- SYM     build/cv1800b_milkv_duo_sd/blmacros/blmacros.sym

  build/cv1800b_milkv_duo_sd/blmacros/blmacros.elf

  ---

  build/cv1800b_milkv_duo_sd/blmacros/blmacros.sym

- OD      build/cv1800b_milkv_duo_sd/blmacros/blmacros.dis

  build/cv1800b_milkv_duo_sd/blmacros/blmacros.elf

  ---

  build/cv1800b_milkv_duo_sd/blmacros/blmacros.dis

---

- BIN build/cv1800b_milkv_duo_sd/bl2.bin

  build/cv1800b_milkv_duo_sd/bl2/bl2.elf

  ---

  build/cv1800b_milkv_duo_sd/bl2.bin

- SYM build/cv1800b_milkv_duo_sd/bl2/bl2.sym

  build/cv1800b_milkv_duo_sd/bl2/bl2.elf

  ---

  build/cv1800b_milkv_duo_sd/bl2/bl2.sym

- OD build/cv1800b_milkv_duo_sd/bl2/bl2.dis

  build/cv1800b_milkv_duo_sd/bl2/bl2.elf

  ---

  build/cv1800b_milkv_duo_sd/bl2/bl2.dis

## fiptool.py

```
INFO:root:PROG: fiptool.py
DEBUG:root:  BL2='/home/juhan/local/duo-buildroot-sdk/fsbl/build/cv1800b_milkv_duo_sd/bl2.bin'
DEBUG:root:  BL2_FILL=None
DEBUG:root:  BLCP='test/empty.bin'
DEBUG:root:  BLCP_2ND='/home/juhan/local/duo-buildroot-sdk/freertos/cvitek/install/bin/cvirtos.bin'
DEBUG:root:  BLCP_2ND_RUNADDR=2213806080 #0x83f40000
DEBUG:root:  BLCP_IMG_RUNADDR=85983744   #0x05200200
DEBUG:root:  BLCP_PARAM_LOADADDR=0
DEBUG:root:  BLOCK_SIZE=None
DEBUG:root:  CHIP_CONF='/home/juhan/local/duo-buildroot-sdk/fsbl/build/cv1800b_milkv_duo_sd/chip_conf.bin'
DEBUG:root:  DDR_PARAM='test/cv181x/ddr_param.bin'
DEBUG:root:  LOADER_2ND='/home/juhan/local/duo-buildroot-sdk/u-boot-2021.10/build/cv1800b_milkv_duo_sd/u-boot-raw.bin'
DEBUG:root:  MONITOR='../opensbi/build/platform/generic/firmware/fw_dynamic.bin'
DEBUG:root:  MONITOR_RUNADDR=2147483648  #0x80000000
DEBUG:root:  NAND_INFO=b'\x00\x00\x00\x00'
DEBUG:root:  NOR_INFO=b'\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff'
DEBUG:root:  OLD_FIP=None
DEBUG:root:  compress='lzma'
DEBUG:root:  func=<function generate_fip at 0x7f196e4aa5c0>
DEBUG:root:  output='/home/juhan/local/duo-buildroot-sdk/fsbl/build/cv1800b_milkv_duo_sd/fip.bin'
DEBUG:root:  subcmd='genfip'
DEBUG:root:  verbose=10
```
