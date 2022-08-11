# -*- coding: utf-8 -*-

import pandas as pd
import re
import sys

scr_sheet, scr_header = sys.argv[1:]

scr_id = {
    "SCR_SAFETY":   0,
    "SCR_SEC":      1,
    "SCR_HPI":      2,
}

scr_type = {
    "RW":       0,
    "RO":       1,
    "L16":      2,
    "L31":      3,
    "R16W16":   4,
}

def extract_scr_addr(scr_addr):
    # APB_SCR_SEC_BASE+(0x684<<10)
    r = re.compile(".*\+\((.*)<<.*")
    m = r.match(scr_addr)
    return m[1]

"""
48 bits SCR signal format

[47:40]  SCR ID (safety/sec/hpi)
[39:32]  SCR type (RO, L16, L31, RW, R16W16)
[31:16]  SCR register addr
[15:8]   start bit (0~31)
[7:0]    width (1~32)
"""
def scr_signal(_scr_id, _scr_type, _scr_addr, _start_bit, _width):
    return ((scr_id[_scr_id] << 40) +
            (scr_type[_scr_type] << 32) +
            (_scr_addr << 16) +
            (_start_bit << 8) +
            (_width << 0))


def parse_scr_sheet(scr_sheet, scr_header):
    with open(scr_header, "w") as fh:
        fh.write("#ifndef _SCR_HW_H\n"
                 "#define _SCR_HW_H\n\n")

        for scrid in scr_id.keys():
            fh.write("\n// =============== %s =============== //\n" % scrid)

            data = pd.read_excel(scr_sheet, sheetname=scrid)
            for i in range(len(data)):
                _scr_type, _scr_bitname, _scr_addr, _start_bit, _width = (
                    data["SCR type"][i],
                    data["SCR bit"][i],
                    int(extract_scr_addr(data["SCR addr"][i]), 16),
                    int(data["start bit"][i]),
                    int(data["width"][i]))

                _scr_signal = scr_signal(scrid, _scr_type, _scr_addr, _start_bit, _width)
                print("%s 0x%x %d %d 0x%xull" %
                      (_scr_bitname, _scr_addr, _start_bit, _width, _scr_signal))
                fh.write("#define %s__%s__%s\t\t0x%016xull\n" %
                         (scrid, _scr_type, _scr_bitname, _scr_signal))

        fh.write("\n\n#endif /* _SCR_HW_H */\n")

if __name__ == "__main__":
    parse_scr_sheet(scr_sheet, scr_header)
