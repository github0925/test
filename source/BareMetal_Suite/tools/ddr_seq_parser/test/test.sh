#!/bin/bash

./ddr_seq_parser if=test/x9_evb_lpddr4x_2133.c:test/x9_evb_lpddr4x_2133.c:test/x9_evb_lpddr4x_2133.c:test/x9_evb_lpddr4x_2133.c id=0x12:0x23:0x34:0x45 msk=0x01:0x02:0x03:0x04 of=ddr_seq1.bin
./ddr_seq_parser if=test/x9_evb_lpddr4x_2133.c of=ddr_seq2.bin
./ddr_seq_parser if=test/x9_evb_lpddr4x_2133.c of=ddr_seq3.bin gen_hdr

 ./ddr_seq_parser if=test/x9_evb_lpddr4x_2133.c:test/x9_evb_lpddr4x_2133.c:test/x9_evb_lpddr4x_2133.c:test/x9_evb_lpddr4x_2133.c:test/x9_evb_lpddr4x_2133.c:test/x9_evb_lpddr4x_2133.c:test/x9_evb_lpddr4x_2133.c:test/x9_evb_lpddr4x_2133.c id=0x12:0x23:0x34:0x45 msk=0x01:0x02:0x03:0x04 of=ddr_seq4.bin

 ./ddr_seq_parser if=test/x9_evb_lpddr4x_2133.c:test/x9_evb_lpddr4x_2133.c:test/x9_evb_lpddr4x_2133.c:test/x9_evb_lpddr4x_2133.c:test/x9_evb_lpddr4x_2133.c:test/x9_evb_lpddr4x_2133.c:test/x9_evb_lpddr4x_2133.c:test/x9_evb_lpddr4x_2133.c:test/x9_evb_lpddr4x_2133.c id=0x12:0x23:0x34:0x45 msk=0x01:0x02:0x03:0x04 of=ddr_seq5.bin

./ddr_seq_parser if=test/x9_evb_lpddr4x_2133.c:test/x9_evb_lpddr4x_2133.c:test/x9_evb_lpddr4x_2133.c:test/x9_evb_lpddr4x_2133.c id=0x12:0x23:0x34:0x45:0:0:0:0xff msk=0x01:0x02:0x03:0x04 of=ddr_seq6.bin
