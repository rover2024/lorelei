#!/usr/bin/env python3
import sys
import os
import re

def is_hex_string(s):
    return bool(re.search(r'[a-fA-F]', s))

def parse_address(addr_str):
    try:
        if addr_str.startswith('0x'):
            return int(addr_str, 16)
        
        if is_hex_string(addr_str):
            return int(addr_str, 16)
        
        return int(addr_str)
    except ValueError:
        raise ValueError(f"Invalid address format: {addr_str}")

def main():
    if len(sys.argv) != 3:
        print(f"Usage: {sys.argv[0]} <pid> <address>")
        sys.exit(1)
    
    pid, addr_str = sys.argv[1], sys.argv[2]
    
    # 解析地址
    try:
        address = parse_address(addr_str)
    except ValueError as e:
        print(e)
        sys.exit(1)
    
    maps_file = f"/proc/{pid}/maps"
    
    if not os.path.exists(maps_file):
        print(f"Error: Process {pid} or /proc/{pid}/maps does not exist or is not accessible")
        sys.exit(1)
    
    # Search for matching memory regions
    found = False
    try:
        with open(maps_file, 'r') as f:
            for line in f:
                parts = line.split()
                if not parts:
                    continue
                
                # Parse address range
                addr_range = parts[0]
                try:
                    start, end = addr_range.split('-')
                    start_addr = int(start, 16)
                    end_addr = int(end, 16)
                except ValueError:
                    continue
                
                # Check if address is within range
                if start_addr <= address < end_addr:
                    print(line.strip())
                    print(f"Offset: {hex(address - start_addr)}")
                    found = True
                    break
    except IOError as e:
        print(f"Error: Failed to read {maps_file}: {e}")
        sys.exit(1)
    
    if not found:
        print(f"Error: Address {hex(address)} is not mapped in process {pid}")
        sys.exit(1)

if __name__ == "__main__":
    main()