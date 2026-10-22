#!/usr/bin/env python3
"""Verify recovered PC-MODE inputs and byte-exact default reconstructions."""
import hashlib
import json
from pathlib import Path
import shutil
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parent.parent
BASE = ROOT / 'kernel/rsp3'
REFERENCE = ROOT / 'kernel/pcmode/tests/recovered'


def verify():
    manifest = json.loads((BASE / 'inputs/manifest.json').read_text())
    for name, record in manifest['inputs'].items():
        data = (ROOT / name).read_bytes()
        if len(data) != record['bytes'] or hashlib.sha256(data).hexdigest() != record['sha256']:
            raise ValueError('recovered input differs: ' + name)
    with tempfile.TemporaryDirectory(prefix='pcmode-verify-') as temporary:
        rsp_output = Path(temporary) / 'rsp3'
        rsp_output.mkdir()
        for path in BASE.iterdir():
            if path.suffix.lower() in ('.a86', '.lib') or path.name == 'Makefile':
                shutil.copy2(path, rsp_output / path.name)
        (rsp_output / 'pcdefs.lib').write_bytes(b'PCMODE_BUILD equ 0\r\n')
        subprocess.run(['make', 'all'], cwd=rsp_output, check=True,
                       stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        from pctmpcheck import check as check_pctmp
        check_pctmp(rsp_output / 'pctmp.rsp', original=True)
        for source, original, folder in [('xvideo', 'xios', REFERENCE), ('oslabel', 'oslabel', REFERENCE)]:
            target = Path(temporary) / (source + '.bin')
            subprocess.run(['nasm', '-f', 'bin', source + '.asm', '-o', str(target)],
                           cwd=folder, check=True)
            if target.read_bytes() != (folder / (original + '.bin')).read_bytes():
                raise ValueError('default reconstruction differs: ' + source)
    print('Native PCTMP RSP and historical XIOS/label reconstructions verified')


if __name__ == '__main__':
    verify()
