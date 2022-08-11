import sys
import os
import argparse

def write_template(filename, data):
    with open(filename, 'w') as f:
        f.write(data)

def build_template(testdir, testname):
    return ''.join([
        "#!/usr/bin/env sh\n",
        "export GST_STATE_IGNORE_ELEMENTS=''\n",
        "export CK_DEFAULT_TIMEOUT=20\n",
        "export GST_PLUGIN_LOADING_WHITELIST='gstreamer'\n",
        "{}\n".format(os.path.join(testdir, testname)),
    ])

argparser = argparse.ArgumentParser(description='Generate installed-test data.')
argparser.add_argument('--test-execdir', metavar='dir', required=True, help='Installed test directory')
argparser.add_argument('--testname', metavar='name', required=True, help='Installed test name')
argparser.add_argument('--output', metavar='file', required=True, help='Output file')
args = argparser.parse_args()

write_template(args.output, build_template(args.test_execdir, args.testname))
os.chmod(args.output, 0o755)
