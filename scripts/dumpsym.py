#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys
import argparse
import subprocess

def get_exports(so_file_path, nm_cmd='nm'):
    """
    Get all exported symbols from a .so file using nm command
    :param so_file_path: Path to the .so file
    :param nm_cmd: Name or path of the nm command
    :return: (list of functions, list of variables)
    """
    try:
        # Use nm command to list defined symbols in dynamic symbol table
        # -D: Show only dynamic symbols
        # --defined-only: Show only defined symbols
        result = subprocess.run(
            [nm_cmd, '-D', '--defined-only', so_file_path],
            capture_output=True,
            text=True,
            check=True
        )
        
        functions = []
        variables = []
        
        # Parse nm output
        for line in result.stdout.splitlines():
            parts = line.split()
            if len(parts) < 2:
                continue
                
            # Symbol type
            # T/t: Text segment symbols (usually functions)
            # D/d: Initialized data segment symbols (usually variables)
            # B/b: Uninitialized data segment symbols (usually variables)
            # R/r: Read-only data segment symbols (usually constants)
            sym_type = parts[-2]
            sym_name = parts[-1]

            # if sym_name.startswith('__') or sym_name.startswith('_'):
            #     continue
            
            # Determine symbol type
            if sym_type in ('T', 't'):
                functions.append(sym_name)
            elif sym_type in ('D', 'd', 'B', 'b', 'R', 'r'):
                variables.append(sym_name)
        
        # Sort symbols
        functions.sort()
        variables.sort()
        
        return functions, variables
        
    except subprocess.CalledProcessError as e:
        print(f"Error: Failed to run {nm_cmd} command: {e}", file=sys.stderr)
        sys.exit(1)
    except FileNotFoundError:
        print(f"Error: Command {nm_cmd} not found", file=sys.stderr)
        sys.exit(1)
    except Exception as e:
        print(f"Error: {e}", file=sys.stderr)
        sys.exit(1)

def write_symbols(functions, variables, output_file_path):
    """
    Write symbols to output file
    :param functions: List of functions
    :param variables: List of variables
    :param output_file_path: Path to output file
    """
    try:
        # Ensure output directory exists
        output_dir = os.path.dirname(output_file_path)
        if output_dir and not os.path.exists(output_dir):
            os.makedirs(output_dir)
        
        with open(output_file_path, 'w') as f:
            # Write functions section
            f.write("[Function]\n")
            for func in functions:
                f.write(f"{func}\n")
            
            # If there are variables, add empty line and variables section
            if variables:
                f.write("\n[Variable]\n")
                for var in variables:
                    f.write(f"{var}\n")
        
        print(f"Symbols successfully written to {output_file_path}")
        
    except Exception as e:
        print(f"Error: Failed to write file: {e}", file=sys.stderr)
        sys.exit(1)

def main():
    """
    Main function to process command line arguments and perform operations
    """
    parser = argparse.ArgumentParser(description='Extract exported symbols and variables from .so file')
    parser.add_argument('so_file', help='Path to input .so file')
    parser.add_argument('output_file', help='Path to output txt file')
    parser.add_argument('--nm', default='nm', help='Name or path of nm command (default: nm)')
    
    args = parser.parse_args()
    
    # Check if input file exists
    if not os.path.exists(args.so_file):
        print(f"Error: Input file {args.so_file} not found", file=sys.stderr)
        sys.exit(1)
    
    # Get exported symbols
    print(f"Extracting symbols from {args.so_file}...")
    functions, variables = get_exports(args.so_file, args.nm)
    
    # Write to output file
    write_symbols(functions, variables, args.output_file)

if __name__ == '__main__':
    main()