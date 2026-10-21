#!/usr/bin/env python3
"""Verify recovered PC-MODE inputs and byte-exact default reconstructions."""
import hashlib
import json
from pathlib import Path
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
        for source, original, folder in [('pctmp', 'inputs/pctmp', BASE), ('xvideo', 'xios', REFERENCE), ('oslabel', 'oslabel', REFERENCE)]:
            target = Path(temporary) / (source + '.bin')
            subprocess.run(['nasm', '-f', 'bin', source + '.asm', '-o', str(target)],
                           cwd=folder, check=True)
            if target.read_bytes() != (folder / (original + '.bin')).read_bytes():
                raise ValueError('default reconstruction differs: ' + source)
    print('PC-MODE inputs and three default reconstructions verified')


if __name__ == '__main__':
    verify()
