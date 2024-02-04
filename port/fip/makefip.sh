#!/usr/bin/env bash

./fiptool.py -v genfip \
        'fip.bin' \
        --MONITOR_RUNADDR="0x0000000080000000" \
        --BLCP_2ND_RUNADDR="0x0000000083f40000" \
        --CHIP_CONF='chip_conf.bin' \
        --NOR_INFO='FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF' \
        --NAND_INFO='00000000'\
        --BL2='bl2_debug.bin' \
        --BLCP_IMG_RUNADDR=0x05200200 \
        --BLCP_PARAM_LOADADDR=0 \
        --BLCP=empty.bin \
        --DDR_PARAM='ddr_param.bin' \
        --BLCP_2ND='empty.bin' \
        --MONITOR='../../kernel/kernel.bin' \
        --LOADER_2ND='bl33.bin' \
        --compress='lzma' ;
