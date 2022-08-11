#coding=utf-8

import base64
from zlib import crc32 as CRC32
import serial
import sys
import time
import threading
import os
import struct
import threading

print("-------- Serial Load PC tool ----------")

if len(sys.argv) != 3:
    print("Usage: ./serial_loader.py [com] [image_file]")
    #print("\n\t\t if 'image_file' is empty, to tell spl to use default one")
    os._exit(1)

port = sys.argv[1]

if len(sys.argv) == 3:
    img_file_name = sys.argv[2]
    f_img = open(img_file_name,"rb")
    img = f_img.read()
    img_sz = len(img)

# This script is for DDR sequence (in binary) loading.
# The protocol here is very simple: 
#   Host sends 0x81, waiting device ack 0xC1. Or Host sends 0x82 to skip the load,
#   SPL will use default init sequence.
#   Host sends [Binary size + Binary + CRC32]
#   Device receives the payload then do CRC32 check on it, ack 0xC1 if CRC32 matchs
#   otherwise 0x1C.

def serial_init():
    global console
    global port
    console = serial.Serial(port, 115200, timeout = 10)
    console.flushInput()
    console.flushOutput()

    return True 

def crc32_calc(block):
    return CRC32(bytes(block))

if __name__ == "__main__":
    global console
    serial_init()
    
    #output_data=str.encode('help\n')
    if len(sys.argv) == 2:
         output_data = struct.pack("<B", 0x82)  # Skip loading
         print("Loading skipped, nothing loaded")
    else:
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
                    os._exit(0)

        print("Image xfer failed")
        os._exit(1)
