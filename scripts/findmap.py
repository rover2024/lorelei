#!/usr/bin/env python3
import sys
import os
import re

def is_hex_string(s):
    """检查字符串是否包含十六进制字符（A-Fa-f）"""
    return bool(re.search(r'[a-fA-F]', s))

def parse_address(addr_str):
    """解析地址字符串，自动检测十六进制格式"""
    try:
        # 如果以0x开头，直接解析为十六进制
        if addr_str.startswith('0x'):
            return int(addr_str, 16)
        
        # 如果包含十六进制字符，解析为十六进制
        if is_hex_string(addr_str):
            return int(addr_str, 16)
        
        # 否则解析为十进制
        return int(addr_str)
    except ValueError:
        raise ValueError(f"无效的地址格式: {addr_str}")

def main():
    if len(sys.argv) != 3:
        print(f"用法: {sys.argv[0]} <pid> <address>")
        sys.exit(1)
    
    pid, addr_str = sys.argv[1], sys.argv[2]
    
    # 解析地址
    try:
        address = parse_address(addr_str)
    except ValueError as e:
        print(e)
        sys.exit(1)
    
    # 构建maps文件路径
    maps_file = f"/proc/{pid}/maps"
    
    # 检查文件是否存在
    if not os.path.exists(maps_file):
        print(f"找不到进程 {pid} 或无权访问 /proc/{pid}/maps")
        sys.exit(1)
    
    # 搜索匹配的内存区域
    found = False
    try:
        with open(maps_file, 'r') as f:
            for line in f:
                parts = line.split()
                if not parts:
                    continue
                
                # 解析地址范围
                addr_range = parts[0]
                try:
                    start, end = addr_range.split('-')
                    start_addr = int(start, 16)
                    end_addr = int(end, 16)
                except ValueError:
                    continue
                
                # 检查地址是否在范围内
                if start_addr <= address < end_addr:
                    print(line.strip())
                    print(f"Offset: {hex(address - start_addr)}")
                    found = True
                    break
    except IOError as e:
        print(f"读取 {maps_file} 时出错: {e}")
        sys.exit(1)
    
    if not found:
        print(f"没有找到包含地址 {hex(address)} 的内存映射")
        sys.exit(1)

if __name__ == "__main__":
    main()