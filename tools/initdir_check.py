#!/usr/bin/env python3
"""Boot INITDIR in QEMU on disposable images; never modify user disks.

Run after make all. Exercises drive prompts, cancellation, timestamp creation
and removal, repeated use, file extents and bytes outside the directory.
"""
import json
from pathlib import Path
import select
import subprocess
import tempfile
import time

ROOT = Path(__file__).resolve().parent.parent
MEDIA = ROOT / 'media'


def run(*args):
    return subprocess.run(args, cwd=MEDIA, check=True, capture_output=True)


def twist(action, source, target):
    run('python3', str(ROOT / 'tools/cpm86twist.py'), action, str(source), str(target))


def check():
    with tempfile.TemporaryDirectory(prefix='initdir-boot-') as td:
        folder = Path(td)
        logical, boot, target = (folder / name for name in ('logical.img', 'boot.img', 'target.img'))
        video = folder / 'video.bin'
        twist('untwist', MEDIA / 'ccpm86.img', logical)
        run('cpmrm', '-f', 'ccpm86-320', str(logical), '0:help.hlp')
        run('cpmcp', '-f', 'ccpm86-320', str(logical), str(ROOT / 'commands/initdir/initdir.cmd'), '0:')
        twist('twist', logical, boot)
        twist('untwist', MEDIA / 'ccpmtools.img', logical)
        before = bytearray(logical.read_bytes())
        for pos in range(4096, 6144, 32):
            if before[pos] == 33:
                before[pos] = 0xe5
            elif before[pos] == 32:
                before[pos + 12] &= 0x81
        before = bytes(before)
        logical.write_bytes(before)
        twist('twist', logical, target)

        def extents(data):
            return sorted(data[i:i + 32] for i in range(4096, 6144, 32) if data[i] < 16)

        def disk(stamped=None):
            twist('untwist', target, logical)
            data = logical.read_bytes()
            assert data[:4096] == before[:4096] and data[6144:] == before[6144:], 'non-directory data changed'
            assert extents(data) == extents(before), 'file extents changed'
            if stamped is None:
                assert data == before, 'cancellation modified disk'
            elif stamped:
                assert all(data[i] == 33 for i in range(4192, 6144, 128)), 'missing stamp entries'
            else:
                assert all(data[i] != 33 for i in range(4096, 6144, 32)), 'stamp entries remain'

        args = ['qemu-system-i386', '-machine', 'pc', '-accel', 'tcg', '-m', '16',
                '-display', 'none', '-serial', 'none', '-monitor', 'none', '-qmp', 'stdio',
                '-net', 'none', '-boot', 'a', '-no-fd-bootchk', '-no-reboot']
        # Both images are temporary copies: writes must persist for validation.
        for index, path in enumerate((boot, target)):
            args += ['-drive', f'file={path},format=raw,if=floppy,index={index}']
        process = subprocess.Popen(args, stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                                   stderr=subprocess.DEVNULL, bufsize=0)

        def receive():
            assert select.select([process.stdout], [], [], 10)[0], 'QMP timeout'
            return json.loads(process.stdout.readline())

        def command(name, arguments=None):
            process.stdin.write((json.dumps({'execute': name, 'arguments': arguments or {}}) + '\n').encode())
            while True:
                reply = receive()
                if 'error' in reply:
                    raise RuntimeError(reply['error'])
                if 'return' in reply:
                    return reply['return']

        def hmp(text):
            return command('human-monitor-command', {'command-line': text})

        def screen():
            result = hmp(f'pmemsave 0xb8000 4000 "{video}"')
            assert not result, result
            data = video.read_bytes()
            return '\n'.join(''.join(chr(c) if 32 <= c < 127 else ' '
                                     for c in data[row * 160:row * 160 + 160:2])
                             for row in range(25))

        def wait(predicate):
            end = time.monotonic() + 30
            while time.monotonic() < end:
                text = screen()
                if predicate(text):
                    return text
                assert 'Interrupt from' not in text and 'Disk Error' not in text, text
                time.sleep(.2)
            raise AssertionError(screen())

        def prompt(text):
            lines = [line.strip() for line in text.splitlines()[:24] if line.strip()]
            return bool(lines) and lines[-1] == 'A>'

        def type_command(text):
            keys = {'\n': 'ret', ':': 'shift-semicolon', ' ': 'spc'}
            for char in text:
                hmp('sendkey ' + keys.get(char, char) + ' 50')
                time.sleep(.12)

        def invoke():
            type_command('initdir b:\n')
            wait(lambda t: 'INITDIR' in t.splitlines()[-1])
            type_command('y\n')

        try:
            receive()
            command('qmp_capabilities')
            wait(prompt)
            type_command('initdir\n')
            text = wait(lambda t: 'Enter Drive:' in t)
            assert 'ERROR:' in text and 'Unrecognized drive.' in text, text
            type_command('b\n')
            wait(lambda t: 'Do you want to re-format the directory on drive: B' in t)
            type_command('n\n')
            wait(prompt)
            disk()
            print('PASS: drive prompt and cancellation without writes')
            invoke()
            wait(prompt)
            disk(True)
            print('PASS: enable timestamps; file extents and non-directory bytes preserved')
            invoke()
            wait(lambda t: 'Do you want to recover time/date directory space' in t)
            type_command('y\n')
            wait(prompt)
            disk(False)
            print('PASS: remove timestamps; files preserved and locks released')
            invoke()
            wait(prompt)
            disk(True)
            print('PASS: enable again; repeated access and cleanup')
        finally:
            process.terminate()
            process.wait(timeout=5)


if __name__ == '__main__':
    check()
