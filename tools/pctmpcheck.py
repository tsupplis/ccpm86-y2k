#!/usr/bin/env python3
"""Check native-built PCTMP groups and patch boundaries; never package an RSP."""
import argparse
from pathlib import Path

SOURCE = Path(__file__).resolve().parent.parent / 'kernel/rsp3/inputs'


def check(path, original=False):
    image = path.read_bytes()
    reference = (SOURCE / 'pctmp.rsp').read_bytes()
    if original:
        if image != reference:
            raise ValueError('Original native RSP differs from the recovered template')
        return
    code_length = int.from_bytes(image[1:3], 'little') * 16
    data_length = int.from_bytes(image[10:12], 'little') * 16
    header = bytearray(reference[:128])
    header[1:3] = header[5:7] = (code_length // 16).to_bytes(2, 'little')
    if image[:128] != header or data_length != 2800:
        raise ValueError('Unexpected native RSP header or DATA size')
    original_code = (SOURCE / 'pctmp.bin').read_bytes()
    code = image[128:128 + code_length]
    data = image[128 + code_length:128 + code_length + data_length]
    if code_length < len(original_code) or data != reference[128 + len(original_code):128 + len(original_code) + data_length]:
        raise ValueError('Native RSP changed recovered DATA or truncated CODE')
    allowed = (set(range(0x7dd, 0x7e0)) | set(range(0x171, 0x178)) | set(range(0x0f8d, 0x0f91)) | {0x0fa5}
               | set(range(0x105f, 0x1066)) | set(range(0x10a1, 0x10b3))
               | set(range(0x111e, 0x1124)) | set(range(0x1140, 0x1146))
               | set(range(0x1dab, 0x1db8)) | set(range(0x1e04, 0x1e07))
               | set(range(0x4c1, 0x4c8)) | set(range(0x1a8, 0x1ab))
               | set(range(0x208, 0x20b)) | set(range(0x2096, 0x2099))
               | set(range(0x1705, 0x1708)))
    changed = {i for i, byte in enumerate(original_code) if code[i] != byte}
    if changed - allowed:
        raise ValueError('Native RSP changed an undocumented original CODE location')
    extent = 128 + code_length + data_length
    if len(image) != (extent + 127) // 128 * 128 or any(image[extent:]):
        raise ValueError('Unexpected native RSP record padding')
    print(f'PCTMP: CODE {code_length}, DATA {data_length}, {len(changed)} changed original bytes')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('rsp', type=Path)
    parser.add_argument('--original', action='store_true')
    args = parser.parse_args()
    check(args.rsp, args.original)
