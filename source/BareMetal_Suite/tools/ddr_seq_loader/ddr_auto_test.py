#coding=utf-8

# Sandbox, not finished yet.

import base64
from zlib import crc32 as CRC32
import serial
import sys
import time
import threading
import os
import struct
import threading
import random

print("-------- Serial Load PC tool ----------")

if len(sys.argv) != 3:
    print("Usage: ./ddr_test_auto.py [com] [image_file]")
    #print("\n\t\t if 'image_file' is empty, to tell spl to use default one")
    os._exit(1)

port = sys.argv[1]

if len(sys.argv) == 3:
    img_file_name = sys.argv[2]
    f_img = open(img_file_name,"rb")
    img = f_img.read()
    img_sz = len(img)

def serial_init():
    global console
    global port
    console = serial.Serial(port, 115200, timeout = 10)
    console.flushInput()
    console.flushOutput()

    return True 

def crc32_calc(block):
    return CRC32(bytes(block))

# This script is for DDR sequence (in binary) loading.
# The protocol here is very simple: 
#   Host sends 0x81, waiting device ack 0xC1. Or Host sends 0x82 to skip the load,
#   SPL will use default init sequence.
#   Host sends [Binary size + Binary + CRC32]
#   Device receives the payload then do CRC32 check on it, ack 0xC1 if CRC32 matchs
#   otherwise 0x1C.

def load_seq():
    global console
    output_data = struct.pack("<B", 0x81)
    console.write(output_data)
    rx_data = int.from_bytes(console.read(1), byteorder='little', signed=False)
    if rx_data == 0xC1:
        console.write(struct.pack("<I", img_sz))
        rx_data = int.from_bytes(console.read(1), byteorder='little', signed=False)
        if rx_data == 0xC1:
            console.write(img)
            crc32 = struct.pack("<I",crc32_calc((img)))
            console.write(crc32)
            console.flushOutput()
            rx_data = int.from_bytes(console.read(1), byteorder='little',signed=False)
            if rx_data == 0xC1:
                print("Image xfer-ed")
                return True 
    return False

cmd_list = ['setper 2 6\n', 
            'setclk 2 DDR_0 ',
            'load_ddr_seq\n',
            'spl_init\n',
            'spl_boot\n',
            'reset\n'
            ]

if __name__ == "__main__":
    global console
    serial_init()
    
    #output_data=str.encode('help\n')
    line = ''
    index = 0
    while console.inWaiting() >0:
        c = console.read(1)
        line += c
        if (c == '\n'):
            print(line)
            if (line == ']\r\n'):
                if index == 1:
                    freq = 1066000000 * (random.random() * 0.06 + 0.97)
                    print "DDR PLL clock set as ", freq
                    cmd_list[1] += freq + '\n'
                cmd = str.encode(cmd_list[index])
                console.write(cmd)
                if (2 == index):
                    if False == load_seq():
                        print("Opps, failed to load ddr seq\n")
                        os._exit(1)
                if (cmd_list[index] == 'reset\n'):
                    index = 0
                else:
                    index++
            line = ''

    os._exit(0)
