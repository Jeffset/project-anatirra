#!/usr/bin/env python3
import argparse
import os
import sys
import re

TEST_SCRIPT = """#!/usr/bin/env python3
import subprocess
if __name__ == '__main__':
  ret_code = subprocess.call({cmd})
  expects = '{expects}'
  if expects == 'COMPILES':
    assert ret_code == 0
  elif expects == 'FAILS':
    assert ret_code != 0
  else:
    assert False
"""

HEADER_RE = re.compile(r'// COMPILER TEST EXPECTS: (COMPILES|FAILS)')


def parse_args():
  p = argparse.ArgumentParser()
  p.add_argument('-c', dest='source', required=True)
  known, rest = p.parse_known_args()
  return known


if __name__ == '__main__':
  output = sys.argv[1]

  args = parse_args()

  with open(args.source, 'r') as source_file:
    header = source_file.readline().strip()
    match = HEADER_RE.fullmatch(header)
    assert match, "'{}'".format(header)
    expects = match.group(1)

  with open(output, 'w') as out:
    out.write(TEST_SCRIPT.format(cmd=sys.argv[2:],
                                 expects=expects))
  os.chmod(output, 0o775)
