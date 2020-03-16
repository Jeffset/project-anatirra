import colorama
import argparse
import subprocess
import logging
import os

from infra import base


def parse_args():
    parser = argparse.ArgumentParser(description='Runs tests from project-anatirra',
                                     add_help=True)
    parser.add_argument('outdir', help='gn output directory.')
    parser.add_argument('--targets', help='test binaries to run',
                        nargs='+', required=False)
    base.add_verbosity_args(parser)
    return parser.parse_args()


def main():
    args = parse_args()
    base.setup_logging_with_verbosity(args)

    command = ('gn', 'ls', args.outdir)
    output = subprocess.check_output(command, universal_newlines=True,
                                     cwd=base.PROJECT_SRC_PATH)
    logging.debug('Result of %s is:\n%s', command, output)

    targets = [target[2:] for target in output.split()
               if target.endswith('_tests')]
    logging.debug('Found test targets: %s', targets)

    build_command = ('ninja', '-C', args.outdir, *targets)
    logging.debug('Build command is %s', build_command)
    subprocess.check_call(build_command, cwd=base.PROJECT_SRC_PATH)
    logging.debug('Targets are built.')

    build_dir = os.path.join(base.PROJECT_SRC_PATH, args.outdir)
    for target in targets:
        _, _, binary_name = target.rpartition(':')
        if args.targets is not None and binary_name not in args.targets:
            logging.debug('Skipping %s - it is not listed in --targets',
                          binary_name)
            continue

        logging.info('Running target %s (%s)', target, binary_name)
        if binary_name.endswith('_compiler_tests'):
            binary = os.path.join(build_dir, binary_name + '__runner')
        else:
            binary = os.path.join(build_dir, binary_name)
        logging.debug('Running file %s in %s', binary, build_dir)
        subprocess.check_call((binary,), cwd=build_dir)
