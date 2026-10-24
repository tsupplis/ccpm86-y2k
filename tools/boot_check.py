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


def search_fixtures(temporary, image, second):
    """Put distinguishable commands on disposable disks to test resolution."""
    shutil.copyfile(MEDIA / "diskdefs", Path(temporary) / "diskdefs")
    def run(*args):
        subprocess.run(args, check=True, cwd=temporary, stdout=subprocess.DEVNULL)
    for letter in ('A', 'B'):
        source = Path(temporary) / ('probe' + letter.lower() + '.a86')
        source.write_text("""cseg
org 0
    mov cl,9 ! mov dx,offset message ! int 0e0h
    mov cl,25 ! int 0e0h
    mov dl,al ! add dl,'A' ! mov cl,2 ! int 0e0h
    mov cl,9 ! mov dx,offset ending ! int 0e0h
    mov cl,0 ! int 0e0h
 dseg
org 100h
message db 'PATH %s DEFAULT $'
ending db 13,10,'$'
end
""" % letter)
        run('cpm86_asm86', source.name)
        run('cpm86_gencmd', source.with_suffix('.h86').name)
    script = Path(temporary) / 'search.sub'
    script.write_bytes(b'SDPTEST\r\nSDONLY\r\n\x1a')
    profile = Path(temporary) / 'profile.sub'
    profile.write_bytes(b'A:SETDEF B:\r\nSDPTEST\r\nA:SETDEF C:,B:\r\n\x1a')
    disks = []
    for letter, original in zip(('A', 'B'), (image, second)):
        logical = Path(temporary) / (letter + '.logical')
        disk = Path(temporary) / (letter + '.img')
        run('python3', str(MEDIA.parent / 'tools/cpm86twist.py'),
            'untwist', str(original), str(logical))
        # Additional SUB fixtures use space only in these disposable copies.
        run('cpmrm', '-f', 'ccpm86-320', str(logical),
            '0:ED.CMD' if letter == 'A' else '0:ASM86.CMD')
        if letter == 'A':
            run('cpmrm', '-f', 'ccpm86-320', str(logical), '0:CONFIG.CMD')
        probe = str(Path(temporary) / ('probe' + letter.lower() + '.cmd'))
        for name in ('SDPTEST', 'SYSFALL') + (('SDONLY',) if letter == 'B' else ()):
            run('cpmcp', '-f', 'ccpm86-320', str(logical), probe, '0:' + name + '.CMD')
        scripts = {'ONLYSUB': 'ECHO SCRIPT ' + letter + ' $1 $2'}
        if letter == 'A':
            scripts['SYSBATCH'] = 'ECHO SYSTEM SCRIPT A'
            scripts['USERTEST'] = (';user-zero record padding\r\n' * 6 + 'ECHO USER ZERO A')
            run('cpmcp', '-f', 'ccpm86-320', str(logical), probe, '0:EARLY.CMD')
            run('cpmchattr', '-f', 'ccpm86-320', str(logical), 's', '0:SUBMIT.CMD')
        else:
            scripts.update({'SDPTEST': 'ECHO WRONG SUB SHADOW',
                            'EARLY': 'ECHO EARLY SUB B',
                            'INNER': 'ECHO INNER $1',
                            'NEST': 'ECHO NEST START\r\nINNER ARG\r\nECHO NEST END',
                            'INCLUDE': '$INCLUDE B:INNER ARG\r\nECHO INCLUDE END',
                            'BADCMD': 'ECHO WRONG BAD CMD FALLBACK',
                            'SUBMIT': 'ECHO WRONG INTERPRETER RECURSION'})
        if letter == 'B':
            empty = Path(temporary) / 'empty.cmd'
            empty.write_bytes(b'')
            run('cpmcp', '-f', 'ccpm86-320', str(logical), str(empty), '0:BADCMD.CMD')
        for name, content in scripts.items():
            source = Path(temporary) / (name + '.sub')
            source.write_bytes(content.encode() + b'\r\n\x1a')
            run('cpmcp', '-f', 'ccpm86-320', str(logical), str(source), '0:' + name + '.SUB')
        if letter == 'A':
            run('cpmchattr', '-f', 'ccpm86-320', str(logical), 's',
                '0:SYSBATCH.SUB', '0:USERTEST.SUB')
            source = Path(temporary) / 'userone.sub'
            source.write_bytes(b'ECHO WRONG USER ONE\r\n\x1a')
            run('cpmcp', '-f', 'ccpm86-320', str(logical), str(source), '1:USERTEST.SUB')
        run('cpmchattr', '-f', 'ccpm86-320', str(logical), 's', '0:SYSFALL.CMD')
        if letter == 'A':
            run('cpmcp', '-f', 'ccpm86-320', str(logical), str(script), '0:SEARCH.SUB')
            run('cpmcp', '-f', 'ccpm86-320', str(logical), str(profile), '0:PROFILE.SUB')
        run('python3', str(MEDIA.parent / 'tools/cpm86twist.py'),
            'twist', str(logical), str(disk))
        disks.append(disk)
    return disks


