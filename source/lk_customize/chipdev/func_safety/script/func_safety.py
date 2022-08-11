
# -*- coding: utf-8 -*-

import pandas as pd
import re


sem_sheet     = r"c:\projects\x9\function_safety\hw\dma_irq_v0.5.4.xlsx"
monitor_sheet = r"c:\projects\x9\function_safety\hw\SignalMonitor.xlsx"
eic_sheet     = r"c:\projects\x9\function_safety\hw\EIC.xlsx"

sem_header    = r"c:\projects\x9\function_safety\hw\sem_hw.h"
eic_header    = r"c:\projects\x9\function_safety\hw\eic_hw.h"


def c_token(s):
    return re.sub("[^a-zA-Z0-9_]", "_", s)


def parse_sem(sem_sheet, monitor_sheet, sem_header):
    with open(sem_header, "w") as fh:
        fh.write("#ifndef _SEM_HW_H\n")
        fh.write("#define _SEM_HW_H\n\n")

        fh.write("/**********************************************************\n"
                 " SEM signals generated from %s.\n" % sem_sheet)
        fh.write(" DO NOT modify!\n"
                 "**********************************************************/\n")

        data = pd.read_excel(sem_sheet, sheet_name="SEM")
        fh.write("enum sem_input {\n")
        for i in range(len(data)):
            spi = data["SPI Index"][i]
            module = data["MODULE"][i]
            pin = data["PIN"][i]
            trigger = data["Trigger"][i]
            description = data["Description"][i]
            description = description if type(description) is str else ""
            module = c_token(module)
            pin = c_token(pin)
            fh.write("   %s____%s = %d,\t\t// %s. %s\n" %
                     (module, pin, spi, trigger, description))
        fh.write("};\n")

        data = pd.read_excel(monitor_sheet, sheet_name="Sheet1")
        fh.write("\nenum sem_monitor_sig {\n")
        for i in range(len(data)):
            slot = int(data["Monitor slot"][i])
            signal = c_token(data["Monitor context"][i])
            comment = data["Comment"][i]
            fh.write("\t%s = %d,\t\t// %s\n" % (signal, slot, comment))
        fh.write("};\n")

        fh.write("#endif\n")

def parse_eic(eic_sheet, eic_header):
        data = pd.read_excel(eic_sheet, sheet_name="EIC")

        with open(eic_header, "w") as fh:
            fh.write("#ifndef _EIC_HW_H\n"
                     "#define _EIC_HW_H\n\n")

            fh.write("/**********************************************************\n"
                     " EIC injection points and signals generated from %s.\n"
                     " DO NOT modify!\n" % eic_sheet)
            fh.write("**********************************************************/\n\n")

            fh.write("/* EIC signal id format:\n"
                     " [31:24] eic_id (0: eic_safe, 1: eic_sec, 2: eic_hpi, 3: eic_vsn\n"
                     " [23:16] eic enable bit (bit offset of EN register)\n"
                     " [15:0]  eic inject bit (bit offset of BIT register)\n"
                     "*/\n")

            # _eic_id MUST be same with enum eic, defined in eic.c
            _eic_id = {
                "eic_safe": 0,
                "eic_sec":  1,
                "eic_hpi":  2,
                "eic_vsn":  3,
            }

            fh.write("enum eic_signal {\n")

            #
            # EIC signal naming convention & coding
            # E38____eic_hpi____wreadycode = (eic_id << 24) | (enable_bit << 16) | inject_bit
            # E38____eic_hpi____ridcode_4_ = (eic_id << 24) | (enable_bit << 16) | inject_bit
            #
            def print_eic_pin(eic, point, pin, pin_idx, enable_bit, inject_bit):
                eic_id = _eic_id[eic]
                idx = "" if pin_idx < 0 else "_%d_" % pin_idx
                fh.write("\t%s____%s____%s%s\t\t = 0x%08x,\n" %
                         (point, eic, pin, idx, (eic_id << 24) + (enable_bit << 16) + inject_bit))

            for i in range(len(data)):
                eic = data["Injection Control"][i]
                if type(eic) is str:
                    point_enable_bit = int(data["Enable Control"][i].split("[")[1].split("]")[0])
                    point = data["Injection Point"][i]
                    inject_bit_end, inject_bit_start = list(map(int,
                        re.sub("[\[\] ]", "", data["Injection Control Offset"][i]).split(":")))
                    pin_field = data["Field"][i]
                    if ':' in pin_field:
                        # Field has multiple pins
                        pin = c_token(pin_field.split('[')[0])
                        pin_end, pin_start = list(map(int,
                            re.sub("[\[\] ]", "", pin_field.split('[')[1]).split(":")))
                        for idx in range(0, pin_end - pin_start + 1):
                            print_eic_pin(eic, point, pin, pin_end - idx, point_enable_bit, inject_bit_end - idx)
                    else:
                        pin_field = c_token(pin_field)
                        print_eic_pin(eic, point, pin_field, -1, point_enable_bit, inject_bit_end)

            fh.write("};\n")
            fh.write("#endif\n")

if __name__ == "__main__":
    parse_sem(sem_sheet, monitor_sheet, sem_header)
    parse_eic(eic_sheet, eic_header)
