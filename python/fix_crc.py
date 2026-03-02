#!/usr/bin/env python3
"""Compute/fix CRC16-CCITT (0xFFFF/0x1021) in binary files."""

import sys, argparse


def crc16(data, crc=0xFFFF):
    for b in data:
        crc ^= b << 8
        for _ in range(8):
            crc = ((crc << 1) ^ 0x1021) & 0xFFFF if crc & 0x8000 else (crc << 1) & 0xFFFF
    return crc


def parse_int(s):
    return int(s, 0)


def parse_end(s):
    if s.startswith("+"):
        return ("len", int(s[1:], 0))
    return ("abs", int(s, 0))


def main():
    p = argparse.ArgumentParser(description="Fix CRC16-CCITT in binary files")
    p.add_argument("infile")
    p.add_argument("start", nargs="?", type=parse_int, default=0)
    p.add_argument("end", nargs="?", type=parse_end, default=None,
                   help="end offset or +length (default: EOF)")
    p.add_argument("--pad", type=parse_int, help="extend file with 0xFF to N bytes")
    p.add_argument("--check", action="store_true", help="verify without modifying")
    p.add_argument("-o", "--output", help="output file (default: in-place)")
    args = p.parse_args()

    with open(args.infile, "rb") as f:
        data = bytearray(f.read())

    if args.pad and args.pad > len(data):
        data += b'\xFF' * (args.pad - len(data))

    start = args.start
    end = len(data) if args.end is None else (start + args.end[1] if args.end[0] == "len" else args.end[1])

    if start >= end or end - start < 4:
        p.error(f"bad range: 0x{start:X}..0x{end:X}")
    if end > len(data):
        p.error(f"0x{end:X} past EOF (0x{len(data):X})")

    crc_off = start + (end - start) - 2
    old = (data[crc_off] << 8) | data[crc_off + 1]

    if args.check:
        ok = crc16(data[start:end]) == 0
        print(f"{'OK' if ok else 'BAD'}  0x{old:04X}")
        sys.exit(0 if ok else 1)

    new = crc16(bytes(data[start:crc_off]))
    data[crc_off] = (new >> 8) & 0xFF
    data[crc_off + 1] = new & 0xFF
    assert crc16(data[start:end]) == 0

    outfile = args.output or args.infile
    with open(outfile, "wb") as f:
        f.write(data)

    if old == new:
        print(f"0x{new:04X} (unchanged)")
    else:
        print(f"0x{old:04X} -> 0x{new:04X}")


if __name__ == "__main__":
    main()