def search_display_matches(text, expected):
    """Compare all displayed search entries and exact order, ignoring padding."""
    if isinstance(expected, tuple):
        if 'Drive Search Path:' not in text:
            return False
        text = text.rsplit('Drive Search Path:', 1)[1]
        field, value = expected
        if field == 'path':
            rows = re.findall(r'^[1-4](?:st|nd|rd|th) Drive +-[ ]+(Default|[A-P]:)[ ]*$',
                              text, re.M)
            fixed = re.findall(r'^([2-5])(?:nd|rd|th) Drive +-[ ]+Sys Disk \([A-P]:\)[ ]*$', text, re.M)
            return (','.join('*' if row == 'Default' else row for row in rows) == value
                    and fixed == [str(len(rows) + 1)])
        order = re.search(r'^Search Order +-[ ]+([A-Z, ]+?)[ ]*$', text, re.M)
        return bool(order) and order.group(1).replace(' ', '') == value
    return expected in text


def check_setdef(type_command, wait_for, shortcut):
    def prompt(t, drive='A'):
        lines = [line.rstrip() for line in t.splitlines()[:24] if line.strip()]
        return bool(lines) and lines[-1] == drive + '>'

    def command(tail, expected, drive='A'):
        type_command('cls\n')
        wait_for(lambda t: not search_display_matches(t, expected) and prompt(t, drive))
        type_command(tail + '\n')
        text, _ = wait_for(lambda t: search_display_matches(t, expected) and prompt(t, drive))
        assert 'BDOS Err' not in text, text
        return text

    command('setdef', ('path', '*'))
    command('setdef b:,a:', ('path', 'B:,A:'))
    command('sdptest', 'PATH B DEFAULT A')
    command('a:sdptest', 'PATH A DEFAULT A')
    command('a:setdef a:,b:', ('path', 'A:,B:'))
    command('sdptest', 'PATH A DEFAULT A')
    command('a:setdef c:,b:', ('path', 'C:,B:'))
    command('sdonly', 'PATH B DEFAULT A')
    command('sysfall', 'PATH B DEFAULT A')
    command('a:submit a:search', 'PATH B DEFAULT A')
    command('a:submit a:profile', ('path', 'C:,B:'))
    command('a:setdef', ('path', 'C:,B:'))
    command('a:setdef b:,b:,*,a:', ('path', 'B:,B:,*,A:'))
    command('sdptest', 'PATH B DEFAULT A')
    for bad in ('a:,,b:', 'a:,', 'q:', 'sys:', 'a', '*,a:,b:,c:,d:', '[order=(sub,cmd)]'):
        command('a:setdef ' + bad, 'Usage: SETDEF')
        command('a:setdef', ('path', 'B:,B:,*,A:'))
    command('a:setdef \t b: , a: \t', ('path', 'B:,A:'))
    shortcut('ctrl-alt-2', 1)
    command('setdef', ('path', '*'))
    command('sdptest', 'PATH A DEFAULT A')
    command('setdef a:', ('path', 'A:'))
    shortcut('ctrl-alt-1', 0)
    command('a:setdef', ('path', 'B:,A:'))
    command('a:setdef c:', ('path', 'C:'))
    command('sysfall', 'PATH A DEFAULT A')
    command('sysdisk b:', 'System disk is B:')
    text = command('a:setdef', ('path', 'C:'))
    assert 'Sys Disk (B:)' in text, text
    command('sysfall', 'PATH B DEFAULT A')
    command('sdonly', 'Command not found. Use ? for help')
    command('sysdisk a:', 'System disk is A:')
    command('a:setdef *', ('path', '*'))
    type_command('help setdef [nopage]\n')
    wait_for(lambda t: 'HELP>' in t and 'SETDEF does not allocate' in t)
    type_command('\n')
    wait_for(prompt)
    type_command('b:\n')
    wait_for(lambda t: prompt(t, 'B'))
    command('sdptest', 'PATH B DEFAULT B', 'B')
    command('a:setdef', ('path', '*'), 'B')
    type_command('a:\n')
    wait_for(prompt)
    shortcut('ctrl-alt-2', 1)
    command('setdef *', ('path', '*'))
    shortcut('ctrl-alt-1', 0)
    print('SETDEF: defaults, ordering, explicit drive, absent drive, duplicates, '
          'syntax atomicity, console isolation, working drive, SUBMIT/chain and fixed SYS fallback passed', flush=True)


