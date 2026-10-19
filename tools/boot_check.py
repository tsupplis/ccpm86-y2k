#!/usr/bin/env python3
"""Boot the three system images in an isolated QEMU VM and exercise the console."""
import json
from pathlib import Path
import re
import select
import shutil
import subprocess
import tempfile
import time

MEDIA = Path(__file__).resolve().parent.parent / 'media'


def check_recall(type_command, hmp, wait_for, shortcut):
    """Exercise single-line recall through the real keyboard, CIO and TMP."""
    def key(name):
        reply = hmp('sendkey ' + name + ' 50')
        assert not reply, reply
        time.sleep(.12)

    def current(expected):
        # The current input is the last nonblank line above the status row.
        def matches(text):
            lines = [line.rstrip() for line in text.splitlines()[:24] if line.strip()]
            return (''.join(lines).endswith('A>' + expected)
                    or bool(expected) and bool(lines) and lines[-1].lstrip() == expected)
        return wait_for(matches)

    # No saved line yet.  Comment commands avoid launching a program.
    key('ctrl-w')
    current('')
    type_command(';recallone\n')
    current('')
    key('ctrl-w')
    current(';recallone')
    key('ctrl-w')
    current(';recallone')
    type_command('x')
    key('ctrl-w')
    current(';recallonex')
    key('backspace')
    type_command('y\n')
    current('')

    # A new accepted line replaces the only saved line.  Blank submission
    # and abandoning an edit with ^U retain it.
    type_command('\n')
    key('ctrl-w')
    current(';recalloney')
    key('ctrl-u')
    key('ctrl-w')
    current(';recalloney')
    key('ctrl-x')
    type_command('\n')
    current('')

    shortcut('ctrl-alt-2', 1)
    key('ctrl-w')
    current('')
    type_command(';otherconsole\n')
    current('')
    key('ctrl-w')
    current(';otherconsole')
    key('ctrl-x')
    shortcut('ctrl-alt-1', 0)
    key('ctrl-w')
    current(';recalloney')
    key('ctrl-x')
    type_command('\n')
    current('')
    # TMP's 128-byte limit auto-accepts typed input.  Recall of a full
    # buffer must instead wait for Return and reject additional bytes.
    full = ';' + 'a' * 127
    type_command(full)
    current('')
    key('ctrl-w')
    current(full)
    type_command('z')
    current(full)
    key('backspace')
    key('backspace')
    type_command('x')
    current(full[:-2] + 'x')
    type_command('\n')
    current('')
    key('ctrl-w')
    current(full[:-2] + 'x')
    key('ctrl-x')
    type_command('\n')
    current('')
    print('single-line recall: replacement, editing, blank/^U, console isolation and full-buffer recall passed')



