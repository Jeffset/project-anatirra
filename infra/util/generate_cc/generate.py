import argparse
import datetime
import os
import re
import subprocess

from infra import base

PROJECT_NAME = 'anatirra'
HEADER_EXT = '.hpp'
SOURCE_EXT = '.cc'
GN_EXT = '.gn'
GNI_EXT = '.gni'

CC_COMMENT = r'(?:^(?:\s*(?://.*)?(?:/\*(?:.|\n)*?\*/)?)$\n)*'
CC_HEADER_PREAMBLE = re.compile(
    CC_COMMENT + r'(?:^#ifndef (.*?)$\n#define \1$)?', re.M)
CC_SOURCE_PREAMBLE = re.compile(CC_COMMENT, re.M)
CC_HEADER_EPILOGUE = re.compile(r'#endif.*$', 0)


class GenerateError(BaseException):
    def __init__(self, message):
        super().__init__()
        self.message = message


def get_copyright(comment_begin):
    return '{cb} Copyright (C) {} Marco Jeffset (f.giffist@yandex.ru)\n' \
           '{cb} This software is a part of the Anatirra Project.\n' \
           '{cb} "Nothing is certain, but we shall hope."\n'.format(datetime.datetime.now().year,
                                                                    cb=comment_begin)


def to_project_path(path: str):
    if not path.startswith(base.PROJECT_ROOT_PATH):
        raise GenerateError(
            'Path "{}" does not belong to project.'.format(path))
    return os.path.relpath(path, base.PROJECT_ROOT_PATH)


def include_guard_macro(header_path: str):
    if not header_path.endswith(HEADER_EXT):
        raise GenerateError('Path "{}" is not header path - no "{}" ext detected.'
                            .format(header_path, HEADER_EXT))
    header_path = header_path[:-len(HEADER_EXT)]
    path = to_project_path(header_path)
    return (PROJECT_NAME + '_' +
            ''.join(c if c.isalpha() or c.isdigit() else '_' for c in path)).upper()


def header_preamble(header):
    macro = include_guard_macro(header)
    return '\n'.join([get_copyright('//'),
                      '#ifndef {macro}\n'
                      '#define {macro}\n\n'.format(macro=macro)
                      ])


def header_epilogue(header):
    macro = include_guard_macro(header)
    return '#endif  // {macro}\n'.format(macro=macro)


def generate_source(source, header=None, content=None):
    with(open(source, 'w')) as output:
        output.write(get_copyright('//'))
        if header:
            output.write('#include "{}"\n\n'.format(to_project_path(header)))
        if content:
            output.write('\n' + content)


def generate_header(header, content=None):
    with open(header, 'w') as output:
        output.write(header_preamble(header))
        if content:
            output.write(content)
        output.write(header_epilogue(header))


def generate_header_command(name, path, force, **_):
    header = os.path.join(path, name + HEADER_EXT)
    print('Generating header "{}"'.format(header))
    if os.path.exists(header) and not force:
        raise GenerateError('Header already exists. Use --force to overwrite.')
    generate_header(header)
    print('Generation successful.')


def generate_pair_command(name, path, force, **_):
    header = os.path.join(path, name + HEADER_EXT)
    source = os.path.join(path, name + SOURCE_EXT)
    print('Generating header "{}"'.format(header))
    if os.path.exists(header) and not force:
        raise GenerateError(
            'Header file already exists. Use --force to overwrite.')
    generate_header(header)

    print('Generating source "{}"'.format(source))
    if os.path.exists(source) and not force:
        raise GenerateError(
            'Source file already exists. Use --force to overwrite.')
    generate_source(source, header)
    print('Generation successful.')


def update_file(paths, update_frame, reformat, **_):
    for path in paths:
        path = os.path.abspath(path)

        if update_frame:
            print('Updating frame for "{}"'.format(path))

            if path.endswith(HEADER_EXT) or path.endswith(SOURCE_EXT):
                with(open(path, 'r')) as source:
                    text = source.read()

                if path.endswith(HEADER_EXT):
                    ok = text.startswith(header_preamble(
                        path)) and text.endswith(header_epilogue(path))
                    if not ok:
                        print('Frame is invalid for header')
                        text = CC_HEADER_PREAMBLE.sub('', text, 1)
                        text = CC_HEADER_EPILOGUE.sub(
                            '', text, 1).strip() + '\n\n'
                        generate_header(path, content=text)
                    else:
                        print('Header is already fine.')
                else:
                    ok = text.startswith(get_copyright('//'))
                    if not ok:
                        print('Frame is invalid for source')
                        text = CC_SOURCE_PREAMBLE.sub('', text, 1)
                        generate_source(path, header=None, content=text)
                    else:
                        print('Source is already fine.')
            else:
                print('Unsupported file for frame update')

        if reformat:
            if path.endswith(GN_EXT) or path.endswith(GNI_EXT):
                print('Running gn format on "{}"'.format(path))
                subprocess.check_call(['gn', 'format', path])
            elif path.endswith(HEADER_EXT) or path.endswith(SOURCE_EXT):
                print('Running clang-format on "{}"'.format(path))
                subprocess.check_call(['clang-format', '-i', path])
            else:
                print('Unsupported file for reformat')


def parse_args():
    base = argparse.ArgumentParser(add_help=False)
    base.add_argument('--force', action='store_true', help='Whether to overwrite existing files. '
                                                           'Use with care!')
    base.add_argument('path', nargs='?', default=os.getcwd(),
                      help='Path in project where to create file. '
                           'Current working dir is used by default.')
    parser = argparse.ArgumentParser(
        add_help=True, description="Utility script that generates c++ source file stubs.")

    sub = parser.add_subparsers(
        description='Perform specific generation/update task')

    header = sub.add_parser('header', parents=(
        base,), description='Generates single header file.')
    header.add_argument('name', help='Header file name (without extension).')
    header.set_defaults(func=generate_header_command)

    pair = sub.add_parser('pair', parents=(
        base,), description='Generates header/source file pair.')
    pair.add_argument('name', help='Source file name (without extension).')
    pair.set_defaults(func=generate_pair_command)

    update = sub.add_parser('update', description='Updates project file to comply '
                                                  'with project code-style.')
    update.add_argument('paths', nargs='+',
                        help='Project file path to work with.')
    update.add_argument('--update-frame', '-G', action='store_true',
                        help='Whether to update header include guard if possible.')
    update.add_argument('--reformat', '-X', action='store_true',
                        help='Whether to run file-type-specific format tool if possible.')
    update.set_defaults(func=update_file)

    args = parser.parse_args()
    if not hasattr(args, 'func'):
        parser.print_usage()
        exit(1)
    return args


def main():
    args = parse_args()
    try:
        args.func(**args.__dict__)
    except GenerateError as e:
        print(e.message)
        exit(1)