def check_subsearch(type_command, wait_for, shortcut):
    def prompt(t):
        lines = [line.rstrip() for line in t.splitlines()[:24] if line.strip()]
        return bool(lines) and bool(re.fullmatch(r'(?:[0-9]+)?A>', lines[-1]))
    def command(tail, expected):
        type_command('cls\n')
        wait_for(lambda t: not search_display_matches(t, expected) and prompt(t))
        type_command(tail + '\n')
        text, _ = wait_for(lambda t: search_display_matches(t, expected) and prompt(t))
        assert 'WRONG' not in text, text
        return text
    command('a:setdef b:,a:', ('order', 'CMD'))
    command('onlysub', 'Command not found. Use ? for help')
    command('b:onlysub.sub first second', 'SCRIPT B FIRST SECOND')
    command('a:setdef [order=(cmd,sub)]', ('order', 'CMD,SUB'))
    command('a:setdef', ('path', 'B:,A:'))
    command('onlysub first second', 'SCRIPT B FIRST SECOND')
    command('sdptest', 'PATH B DEFAULT A')
    command('early', 'EARLY SUB B')
    command('badcmd', 'Invalid or empty CMD file')
    command('a:early.cmd', 'PATH A DEFAULT A')
    command('b:onlysub.cmd', 'Command not found. Use ? for help')
    command('onlysub.txt', 'Invalid command extension')
    command('nest', 'NEST END')
    command('include', 'INCLUDE END')
    command('a:setdef b:', ('order', 'CMD,SUB'))
    command('sysbatch', 'SYSTEM SCRIPT A')
    command('user 1', 'User Number = 1')
    command('usertest', 'USER ZERO A')
    command('user 0', 'User Number = 0')
    for invalid in ('[order=(sub)]', '[order=(sub,cmd)]', '[order=(cmd,cmd)]',
                    '[order=(cmd,sub,cmd)]', '[order=(cmd)] extra',
                    'a: [order=(sub)]', '[temporary=b:]', '[order=(cmd)'):
        command('a:setdef ' + invalid, 'Usage: SETDEF')
        text = command('a:setdef', ('order', 'CMD,SUB'))
        assert search_display_matches(text, ('path', 'B:')), text
    command('onlysub ' + 'x' * 112, 'SCRIPT B ' + 'X' * 12)
    command('onlysub ' + 'x' * 113, 'Command tail too long for SUBMIT')
    command('onlysub ' + 'x' * 119, 'Command tail too long for SUBMIT')
    shortcut('ctrl-alt-2', 1)
    command('a:setdef', ('order', 'CMD'))
    command('a:setdef b:,a: [order=(cmd,sub)]', ('order', 'CMD,SUB'))
    command('onlysub isolated', 'SCRIPT B ISOLATED')
    command('a:setdef [order=(cmd)]', ('order', 'CMD'))
    command('onlysub', 'Command not found. Use ? for help')
    command('a:setdef *', ('path', '*'))
    shortcut('ctrl-alt-1', 0)
    command('a:setdef', ('order', 'CMD,SUB'))
    command('sysdisk b:', 'System disk is B:')
    command('onlysub', 'SUBMIT.CMD not found')
    command('sysdisk a:', 'System disk is A:')
    command('a:setdef * [order=(cmd)]', ('order', 'CMD'))
    command('onlysub', 'Command not found. Use ? for help')
    print('SUB search: opt-in, explicit types, drive/type precedence, arguments, '
          'nested/include/chain, SYS user zero, overflow, invalid options and isolation passed', flush=True)


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
            disks = (search_fixtures(temporary, image, second)
                     if image.name == 'ccpm86.img' else (image, second))
            for index, disk in enumerate(disks):
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
                            ',': 'comma', '(': 'shift-9', ')': 'shift-0', '=': 'equal',
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
                    check_setdef(type_command, wait_for, shortcut)
                    check_subsearch(type_command, wait_for, shortcut)
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
    check(MEDIA / 'ccpm86.img', MEDIA / 'ccpmext.img')
    check(MEDIA / 'ccpmtools.img', MEDIA / 'ccpm86.img')
    check(MEDIA / 'ccpmmaint.img', MEDIA / 'ccpm86.img')