def check_editor(type_command, hmp, wait_for, shortcut):
    """Edit real Function 10 input using control keys and native navigation."""
    def key(name):
        assert not hmp('sendkey ' + name + ' 50')
        time.sleep(.12)

    def current(expected):
        expanded = ('A>' + expected).expandtabs(8)
        def matches(text):
            lines = [line.rstrip() for line in text.splitlines()[:24] if line.strip()]
            return ''.join(lines).endswith(expanded)
        return wait_for(matches)

    type_command(';abcd')
    key('ctrl-a')
    key('ctrl-a')
    type_command('x')
    current(';abxcd')
    key('ctrl-f')
    key('ctrl-g')
    current(';abxc')
    key('ctrl-b')
    key('ctrl-f')
    type_command('z')
    current(';zabxc')
    key('ctrl-b')
    key('ctrl-b')
    type_command('e\n')
    current('')
    key('up')
    current(';zabxce')
    key('home')
    key('up')                   # Up is exactly ^W: nonempty means End.
    type_command('f')
    current(';zabxcef')
    key('home')
    key('right')
    type_command('q')
    current(';qzabxcef')
    key('end')
    key('left')
    key('delete')
    current(';qzabxce')
    key('backspace')
    current(';qzabxc')
    key('home')
    key('right')
    key('ctrl-k')
    type_command('fresh')
    current(';fresh')
    key('home')
    key('ctrl-x')               # Nothing to delete to the left of Home.
    current(';fresh')
    for _ in range(3):
        key('right')
    key('ctrl-x')
    # Prefix removal must retain the suffix in the actual returned buffer.
    key('home')
    type_command(';')
    current(';esh')
    key('end')
    key('ctrl-x')
    type_command('\n')
    current('')

    type_command(';a\tbc')
    key('home')
    key('right')
    key('right')
    type_command('z')
    current(';az\tbc')
    key('right')               # Cross the tab as one buffer character.
    key('backspace')
    current(';azbc')
    key('end')
    key('ctrl-x')
    type_command('\n')
    current('')

    # Retyping and physical newlines must preserve the logical cursor.
    type_command(';abc')
    key('left')
    key('ctrl-r')
    type_command('x\n')
    current('')
    key('up')
    current(';abxc')
    key('left')
    key('ctrl-e')
    type_command('y\n')
    current('')
    key('up')
    current(';abxyc')
    key('end')
    key('ctrl-x')
    type_command('\n')
    current('')

    type_command(';a')
    key('ctrl-d')               # Literal ^D displays in two screen cells.
    type_command('b')
    key('home')
    for _ in range(3):
        key('right')
    key('left')
    key('delete')
    current(';ab')
    key('end')
    key('ctrl-x')
    type_command('\n')
    current('')

    wrapped = ';' + 'a' * 77 + 'bcd'
    type_command(wrapped)
    key('home')
    key('end')
    for _ in range(3):
        key('left')
    type_command('x')
    current(wrapped[:-3] + 'xbcd')
    # The cursor remains inside the line across a console switch.
    shortcut('ctrl-alt-2', 1)
    type_command(';side')
    key('home')
    key('end')
    key('ctrl-x')
    type_command('\n')
    shortcut('ctrl-alt-1', 0)
    type_command('y')
    current(wrapped[:-3] + 'xybcd')
    type_command('\n')          # Return from the middle accepts the suffix.
    current('')
    key('up')
    current(wrapped[:-3] + 'xybcd')
    key('end')
    key('ctrl-x')
    type_command('\n')
    current('')
    print('line editor: insertion/deletion, controls, arrows/Home/End, Up=^W, tabs, control widths, retype, wrapping and console switching passed')



