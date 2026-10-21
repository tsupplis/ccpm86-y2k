#!/usr/bin/env python3
"""Build PCTMP as its own RSP, preserving recovered DATA and checked patch sites."""
import argparse
from pathlib import Path
import subprocess

SOURCE = Path(__file__).resolve().parent.parent / 'kernel/rsp3'


def build(output):
    output.mkdir(parents=True, exist_ok=False)
    original_code = (SOURCE / 'inputs/pctmp.bin').read_bytes()
    subprocess.run(['nasm', '-f', 'bin', '-DPCMODE_SAFE_LOADER=1', '-DPCMODE_CHECKED_INPUT=1', '-DPCMODE_PIXEL_READ=1', '-DPCMODE_SHELL=1', '-DPCMODE_HELP=1', '-DPCMODE_SEARCH=1', str(SOURCE / 'pctmp.asm'),
         '-o', str(output / 'pctmp.bin')], cwd=SOURCE, check=True)
    safe_code = (output / 'pctmp.bin').read_bytes()
    # Only the documented header jump, paragraph-rounding constant, fixup
    # check, initial-stack check, checked relocation entry, record-offset
    # calculation and checked-input
    # call, returning INT 23h continuation and pixel-result call may differ
    # in original CODE.
    # The shell dispatch (0208h) and pre-parse help (01A8h) jumps are intentional; helpers append.
    patched_offsets = (set(range(0x0f8d, 0x0f91)) | {0x0fa5}
                       | set(range(0x105f, 0x1066)) | set(range(0x10a1, 0x10b3))
                       | set(range(0x111e, 0x1124)) | set(range(0x1140, 0x1146)) | set(range(0x1dab, 0x1db8)) | set(range(0x1e04, 0x1e07))
                       | set(range(0x4c1, 0x4c8)) | set(range(0x1a8, 0x1ab)) | set(range(0x208, 0x20b)) | set(range(0x2096, 0x2099)) | set(range(0x1705, 0x1708)))
    assert len(safe_code) >= len(original_code)
    assert all(safe_code[i] == byte for i, byte in enumerate(original_code)
               if i not in patched_offsets), 'unexpected change to recovered CODE'

    template = bytearray((SOURCE / 'inputs/pctmp.rsp').read_bytes())
    assert template[128:128 + len(original_code)] == original_code
    assert int.from_bytes(template[1:3], 'little') * 16 == len(original_code)
    assert len(safe_code) % 16 == 0
    # Discard old file-record padding before changing CODE length; otherwise
    # growth can retain an extra whole record beyond the declared groups.
    data_bytes = int.from_bytes(template[10:12], 'little') * 16
    del template[128 + len(original_code) + data_bytes:]
    template[1:3] = template[5:7] = (len(safe_code) // 16).to_bytes(2, 'little')
    template[128:128 + len(original_code)] = safe_code
    template.extend(b'\0' * (-len(template) % 128))  # CMD file record padding
    (output / 'pctmp.rsp').write_bytes(template)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('output', type=Path, help='fresh directory for PCTMP CODE/RSP')
    args = parser.parse_args()
    build(args.output.resolve())
