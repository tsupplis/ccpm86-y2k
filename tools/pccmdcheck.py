"""Native CMD relocation boundary fixtures; execute only on disposable disks."""
import argparse
from pathlib import Path
import shutil
import struct
import time

from pcmodebuild import ROOT, run


def fixture(build):
    work = build / 'cmd-fixtures'
    work.mkdir(exist_ok=True)
    run(['nasm', '-f', 'bin', str(ROOT / 'tests/cmdload.asm'),
         '-o', str(work / 'valid.cmd')], ROOT)
    original = (work / 'valid.cmd').read_bytes()
    # Include noncanonical byte offsets: the format permits all 0..255 values.
    cases = {'edge': (7, 14), 'wide': (0, 126), 'cross': (7, 15),
             'past': (8, 0), 'wrap': (65535, 255)}
    logical = work / 'logical.img'
    shutil.copy2(build / 'search-fixtures/b-logical.img', logical)
    for name, (paragraph, offset) in cases.items():
        data = bytearray(original)
        struct.pack_into('<HB', data, 769, paragraph, offset)
        path = work / (name + '.cmd')
        path.write_bytes(data)
        run(['cpmcp', '-f', 'ccpm86-320', str(logical), str(path),
             '0:' + path.name], ROOT / 'media')
    run(['python3', str(ROOT / 'tools/cpm86twist.py'), 'twist', str(logical),
         str(build / 'pc-cmdb.img')], ROOT)


def checks(wait, type_command, prompt, output, baseline=False):
    results = []
    # Baseline only exercises a write within the fixture's combined allocation.
    cases = ('edge', 'wide', 'past') if baseline else ('edge', 'wide', 'cross', 'past', 'wrap')
    for name in cases:
        type_command('cls\n')
        time.sleep(.4)
        wait(prompt)
        type_command('b:' + name + '.cmd\n')
        time.sleep(.5)
        text = wait(prompt)
        (output / (name + '.txt')).write_text(text)
        if baseline or name in ('edge', 'wide'):
            assert 'CMD FIXTURE EXECUTED' in text, text
        else:
            assert 'Invalid CMD relocation data' in text, text
            assert 'CMD FIXTURE EXECUTED' not in text, text
        results.append('PASS: ' + name + (' baseline accepted' if baseline else ' relocation boundary'))
    type_command('b:edge.cmd\n')
    time.sleep(.5)
    assert 'CMD FIXTURE EXECUTED' in wait(prompt)
    results.append('PASS: valid CMD executes after boundary cases')
    return results


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('build', type=Path)
    fixture(parser.parse_args().build.resolve())