def check_shell(type_command, hmp, wait_for):
    """Check TMP help aliases and diagnostics through actual console input."""
    def prompt(text):
        lines = [line.rstrip() for line in text.splitlines()[:24] if line.strip()]
        return bool(lines) and lines[-1] == 'A>'

    type_command('?\n')
    wait_for(lambda t: 'Common commands:' in t and '? topic opens HELP' in t and prompt(t))
    type_command('? pip [nopage]\n')
    wait_for(lambda t: 'HELP>' in t and 'PIP' in t)
    type_command('\n')
    wait_for(prompt)
    type_command('? pip [nopage]'.ljust(125) + '\n')
    wait_for(lambda t: 'HELP>' in t and 'PIP' in t)
    type_command('\n')
    wait_for(prompt)
    for tail, expected in (
            ('zzqx987', 'Command not found. Use ? for help'),
            ('toolongname', 'Invalid command name'),
            ('dir.toolong', 'Invalid command extension'),
            ('z:dir', 'Invalid drive (use A: through P:)'),
            ('c:dir', 'Drive unavailable: C:'),
            ('c:', 'Drive unavailable: C:'),
            ('? ' + 'a' * 124, 'Help request too long')):
        type_command('cls\n')
        wait_for(lambda t: 'Command error:' not in t and prompt(t))
        type_command(tail + '\n')
        wait_for(lambda t: expected in t and prompt(t))
    type_command('b:\n')
    wait_for(lambda t: any(line.rstrip() == 'B>' for line in t.splitlines()[:24]))
    type_command('a:\n')
    wait_for(prompt)
    # A failed drive change must not redirect the next command to that drive.
    type_command('dir\n')
    wait_for(lambda t: 'CCPM' in t and 'SYS' in t and prompt(t))
    print('shell: ? quick help/topic alias, missing commands, syntax, unavailable drives and alias bounds passed')


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
                    keys = {'\n': 'ret', ' ': 'spc', '.': 'dot', ':': 'shift-semicolon',
                            '*': 'shift-8', ';': 'semicolon', '?': 'shift-slash', '\t': 'tab', '[': 'bracket_left', ']': 'bracket_right'}
                    hmp('sendkey ' + keys.get(char, char) + ' 50')
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
                if image.name == 'ccpm86.img':
                    check_recall(type_command, hmp, wait_for, shortcut)
                    check_editor(type_command, hmp, wait_for, shortcut)
                    check_shell(type_command, hmp, wait_for)
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
                    type_command('cls\n')
                    text, _ = wait_for(lambda t: re.search(r'^A>\s*$', t, re.M) and 'Copyright' not in t and 'Terminal' in t)
                    assert 'Terminal' in text, text
                    type_command('sysdisk\n')
                    wait_for(lambda t: 'System disk is A:' in t and re.search(r'^A>\s*$', t, re.M))
                    type_command('sysdisk b:\n')
                    wait_for(lambda t: 'System disk is B:' in t and re.search(r'^A>\s*$', t, re.M))
                    type_command('sysdisk a:\n')
                    wait_for(lambda t: 'System disk is A:' in t and re.search(r'^A>\s*$', t, re.M))
                    type_command('sysdisk nonsense\n')
                    wait_for(lambda t: 'Invalid drive code' in t and re.search(r'^A>\s*$', t, re.M))
                    for tail, expected in (('sysdisk c:', 'Specified device does not exist'),
                                           ('sysdisk a: extra', 'Invalid drive code')):
                        type_command('cls\n')
                        wait_for(lambda t: re.search(r'^A>\s*$', t, re.M) and 'System disk is' not in t)
                        type_command(tail + '\n')
                        wait_for(lambda t: expected in t and re.search(r'^A>\s*$', t, re.M))
                    shortcut('ctrl-alt-2', 1)
                    type_command('cls\n')
                    type_command('sysdisk\n')
                    text, _ = wait_for(lambda t: 'System disk is A:' in t and re.search(r'^A>\s*$', t, re.M))
                    assert 'Terminal' in text, text
                    shortcut('ctrl-alt-1', 0)
                    print(f'{image.name}: TMP built-in CLS and SYSDISK query, set and invalid syntax passed')

                    for topic, expected in (('sysdisk examples', 'System disk is M:'),
                                            ('wmenu', 'Ctrl-Alt-2')):
                        type_command('help ' + topic + ' [nopage]\n')
                        wait_for(lambda t: 'HELP>' in t and expected in t)
                        type_command('\n')
                        wait_for(lambda t: re.search(r'^A>\s*$', t, re.M))
                    print(f'{image.name}: restored HELP topics, including text beyond 64 KB, passed')

                    type_command('b:dredix\n')
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
                    wait_for(lambda t: 'CCPM' in t and 'SYS' in t and 'SYSDISK' in t)
                    print(f'{image.name}: DREDIX editing, navigation, delete/backspace and exit passed')

                if image.name in ('ccpmtools.img', 'ccpmmaint.img'):
                    type_command('? \t\n')
                    wait_for(lambda t: 'Common commands:' in t
                             and '? topic opens HELP' in t
                             and re.search(r'^A>\s*$', t, re.M))
                    print(f'{image.name}: built-in quick help without HELP.CMD passed')
                if image.name == 'ccpmtools.img':
                    # This disk has no SYSDISK.CMD: exercise the TMP built-in.
                    type_command('sysdisk\n')
                    wait_for(lambda t: 'System disk is A:' in t and re.search(r'^A>\s*$', t, re.M))
                    type_command('window view\n')
                    wait_for(lambda t: 'FGColor' in t and 'BGColor' in t
                             and re.search(r'^A>\s*$', t, re.M))
                    type_command('wmenu\n')
                    wait_for(lambda t: 'Window Manager installed.' in t
                             and re.search(r'^A>\s*$', t, re.M))
                    print(f'{image.name}: WINDOW VIEW and WMENU installation passed')
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
    check(MEDIA / 'ccpmmaint.img', MEDIA / 'ccpm86.img')
