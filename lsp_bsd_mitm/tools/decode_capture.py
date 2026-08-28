#!/usr/bin/env python3
"""Decode sdmc:/lsp_mitm/capture.bin from lsp_bsd_mitm."""

import struct
import sys

DIR = {0: "TX", 1: "RX"}
PROTO = {0: "tcp", 1: "udp"}


def main() -> None:
    path = sys.argv[1] if len(sys.argv) > 1 else "capture.bin"
    data = open(path, "rb").read()
    off = 0
    n = 0
    while off + 12 <= len(data):
        dir_b, proto_b, p_hi, p_lo, l0, l1, l2, l3 = data[off : off + 8]
        port = (p_hi << 8) | p_lo
        length = (l0 << 24) | (l1 << 16) | (l2 << 8) | l3
        off += 12
        if off + length > len(data):
            print(f"truncated at record {n}", file=sys.stderr)
            break
        payload = data[off : off + length]
        off += length
        head = payload[:32].hex()
        print(f"{n:5d} {DIR.get(dir_b, dir_b)} {PROTO.get(proto_b, proto_b)} port={port} len={length} head={head}")
        n += 1


if __name__ == "__main__":
    main()
