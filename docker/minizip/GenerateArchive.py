#!/usr/bin/env python3
#
# Generate a synthetic, highly compressible input file for the minizip benchmark: a small random
# seed block repeated to the requested size. Repeating one seed keeps the data compressible and the
# compression time stable. Usage: GenerateArchive.py <output> <size>, e.g. GenerateArchive.py a.bin 64M

import os
import re
import sys

UNITS = {"": 1, "K": 1024, "M": 1024 ** 2, "G": 1024 ** 3}


def parse_size(text):
    m = re.fullmatch(r"\s*(\d+)\s*([kKmMgG]?)\s*", text)
    if not m:
        sys.exit(f"invalid size {text!r}; expected forms like 4096, 8K, 64M, 1G")
    return int(m.group(1)) * UNITS[m.group(2).upper()]


def main():
    if len(sys.argv) != 3:
        sys.exit("usage: GenerateArchive.py <output> <size>")
    output, total = sys.argv[1], parse_size(sys.argv[2])

    seed = os.urandom(8 * 1024)
    with open(output, "wb") as fp:
        written = 0
        while written < total:
            chunk = seed if total - written >= len(seed) else seed[: total - written]
            fp.write(chunk)
            written += len(chunk)
    print(f"wrote {output} ({total} bytes)")


if __name__ == "__main__":
    main()
