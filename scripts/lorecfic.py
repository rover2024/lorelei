from __future__ import annotations;

import argparse
import json
import shlex
import sys
import subprocess
import os
import re
import shutil

from dataclasses import dataclass
import json
import shlex
from typing import Optional, List

@dataclass
class CompileCommand:
    directory: str
    file: str
    command: Optional[str] = None
    arguments: Optional[List[str]] = None

    @property
    def args(self) -> List[str]:
        if self.arguments is not None:
            return self.arguments
        return shlex.split(self.command) if self.command else []
    
    @staticmethod
    def from_json(path: str) -> List[CompileCommand]:
        with open(path, 'r') as f:
            return [CompileCommand(**{
                k: v for k, v in entry.items() 
                if k in CompileCommand.__annotations__
            }) for entry in json.load(f)]


def get_name_without_ext(path: str) -> str:
    return os.path.splitext(os.path.basename(path))[0]


def read_list_file_as_list(filename: str) -> list[str]:
    with open(filename) as file:
        symbols = [line.strip() for line in file if line.strip()]
    res: list[str] = []
    for symbol in symbols:
        res.append(symbol)
    return res


def main():
    parser = argparse.ArgumentParser(description='Read compile commands and add CFIs.')
    parser.add_argument('tool_dir', type=str, help='Directory of Lorelei CFI tools.')
    parser.add_argument('callbacks_file', type=str, help='File contains list of callbacks.')
    parser.add_argument('compile_commands', type=str, help='Compile commands file path.')
    parser.add_argument('--files', type=str, required=False, help='File contains list of file names to process.')
    args = parser.parse_args()

    tool_dir: str = os.path.abspath(args.tool_dir)
    callbacks_file: str = os.path.abspath(args.callbacks_file)
    compile_commands_file: str = os.path.abspath(args.compile_commands)

    files: set[str] = set()
    if args.files:
        files_file: str = os.path.abspath(args.files)
        if os.path.exists(files_file):
            files_list = read_list_file_as_list(files_file)
            for file in files_list:
                if os.path.exists(file):
                    file = os.path.normpath(os.path.abspath(file))
                    files.add(file)

    cfic_step1_tool = os.path.join(tool_dir, 'lorecfic_step1')
    cfic_step2_tool = os.path.join(tool_dir, 'lorecfic_step2')
    cfic_tool = os.path.join(tool_dir, 'lorecfic')

    if not os.path.exists(cfic_step1_tool) or not os.path.exists(cfic_step2_tool) or not os.path.exists(cfic_tool):
        print(f'Error: Lorelei CFI tools not found in {tool_dir}')
        sys.exit(-1)

    def run_command(cmds: List[str], cwd: str):
        print(' '.join(cmds))
        process = subprocess.Popen(
            cmds,
            stdout=sys.stdout,
            stderr=sys.stderr,
            cwd=cwd,
        )
        process.wait()
        print(f'Exit code: {process.returncode}')
        if process.returncode != 0:
            sys.exit(-1)

    compile_commands = CompileCommand.from_json(compile_commands_file)
    compile_commands_count = len(compile_commands)
    for i in range(0, compile_commands_count):
        compile_command = compile_commands[i]

        dir: str = compile_command.directory
        filename: str = compile_command.file
        source_file = os.path.normpath(filename if os.path.isabs(filename) else os.path.join(dir, filename))

        print(f'[{i + 1}/{compile_commands_count}] Preprocessing {source_file}')

        if len(files) > 0 and not source_file in files:
            print('SKIPPED')
            continue

        temp_file_org = os.path.join(os.path.dirname(source_file), f'{get_name_without_ext(filename)}__cfic_tmp_org__.c')
        temp_file_pp = os.path.join(os.path.dirname(source_file), f'{get_name_without_ext(filename)}__cfic_tmp_pp__.c')
        temp_file_new = os.path.join(os.path.dirname(source_file), f'{get_name_without_ext(filename)}__cfic_tmp_new__.c')

        tokens = compile_command.args
        cc_tool = tokens[0]
        
        args_no_output: list[str] = []
        args_cc: list[str] = []
        if True:
            j = 1
            while j < len(tokens):
                if tokens[j] == '-o':
                    j += 2
                    continue
                args_no_output.append(tokens[j])
                j += 1
            
            j = 1
            while j < len(tokens):
                if tokens[j] == '-o':
                    j += 2
                    continue
                if tokens[j] == '-c':
                    j += 1
                    continue

                maybe_path = tokens[j]
                if os.path.basename(maybe_path) == os.path.basename(source_file):
                    if not os.path.isabs(maybe_path):
                        maybe_path = os.path.join(dir, maybe_path)
                    if os.path.samefile(maybe_path, source_file):
                        j += 1
                        continue

                args_cc.append(tokens[j])
                j += 1
        
        # 1. Run CFI step1
        if True:
            cmds = [ cfic_step1_tool, '-o', temp_file_org, source_file, '--' ] + args_no_output
            run_command(cmds, dir)
        
        if os.path.exists(temp_file_org):
            # 2. Run CC -E
            if True:
                cmds: list[str] = [ cc_tool, '-E', '-P', '-C', temp_file_org, '-o', temp_file_pp ] + args_cc
                run_command(cmds, dir)
            
            # 3. Run CFI step2
            if True:
                cmds = [ cfic_step2_tool, temp_file_org, temp_file_pp, temp_file_new ]
                run_command(cmds, dir)
        else:
            shutil.copyfile(source_file, temp_file_new)
            

        # 4. Run CFIC
        if True:
            cmds = [ cfic_tool, '-c', callbacks_file, '-o', source_file, temp_file_new, '--' ] + args_no_output
            run_command(cmds, dir)

        # sys.exit(1)

        if os.path.exists(temp_file_org):
            os.remove(temp_file_org)
        if os.path.exists(temp_file_pp):
            os.remove(temp_file_pp)
        if os.path.exists(temp_file_new):
            os.remove(temp_file_new)


if __name__ == '__main__':
    main()