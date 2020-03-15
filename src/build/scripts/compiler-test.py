#!/usr/bin/env python3

import argparse
import os
import re
from typing import Optional, List, Iterable

TEST_SCRIPT = """#!/usr/bin/env python3

import subprocess
import os

TESTS = {tests}
CMDLINE = {cmdline}

if __name__ == '__main__':
  processes = []
  print('Running compiler tests')
  for i, test in enumerate(TESTS):
    code_under_test = test['code']
    process = subprocess.Popen(CMDLINE + ['-x', 'c++', '-o', '/tmp/tmp{{}}.o'.format(i), '-'],
                               env=os.environ, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                               text=True, stdin=subprocess.PIPE)
    process.stdin.write(code_under_test)
    process.stdin.close()
    processes.append(process)

  for process in processes:
    process.wait()

  failed_tests = []
  for process, test in zip(processes, TESTS):
    expectation = test['expectation']
    name = test['name']
    err = process.stderr.read()
    if expectation is True and process.returncode != 0:
      failed_tests.append(test)
      print(name + ' FAILED: expected to compile, but compilation failed:')
      print(err)
    elif expectation is False and process.returncode == 0:
      failed_tests.append(test)
      print(name + ' FAILED: expected to fail, but compilation succeeded.')
    elif isinstance(expectation, str) and (process.returncode == 0 or
                                           expectation not in err):
      print(
        name + ' FAILED: expected to fail with "{{}}", but stderr does not contain this:'.format(
          expectation))
      print(err)
      failed_tests.append(test)
    else:
      print(name + ' OK')
    print('---------')

  for failed_test in failed_tests:
    print('Test failed: {{}}'.format(failed_test['name']))

  if failed_tests:
    exit(1)
"""

HEADER_RE = re.compile(r'#error COMPILER TEST EXPECTS: (COMPILES|FAILS)')


def parse_args():
  p = argparse.ArgumentParser()
  p.add_argument('-c', dest='source', required=True)
  p.add_argument('-o', dest='output', required=True)
  p.add_argument('-MF', dest='depfile', required=True)
  known, rest = p.parse_known_args()
  return known, rest


class SourceParseException(Exception):
  def __init__(self, message):
    super().__init__(message)


class TestHeaderParser:
  """
  TEST_HEADER ::= '#test' STRING EXPECTATION
  STRING ::= '"'.*'"'
  EXPECTATION ::= 'must' ( 'compile' | 'fail' [ 'with' STRING ] )
  """
  TEST_BEGIN = 'test'
  EXPECTATION_SEPARATOR = '::'

  def __init__(self):
    self.state = self.parse_test
    self.name: Optional[str] = None
    self.expectation = None
    self.valid = False

  def feed(self, word: str):
    self.state(word)

  def parse_test(self, word: str):
    if word != self.TEST_BEGIN:
      raise SourceParseException('"{}" expected.'.format(self.TEST_BEGIN))
    self.state = self.parse_name

  def parse_name(self, word: str):
    self.name = self.parse_string(word)
    self.state = self.parse_expectation

  def parse_expectation(self, word):
    if word != 'must':
      raise SourceParseException('"must" expected')
    self.state = self.parse_expectation_impl

  def parse_expectation_impl(self, word: str):
    if word == 'compile':
      self.expectation = True
      self.valid = True
      self.state = None
    elif word == 'fail':
      self.expectation = False
      self.valid = True
      self.state = self.parse_error_with
    else:
      raise SourceParseException('"compile" or "fail" expected.')

  def parse_error_with(self, word: str):
    if word != 'with':
      raise SourceParseException('"with" expected.')
    self.valid = False
    self.state = self.parse_error_text

  def parse_error_text(self, word: str):
    self.expectation = self.parse_string(word)
    self.valid = True

  @staticmethod
  def parse_string(word: str):
    match = re.fullmatch(r'"(.*)"', word)
    if not match:
      raise SourceParseException('String expected.')
    return match.group(1)


class Test:
  def __init__(self):
    self.lines: List[str] = []
    self.name: Optional[str] = None
    self.expectation = None


class SourceParser:
  GLOBAL_HEADER = '#error COMPILER TEST'
  TESTS_BEGIN = '#pragma tests begin'
  TESTS_END = '#pragma tests end'

  def __init__(self):
    self.preamble_lines: List[str] = []
    self.tests: List[Test] = []
    self.epilogue_lines: List[str] = []
    self.state = self.global_header
    self.valid = False

  @staticmethod
  def tokenize(line: str) -> Iterable[str]:
    tokens = ['']
    inside_string = False
    for ch in line:
      if ch.isspace() and not inside_string:
        tokens.append('')
      elif ch == '"':
        inside_string = not inside_string
        tokens[-1] += ch
      else:
        tokens[-1] += ch
    return tokens

  def feed(self, line: str):
    self.state(line)

  def global_header(self, line: str):
    if line != self.GLOBAL_HEADER:
      raise SourceParseException('First compiler test line must be "{}"'.format(self.GLOBAL_HEADER))
    self.state = self.preamble

  def preamble(self, line: str):
    if line == self.TESTS_BEGIN:
      self.state = self.body
    else:
      self.preamble_lines.append(line)

  def body(self, line: str):
    if line == self.TESTS_END:
      self.state = self.epilogue
    elif line.lstrip().startswith('#pragma'):
      words = self.tokenize(line)[1:]
      test = Test()
      parser = TestHeaderParser()
      for word in words:
        parser.feed(word)
      if not parser.valid:
        raise SourceParseException('Incomplete test header.')
      test.expectation = parser.expectation
      test.name = parser.name
      self.tests.append(test)
    elif line:
      if not self.tests:
        raise SourceParseException('Test header expected.')
      test = self.tests[-1]
      test.lines.append(line)

  def epilogue(self, line: str):
    self.epilogue_lines.append(line)
    self.valid = True

  def make_tests(self):
    tests = []
    for test in self.tests:
      lines = []
      lines.extend(self.preamble_lines)
      lines.extend(test.lines)
      lines.extend(self.epilogue_lines)
      tests.append(dict(
        code='\n'.join(lines),
        name=test.name,
        expectation=test.expectation
      ))
    return tests


def main():
  args, rest_args = parse_args()

  with open(args.source, 'r') as source_file:
    parser = SourceParser()
    while True:
      line = source_file.readline()
      parser.feed(line.rstrip())
      if not line:
        break
    if not parser.valid:
      raise SourceParseException('Unexpected EOF.')

  with open(args.output, 'w') as out:
    out.write(TEST_SCRIPT.format(cmdline=rest_args,
                                 tests=parser.make_tests()))
  os.chmod(args.output, 0o775)

  with open(args.depfile, 'w') as depfile:
    depfile.write('{}: {}'.format(args.output, __file__))


if __name__ == '__main__':
  main()
