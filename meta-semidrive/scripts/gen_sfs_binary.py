# -*- coding: utf-8 -*-
import sys
import json
import traceback
import getopt
import struct
from ctypes import create_string_buffer
from sfs_module import sfs_crc32


class GenSFS(object):

    def __init__(self):
        self.tag = 0x53465301
        # self.init_act_data = struct.pack("60x")
        self.init_act_data = create_string_buffer(60)
        self.xfer_data = struct.pack("16x")
        self.ip_data = struct.pack("16x")
        self.freq = 0
        self.training_pattern = struct.pack("8x")

    def parse_value(self, arg):
        if isinstance(arg, str) or isinstance(arg, unicode):
            return int(eval(str(arg)))
        else:
            return int(arg)

    def generate_sfs(self, jason_file, sfs_file):
        try:
            self.load_json(jason_file)
            buffer = struct.pack('<I'
                                 '60s'
                                 '16s'
                                 '16s'
                                 'B'
                                 '7x'
                                 '8s'
                                 'I'
                                 'I'
                                 '4x',
                                 self.tag,
                                 self.init_act_data.raw,
                                 self.xfer_data,
                                 self.ip_data,
                                 self.freq,
                                 self.training_pattern,
                                 0x0,
                                 0x0)

            crc32 = sfs_crc32(0, buffer, 124)
            # print(repr(buffer))
            # print("crc32=0x%X" % crc32)

            data = struct.pack('<124sI',
                               buffer,
                               crc32)

            image = open(sfs_file, 'wb')
            image.write(data)
            return True
        except ValueError as e:
            # error occurred.
            raise Exception("Failed to generate sfs file.")

    def load_json(self, jason_file):
        try:
            load_f = open(jason_file, 'r')
            obj = json.loads(load_f.read())

            # parse tag
            tag_obj = obj.get("tag")
            if tag_obj:
                self.tag = self.parse_value(tag_obj)

            # parse init_act_t
            init_act_t = obj.get("init_act_t")
            if init_act_t:
                count = 0
                for item in init_act_t:
                    struct.pack_into("BBHBB",
                                     self.init_act_data,
                                     count*6,
                                     self.parse_value(item["header"]),
                                     self.parse_value(item["command"]),
                                     self.parse_value(item["addr"]),
                                     self.parse_value(item["data0"]),
                                     self.parse_value(item["data1"]))
                    count += 1
            # print(repr(self.init_act_data.raw ))

            # parse xfer_attr_t
            xfer_obj = obj.get("xfer_attr_t")
            if xfer_obj:
                self.xfer_data = struct.pack("16B",
                                             self.parse_value(xfer_obj["cmd"]),
                                             self.parse_value(xfer_obj["cinst_type"]),
                                             self.parse_value(xfer_obj["caddr_type"]),
                                             self.parse_value(xfer_obj["cdata_type"]),
                                             self.parse_value(xfer_obj["caddr_size"]),
                                             self.parse_value(xfer_obj["cmode"]),
                                             self.parse_value(xfer_obj["cdummy_size"]),
                                             self.parse_value(xfer_obj["cflag"]),
                                             self.parse_value(xfer_obj["rsvd"][0]),
                                             self.parse_value(xfer_obj["rsvd"][1]),
                                             self.parse_value(xfer_obj["rsvd"][2]),
                                             self.parse_value(xfer_obj["rsvd"][3]),
                                             self.parse_value(xfer_obj["rsvd"][4]),
                                             self.parse_value(xfer_obj["rsvd"][5]),
                                             self.parse_value(xfer_obj["rsvd"][6]),
                                             self.parse_value(xfer_obj["rsvd"][7]),
                                             )
            # print(repr(self.xfer_data))

            # parse ip_settings_t
            ip_setting_obj = obj.get("ip_settings_t")
            if ip_setting_obj:
                self.ip_data = struct.pack("H14B",
                                           self.parse_value(ip_setting_obj["flags"]),
                                           self.parse_value(ip_setting_obj["rx_delay"]),
                                           self.parse_value(ip_setting_obj["tx_delay"]),
                                           self.parse_value(ip_setting_obj["csda"]),
                                           self.parse_value(ip_setting_obj["csdada"]),
                                           self.parse_value(ip_setting_obj["cseot"]),
                                           self.parse_value(ip_setting_obj["cssot"]),
                                           self.parse_value(ip_setting_obj["min_rx_win"]),
                                           self.parse_value(ip_setting_obj["rx_training_step"]),
                                           self.parse_value(ip_setting_obj["ctrl_misc"]),
                                           self.parse_value(ip_setting_obj["rsvd"][0]),
                                           self.parse_value(ip_setting_obj["rsvd"][1]),
                                           self.parse_value(ip_setting_obj["rsvd"][2]),
                                           self.parse_value(ip_setting_obj["rsvd"][3]),
                                           self.parse_value(ip_setting_obj["rsvd"][4])
                                           )
            # print(repr(self.ip_data))

            # parse freq
            freq_obj = obj.get("freq")
            if freq_obj:
                self.freq = self.parse_value(freq_obj)

            # parse training_pattern
            training_obj = obj.get("training_pattern")
            if training_obj:
                self.training_pattern = struct.pack("8B",
                                                    self.parse_value(training_obj[0]),
                                                    self.parse_value(training_obj[1]),
                                                    self.parse_value(training_obj[2]),
                                                    self.parse_value(training_obj[3]),
                                                    self.parse_value(training_obj[4]),
                                                    self.parse_value(training_obj[5]),
                                                    self.parse_value(training_obj[6]),
                                                    self.parse_value(training_obj[7])
                                                    )
            # print(repr(self.training_pattern))

        except ValueError as e:
            # Unfortunately we can't easily get the line number where the
            # error occurred.
            raise Exception("Failed to Load jason file.")


def show_usage():
    print("gen_sfs_binary version: 1.0.0.1")
    print("usage: gen_sfs_binary.py <--json file> <--out file> [-h]")
    print("options:")
    print("--json file  Specify nor flash configuration")
    print("--out file   Output specify sfs binary file")
    print("-h           help")
    print("e.g.: gen_sfs_binary.py --out ./sfs.img")


def main(argv):
    """
    主函数
    :param argv: <--out file> [-h]
    :return: 0:pass; other:fail
    """
    exit_code = 1
    try:
        opts, args = getopt.getopt(argv, "-h", ["help", "json=", "out="])
        out_sfs_file = ""
        json_file = ""
        for opt, arg in opts:
            if opt == '-h':
                show_usage()
                sys.exit(0)
            elif opt == "--out":
                out_sfs_file = arg
            elif opt == "--json":
                json_file = arg

        if len(out_sfs_file) == 0:
            show_usage()
            raise Exception("Not specify output sfs binary file info is empty.")

        if len(json_file) == 0:
            show_usage()
            raise Exception("Not input nor flash jason configuration file.")

        obj = GenSFS()
        if not obj.generate_sfs(json_file, out_sfs_file):
            raise Exception("[Error] Failed to sfs binary file.")

    except Exception:
        print("[Error] %s" % traceback.format_exc())
    else:
        print("Successfully generated sfs binary file!")
        exit_code = 0

    return exit_code


if __name__ == '__main__':
    main(sys.argv[1:])