"""Snapshot checks for startup files and real SUBMIT drive/argument handling."""
import argparse
from pathlib import Path
import shutil
import subprocess
import time
from concurrent.futures import ThreadPoolExecutor

from cpm86twist import convert
from pcmodebuild import ROOT, run

CASES = {
    'compact': b'b:\r\n\x1a',
    'spaced': b'b :\r\n\x1a',
    'whitespace': b'  b :\r\n\x1a',
    'unavailable': b'p:\r\n\x1a',
    'comment': b'; ignored\r\n\x1a',
    'empty': b'\r\n\x1a',
    'no-cr': b'b:' + b'\x1a' * 126,
    'native': b'b:native.cmd\r\n\x1a',
    'submit': b'b:onlysub.sub red blue\r\n\x1a',
    'help': b'?\r\n\x1a',
    'console1': b'b:\r\n\x1a',
}


def fixtures(build):
    build = build.resolve()
    work = build / 'startup-fixtures'
    work.mkdir(exist_ok=True)
    for personality, boot in [('pc', 'pc-search.img'), ('native', 'pc-native.img')]:
        original = convert((build / boot).read_bytes(), to_logical=True)
        missing = work / f'{personality}-no-submit-logical.img'
        missing.write_bytes(original)
        run(['cpmrm', '-f', 'ccpm86-320', str(missing), '0:submit.cmd'], ROOT / 'media')
        (work / f'{personality}-no-submit.img').write_bytes(convert(missing.read_bytes(), to_logical=False))
        for case, text in CASES.items():
            source = work / 'startup.txt'
            source.write_bytes(text)
            logical = work / f'{personality}-{case}-logical.img'
            logical.write_bytes(original)
            run(['cpmcp', '-f', 'ccpm86-320', str(logical), str(source),
                 '0:startup.' + ('1' if case == 'console1' else '0')], ROOT / 'media')
            (work / f'{personality}-{case}.img').write_bytes(convert(logical.read_bytes(), to_logical=False))
    logical = work / 'submit-logical.img'
    shutil.copy2(build / 'search-fixtures/b-logical.img', logical)
    script = work / 'drives.sub'
    script.write_bytes(b'; drive and argument fixture\r\n\r\nB:\r\nB:NATIVE.CMD\r\nA:\r\nB:NATIVE.CMD\r\nECHO SUB ARGS $1 $2\r\n\x1a')
    run(['cpmcp', '-f', 'ccpm86-320', str(logical), str(script), '0:drives.sub'], ROOT / 'media')
    (build / 'pc-startb.img').write_bytes(convert(logical.read_bytes(), to_logical=False))


def startup_checks(hmp, wait, type_command, output, case):
    def prompt(text, drive='A', console=0):
        rows = [row.strip() for row in text.splitlines()[:24] if row.strip()]
        return bool(rows) and rows[-1] == drive + '>' and f'Console={console}' in text
    console = int(case == 'console1')
    if console:
        wait(prompt)
        hmp('sendkey ctrl-alt-2 100')
    drive = 'B' if case in ('compact', 'spaced', 'whitespace', 'console1') else 'A'
    text = wait(lambda text: prompt(text, drive, console))
    (output / 'startup.txt').write_text(text)
    marker = {'unavailable': 'Drive unavailable: P:', 'native': 'SEARCH CMD B DEFAULT A',
              'submit': 'SEARCH SUB B RED BLUE', 'help': 'Common commands:'}.get(case)
    if marker:
        assert marker in text, text
    else:
        assert 'Command error:' not in text and 'CP/M Error' not in text, text
    results = ['PASS: STARTUP ' + case]
    if drive == 'B':
        type_command('a:\n')
        time.sleep(.4)
        wait(lambda text: prompt(text, 'A', console))
        results.append('PASS: interactive drive recovery after startup')
    return results


def submit_checks(wait, type_command, prompt, output):
    type_command('cls\n')
    wait(prompt)
    type_command('b:drives.sub red blue\n')
    text = wait(lambda text: prompt(text) and 'SUB ARGS RED BLUE' in text)
    (output / 'submit.txt').write_text(text)
    assert 'SEARCH CMD B DEFAULT B' in text, text
    assert 'SEARCH CMD B DEFAULT A' in text, text
    assert 'Command error:' not in text and 'CP/M Error' not in text, text
    return ['PASS: real SUBMIT changes B:/A: and expands arguments, blank/comment lines']


def submit_failure_checks(wait, type_command, prompt, output, case):
    command = 'b:onlysub.sub' + (' ' + 'x' * 114 if case == 'tail' else '')
    type_command(command + '\n')
    time.sleep(.5)
    text = wait(prompt)
    (output / 'submit-failure.txt').write_text(text)
    message = 'Command tail too long for SUBMIT' if case == 'tail' else 'SUBMIT.CMD not found'
    assert 'Command error: ' + message in text, text
    assert 'SEARCH SUB' not in text, text
    type_command('b:native.cmd\n')
    time.sleep(.5)
    text = wait(prompt)
    (output / 'submit-failure-recovery.txt').write_text(text)
    assert 'SEARCH CMD B DEFAULT A' in text, text
    return ['PASS: real SUBMIT ' + case + ' failure and command recovery']


def check_all(build, failures_only=False):
    build = build.resolve()
    fixtures(build)
    output = build / 'tests/startup'
    jobs = []
    for personality, boot in [('pc', 'pc-search.img'), ('native', 'pc-native.img')]:
        if not failures_only:
            for case in CASES:
                jobs.append((personality + '-' + case,
                             build / f'startup-fixtures/{personality}-{case}.img',
                             ['--startup-case', case]))
            jobs.append((personality + '-submit-script', build / boot, ['--submit-checks']))
        jobs.append((personality + '-submit-missing', build / f'startup-fixtures/{personality}-no-submit.img',
                     ['--submit-failure', 'missing']))
        jobs.append((personality + '-submit-tail', build / boot, ['--submit-failure', 'tail']))
    def run_case(job):
        name, image, flags = job
        target = output / name
        target.mkdir(parents=True, exist_ok=True)
        with (target / 'run.log').open('wb') as log:
            result = subprocess.run(['python3', str(ROOT / 'tools/pcmodetest.py'),
                                     '--image', str(image), '--tools-image', str(build / 'pc-startb.img'),
                                     '--output', str(target)] + flags, stdout=log, stderr=subprocess.STDOUT)
        return name, result.returncode
    with ThreadPoolExecutor(max_workers=4) as pool:
        results = list(pool.map(run_case, jobs))
    (output / ('failure-results.txt' if failures_only else 'results.txt')).write_text(''.join(('PASS: ' if code == 0 else 'FAIL: ') + name + '\n'
                                             for name, code in results))
    for name, code in results:
        print(('PASS: ' if code == 0 else 'FAIL: ') + name)
    if any(code for _, code in results):
        raise AssertionError('inspect tests/startup/*/run.log')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('build', type=Path)
    parser.add_argument('--check', action='store_true')
    parser.add_argument('--check-failures', action='store_true')
    args = parser.parse_args()
    if args.check or args.check_failures:
        check_all(args.build, args.check_failures)
    else:
        fixtures(args.build)
