#!/usr/bin/env python3

import subprocess as sp
import argparse
import json


def parse_args():
  parser = argparse.ArgumentParser()
  parser.add_argument('packages', nargs="+")
  parser.add_argument('--at-least-version', required=False)
  return parser.parse_args()


def call_pkg_config(packages, flag):
  output = sp.check_output(['pkg-config'] + packages + ['--{}'.format(flag)],
                           text=True)
  return list(map(str, output.split()))


def main():
  args = parse_args()
  if args.at_least_version:
    call_pkg_config(args.packages, 'atleast-version='
                    + args.at_least_version)
  data = dict()
  data['cflags'] = call_pkg_config(args.packages, 'cflags')
  libs = call_pkg_config(args.packages, 'libs-only-l')
  libs = [lib[2:] if lib.startswith('-l') else lib for lib in libs]
  data['libs'] = libs
  data['lib_dirs'] = call_pkg_config(args.packages, 'libs-only-L')
  print(json.dumps(data))


if __name__ == "__main__":
  main()
