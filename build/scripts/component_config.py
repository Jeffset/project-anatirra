#!/usr/bin/env python3

import argparse

BASE_CONFIG: str = """// Generated header file. Do not edit.

#if defined(COMPONENT_{name}_BUILD)
#define {cap_name}_PUBLIC  __attribute__ ((visibility ("default")))
#define {cap_name}_PRIVATE __attribute__ ((visibility ("hidden")))
#else
#define {cap_name}_PUBLIC
#define {cap_name}_PRIVATE
#endif  // defined(COMPONENT_{name}_BUILD)
"""

def parse_args():
  parser = argparse.ArgumentParser()
  parser.add_argument('--name', required=True)
  parser.add_argument('--output', required=True)
  parser.add_argument('--params', required=True, nargs='*')
  return parser.parse_args()

def main():
  args = parse_args()
  lines = [BASE_CONFIG.format(name=args.name, cap_name=args.name.upper())]
  for param in args.params:
    lines.append('#define ' + param)
  with open(args.output, 'w') as output:
    output.writelines(lines)


if __name__ == '__main__':
    main()