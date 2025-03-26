#!/bin/python

from __future__ import annotations

import os
import sys
import argparse
import subprocess
import re


class Common:
    source_file_extensions: list[str] = ['.c', '.cc', '.cxx', '.cpp', 'm', 'mm']
    ignored_system_headers: list[str] = [
        'stdc-predef.h'
    ]


def is_descendant(file_path, directory_path):
    file_path = os.path.abspath(file_path)
    directory_path = os.path.abspath(directory_path)
    return os.path.commonpath([file_path]) == os.path.commonpath([file_path, directory_path])


def main():
    parser = argparse.ArgumentParser(description='Remove code content that is not part of the project')
    parser.add_argument('dir', metavar='<dir>', help='Project directory')
    parser.add_argument('-o', metavar='<output>', required=False, help='Output file path')
    parser.add_argument('--expand-headers', type=bool, required=False, default=False, help='Whether to expand project header files')
    parser.add_argument('--args', nargs=argparse.REMAINDER, metavar='<compiler args>', default=[], help='Compiler arguments')
    args = parser.parse_args()

    # Collect arguments
    project_dir:str = args.dir
    compile_args: list[str] = args.args
    output_file: str = args.o if args.o else ''
    expand_headers: bool = args.expand_headers

    # Run CC and get preprocess result
    process = subprocess.Popen(
        compile_args,
        stdout=subprocess.PIPE,
        stderr=sys.stderr,
        text=True
    )
    content, _ = process.communicate()
    if process.returncode != 0:
        return
    
    """
    https://stackoverflow.com/questions/5370539/what-is-the-meaning-of-lines-starting-with-a-hash-sign-and-number-like-1-a-c
    https://gcc.gnu.org/onlinedocs/cpp/Preprocessor-Output.html

    # line-number "source-file" [flags]
    1 - Start of a new file
    2 - Returning to previous file
    3 - Following text comes from a system header file (#include <> vs #include "")
    4 - Following text should be treated as being wrapped in an implicit extern "C" block."
    """

    def is_ignored_file(path: str):
        if not expand_headers:
            _, file_extension = os.path.splitext(path)
            if not file_extension.lower() in Common.source_file_extensions:
                return True
        return os.path.isabs(path) and os.path.exists(path) and not is_descendant(path, project_dir)
    
    lines = content.splitlines()
    level = 0
    new_lines: list[str] = []
    for line in lines:
        match = re.search(r'^# (\d+) "([^"]+)"(( \d+)*)$', line)
        if match:
            line_num = int(match.group(1)) # unused
            filename = match.group(2)
            numbers: list[int] = [int(item) for item in match.group(3).strip().split(' ')] if match.group(3) else []
            is_system = 3 in numbers

            if 1 in numbers:
                if level > 0:
                    level += 1
                elif is_ignored_file(filename):
                    if not os.path.basename(filename) in Common.ignored_system_headers:
                        if is_system:
                            new_lines.append(f'#include <{filename}>')
                        else:
                            new_lines.append(f'#include \"{filename}\"')
                    level += 1
            elif 2 in numbers:
                if level > 0:
                    level -= 1
            
            continue
        
        if level == 0:
            new_lines.append(line)
    
    if output_file == '':
        for line in new_lines:
            print(line)
    else:
        with open(output_file, 'w') as f:
            for line in new_lines:
                print(line, file=f)


if __name__ == '__main__':
    main()