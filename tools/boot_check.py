#!/usr/bin/env python3
"""Boot both media images in an isolated QEMU VM and exercise the console."""
import json
from pathlib import Path
import re
import select
import shutil
import subprocess
import tempfile
import time

MEDIA = Path(__file__).resolve().parent.parent / 'media'


def check(image, second):
    qemu = shutil.which('qemu-system-i386')
    if not qemu:
        raise SystemExit('boot check requires qemu-system-i386')
    with tempfile.TemporaryDirectory(prefix='ccpm-boot-') as temporary:
        memory = Path(temporary) / 'video.bin'
        with (Path(temporary) / 'qemu.log').open('w') as log:
            args = [qemu, '-machine', 'pc', '-accel', 'tcg', '-m', '16',
                    '-display', 'none', '-serial', 'none', '-monitor', 'none',
                    '-qmp', 'stdio', '-net', 'none', '-boot', 'a',
                    '-no-fd-bootchk', '-no-reboot', '-snapshot',
                    '-rtc', 'base=2026-10-19T12:34:56,clock=vm']
            for index, disk in enumerate((image, second)):
                args += ['-drive', f'file={disk},format=raw,if=floppy,index={index}']
            process = subprocess.Popen(args, stdin=subprocess.PIPE,
                                       stdout=subprocess.PIPE, stderr=log, bufsize=0)
            def receive():
                if not select.select([process.stdout], [], [], 10)[0]:
                    raise RuntimeError('QEMU monitor timed out')
                line = process.stdout.readline()
                if not line:
                    raise RuntimeError('QEMU exited before completing the boot check')
                return json.loads(line)

            def command(name, arguments=None):
                request = {'execute': name, 'arguments': arguments or {}}
                process.stdin.write((json.dumps(request) + '\n').encode())
                while True:
                    reply = receive()
                    if 'error' in reply:
                        raise RuntimeError(reply['error'])
                    if 'return' in reply:
                        return reply['return']

            def hmp(text):
                return command('human-monitor-command', {'command-line': text})

            def screen():
                hmp(f'pmemsave 0xb8000 4000 "{memory}"')
                data = memory.read_bytes()
                text = '\n'.join(''.join(chr(c) if 32 <= c < 127 else ' '
                                        for c in data[row * 160:row * 160 + 160:2])
                                 for row in range(25))
                return text, data

            def wait_for(predicate, seconds=25):
                end = time.monotonic() + seconds
                while time.monotonic() < end:
                    text, data = screen()
                    if predicate(text):
                        return text, data
                    if 'Interrupt from' in text or 'Read Error' in text:
                        raise RuntimeError(text)
                    time.sleep(.25)
                raise RuntimeError('Boot/command timed out:\n' + screen()[0])

            def type_command(text):
                for char in text:
                    keys = {'\n': 'ret', ' ': 'spc', '.': 'dot',
                            '*': 'shift-8', '[': 'bracket_left', ']': 'bracket_right'}
                    hmp('sendkey ' + keys.get(char, char))
                    time.sleep(.08)

            try:
                receive()
                command('qmp_capabilities')
                text, data = wait_for(lambda t: re.search(r'^A>\s*$', t, re.M)
                                     and '10/19/26' in t and 'Console=0' in t)
                assert 0x0e in data[3841:4000:2], 'status line is not bright yellow'
                def shortcut(keys, console):
                    reply = hmp('sendkey ' + keys + ' 50')
                    assert not reply, reply
                    time.sleep(.12)  # release modifiers before the next chord
                    wait_for(lambda t: 'Console=' + str(console) + ' ' in t)

                for digit, console in ((4, 3), (2, 1), (3, 2), (1, 0)):
                    shortcut('ctrl-alt-' + str(digit), console)
                for console in (1, 2, 3, 0):
                    shortcut('ctrl-alt-tab', console)
                shortcut('ctrl-kp_1', 1)
                shortcut('ctrl-kp_0', 0)
                print(f'{image.name}: direct, round-robin and keypad console switching passed')
                type_command('dir\n')
                wait_for(lambda t: 'CCPM' in t and 'SYS' in t and 'PIP' in t)
                if image.name == 'ccpm86.img':
                    wait_for(lambda t: re.search(r'^A>\s*$', t, re.M))
                    for tail in ('sdir', 'sdir *.cmd', 'sdir [size]'):
                        type_command(tail + '\n')
                        text, _ = wait_for(lambda t: 'Files Found' in t
                                           and re.search(r'^A>\s*$', t, re.M))
                        assert 'Illegal command tail' not in text, text
                        assert 'SDIR' in text, text
                    print(f'{image.name}: SDIR empty tail, wildcard and options passed')
                    type_command('dredix\n')
                    type_command('hello\nworld')
                    wait_for(lambda t: t.startswith('hello')
                             and t.splitlines()[1].startswith('world'))
                    for key in ('up', 'home', 'right', 'delete', 'backspace'):
                        hmp('sendkey ' + key)
                        time.sleep(.15)
                    type_command('e')
                    text, _ = wait_for(lambda t: t.startswith('ello'))
                    shortcut('ctrl-alt-tab', 1)
                    shortcut('ctrl-alt-1', 0)
                    assert screen()[0].startswith('ello'), 'console switching changed editor text'

                    hmp('sendkey alt-x')
                    text, _ = wait_for(lambda t: 'OK to lose changes' in t)
                    assert text.startswith('ello'), 'quit prompt corrupted editor text'
                    type_command('y\n')
                    text, data = wait_for(lambda t: re.search(r'^A>\s*$', t, re.M))
                    assert 0x0e in data[3841:4000:2], 'editor changed status line color'
                    type_command('dir\n')
                    wait_for(lambda t: 'CCPM' in t and 'SYS' in t and 'DREDIX' in t)
                    print(f'{image.name}: DREDIX editing, navigation, delete/backspace and exit passed')

                print(f'{image.name}: PASS (two drives, A> prompt, RTC, yellow status, DIR)')
            finally:
                process.terminate()
                try:
                    process.wait(timeout=5)
                except subprocess.TimeoutExpired:
                    process.kill()
                    process.wait()


if __name__ == '__main__':
    system = (MEDIA.parent / 'kernel/sysgen/ccpm.sys').read_bytes()
    code_size = int.from_bytes(system[1:3], 'little') * 16
    data_size = int.from_bytes(system[10:12], 'little') * 16
    assert data_size <= 65536, 'CCPM.SYS data/RSP area exceeds 64K'
    data = system[128 + code_size:128 + code_size + data_size]
    flags = int.from_bytes(data[0x56:0x58], 'little')
    count = data[0x4a]
    assert data[flags:flags + count * 3] == b'\xff' * (count * 3), \
        'GENCCPM generated an invalid RTM flag table'
    check(MEDIA / 'ccpm86.img', MEDIA / 'ccpmtools.img')
    check(MEDIA / 'ccpmtools.img', MEDIA / 'ccpm86.img')
