import argparse
import logging
import os

PROJECT_ROOT_PATH = os.path.realpath(
  os.path.dirname(os.path.dirname(os.path.dirname(__file__))))

assert os.path.isfile(os.path.join(PROJECT_ROOT_PATH, '.gn')
                      ), '.gn file expected to be present'


def add_verbosity_args(parser: argparse.ArgumentParser):
  parser.add_argument('-v', action='count', dest='verbosity', default=0)


def setup_logging_with_verbosity(args: argparse.Namespace):
  level = [
    logging.WARNING,
    logging.INFO,
    logging.DEBUG
  ]
  logging.basicConfig(level=level[args.verbosity])
