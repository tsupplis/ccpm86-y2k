#!/usr/bin/env python3
"""Build a disposable native-CMD fixture for the tested PCTMP error helper."""
import argparse
from pathlib import Path
import shutil

from pcmodebuild import ROOT, run


def fixture(build):
    build = build.resolve()
    work = build / 'error-fixture'
    work.mkdir(exist_ok=True)
    shutil.copy2(ROOT / 'kernel/pcmode/tests/probes/errdiag.a86', work)
    shutil.copy2(build / 'rsp3/pcerror.lib', work)
    log = run(['cpm86_asm86', 'errdiag.a86'], work, work / 'asm.log')
    if b'NUMBER OF ERRORS:   0' not in log:
        raise RuntimeError('inspect error-fixture/asm.log')
    run(['cpm86_gencmd', 'errdiag.h86'], work, work / 'gencmd.log')
    logical = work / 'logical.img'
    shutil.copy2(build / 'search-fixtures/b-logical.img', logical)
    run(['cpmcp', '-f', 'ccpm86-320', str(logical), str(work / 'errdiag.cmd'),
         '0:errdiag.cmd'], ROOT / 'media')
    run(['python3', str(ROOT / 'tools/cpm86twist.py'), 'twist', str(logical),
         str(build / 'pc-errorb.img')], ROOT)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('build', type=Path)
    fixture(parser.parse_args().build)
