#!/usr/bin/env python3
"""Exercise recovered PCTMP on a disposable QEMU overlay (text/FCB subset).

Runs on a snapshot and never writes to the source image.
Optional graphics checks cover the supplied demos, not general BIOS compatibility.
"""
import argparse
import json
import re
from pathlib import Path
import select
import subprocess
import tempfile
import time
from pcmem import accounting

ROOT = Path(__file__).resolve().parent.parent


def check(image, output, shell_checks=False, keyboard_checks=False, state_checks=False, graphics_checks=False, loader_checks=False, tools_image=None, failure_checks=False, failure_case='all', mz_checks=False, break_vector_checks=False, memory_map=None, header_checks=False, relocation_checks=False, size_checks=False, recovery_checks=False, equipment_checks=False, pixel_read_checks=False, builtin_checks=False, enhanced_keyboard_checks=False, help_checks=False, help_tail_checks=False, native_help_checks=False, search_checks=False, native_search_checks=False, toolchain_checks=False, drive_checks=False, command_errors=None):
    output.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix='pcmode-test-') as td:
        video = Path(td) / 'video.bin'
        args = ['qemu-system-i386', '-machine', 'pc', '-accel', 'tcg', '-m', '16',
                '-display', 'none', '-serial', 'none', '-monitor', 'none', '-qmp', 'stdio',
                '-net', 'none', '-boot', 'a', '-no-fd-bootchk', '-no-reboot', '-snapshot',
                '-drive', f'file={image.resolve()},format=raw,if=floppy,index=0']
        if tools_image:
            args += ['-drive', f'file={tools_image.resolve()},format=raw,if=floppy,index=1']
        process = subprocess.Popen(args, stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                                   stderr=subprocess.DEVNULL, bufsize=0)
        def receive():
            if not select.select([process.stdout], [], [], 10)[0]:
                raise TimeoutError('QMP timeout')
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
            hmp(f'pmemsave 0xb8000 4000 "{video}"')
            data = video.read_bytes()
            return '\n'.join(''.join(chr(c) if 32 <= c < 127 else ' '
                             for c in data[row * 160:row * 160 + 160:2]) for row in range(25))
        def prompt(text):
            rows = [s.strip() for s in text.splitlines()[:24] if s.strip()]
            return bool(rows) and rows[-1] == 'A>'
        def wait(predicate):
            end = time.monotonic() + 30
            while time.monotonic() < end:
                text = screen()
                if 'Unexpected Interrupt' in text or 'Disk Error' in text or 'TEST FAILED' in text:
                    raise AssertionError(text)
                if predicate(text):
                    return text
                time.sleep(.2)
            raise AssertionError(screen())
        def type_command(text):
            for char in text:
                reply = hmp('sendkey ' + {'\n': 'ret', '\t': 'tab', '?': 'shift-slash', '*': 'shift-8', ' ': 'spc', '.': 'dot', ':': 'shift-semicolon',
                                         '[': 'bracket_left', ']': 'bracket_right',
                                         ';': 'semicolon', '=': 'equal', '(': 'shift-9', ')': 'shift-0', ',': 'comma'}.get(char, 'shift-' + char.lower() if 'A' <= char <= 'Z' else char) + ' 50')
                if reply:
                    raise RuntimeError('QMP key injection failed: ' + reply)
                time.sleep(.12)
        results = []
        try:
            receive()
            command('qmp_capabilities')
            wait(prompt)
            if command_errors:
                from pcerrorcheck import error_checks, diagnostic_checks
                if command_errors == 'diag':
                    results.extend(diagnostic_checks(wait, type_command, prompt, output))
                else:
                    results.extend(error_checks(wait, type_command, prompt, output, command_errors == 'native'))
                (output / 'results.txt').write_text('\n'.join(results) + '\n')
                print('\n'.join(results))
                return
            if drive_checks:
                from pcdrivecheck import drive_checks as check_drives
                results.extend(check_drives(wait, type_command, output))
                (output / 'results.txt').write_text('\n'.join(results) + '\n')
                print('\n'.join(results))
                return
            if native_search_checks:
                from pcpathcheck import search_checks as check_search
                results.extend(check_search(hmp, wait, type_command, prompt, output, True))
                (output / 'results.txt').write_text('\n'.join(results) + '\n')
                print('\n'.join(results))
                return
            if native_help_checks:
                from pchelpcheck import native_help
                results.extend(native_help(hmp, wait, type_command, prompt, output))
                (output / 'results.txt').write_text('\n'.join(results) + '\n')
                print('\n'.join(results))
                return
            for n, (text, marker) in enumerate([
                    ('pcprobe', 'PCMODE DOS PROBE OK'),
                    ('pcfile', 'PCMODE FCB TEST OK'),
                    ('pcfile', 'PCMODE FCB TEST OK'),
                    ('pcvideo', 'PCMODE VIDEO TEST OK'),
                    ('type missing.txt', 'FILE NOT FOUND.'),
                    ('pcprobe', 'PCMODE DOS PROBE OK')]):
                type_command(text + '\n')
                time.sleep(1)
                result = wait(prompt)
                if marker not in result.upper() or 'Console=0' not in result:
                    raise AssertionError(result)
                (output / f'case{n}.txt').write_text(result)
                results.append('PASS: ' + text)
            if search_checks:
                from pcpathcheck import search_checks as check_search
                results.extend(check_search(hmp, wait, type_command, prompt, output))
            if builtin_checks:
                # Require fresh output immediately above the prompt, so old
                # SYSDISK messages cannot satisfy a later command's assertion.
                def builtin(text, marker, name):
                    type_command(text + '\n')
                    time.sleep(.5)
                    result = wait(prompt)
                    rows = [line.strip() for line in result.splitlines()[:24] if line.strip()]
                    expected = marker if isinstance(marker, list) else [marker]
                    if rows[-len(expected)-1:] != expected + ['A>']:
                        raise AssertionError(result)
                    (output / (name + '.txt')).write_text(result)
                    results.append('PASS: ' + name)
                builtin('sysdisk', 'System disk is A:', 'sysdisk-query')
                for index, tail in enumerate(['q:', 'a', 'aa:', 'a: extra', ':', '1:']):
                    builtin('sysdisk ' + tail, 'Invalid drive code, use SYSDISK A:',
                            'sysdisk-syntax-' + str(index))
                builtin('sysdisk p:', 'Specified device does not exist', 'sysdisk-unavailable')
                builtin('sysdisk', 'System disk is A:', 'sysdisk-failure-preserves-system')
                builtin('sysdisk b:', 'System disk is B:', 'sysdisk-set-b')
                builtin('sysdisk   ', 'System disk is B:', 'sysdisk-blank-tail')
                # A second PCTMP instance must observe the shared SYSDAT value.
                hmp('sendkey ctrl-alt-2 100')
                wait(lambda s: 'Console=1' in s and prompt(s))
                builtin('sysdisk', 'System disk is B:', 'sysdisk-global-console1')
                builtin('sysdisk a:   ', 'System disk is A:', 'sysdisk-reset-console1')
                type_command('b:pcbusy\n')
                wait(lambda s: 'PCMODE FILE HELD OPEN' in s and not prompt(s))
                hmp('sendkey ctrl-alt-1 100')
                wait(lambda s: 'Console=0' in s and prompt(s))
                builtin('sysdisk b:', 'Unable to set new system disk because of open files',
                        'sysdisk-open-file-refusal')
                builtin('sysdisk', 'System disk is A:', 'sysdisk-busy-preserves-system')
                hmp('sendkey ctrl-alt-2 100')
                wait(lambda s: 'Console=1' in s and 'PCMODE FILE HELD OPEN' in s)
                hmp('sendkey esc 50')
                wait(prompt)
                builtin('sysdisk b:', 'System disk is B:', 'sysdisk-after-file-close')
                builtin('sysdisk a:', 'System disk is A:', 'sysdisk-restore-after-close')
                hmp('sendkey ctrl-alt-1 100')
                wait(lambda s: 'Console=0' in s and prompt(s))
                builtin('sysdisk', 'System disk is A:', 'sysdisk-global-console0')
                builtin('user', ['User Number = 0', 'Note:  User numbers only apply to CP/M media.'], 'builtin-user-dispatch')
                type_command('cls\n')
                time.sleep(.5)
                result = wait(prompt)
                rows = [line.strip() for line in result.splitlines()[:24] if line.strip()]
                if rows != ['A>'] or 'Console=0' not in result:
                    raise AssertionError(result)
                (output / 'cls.txt').write_text(result)
                results.append('PASS: CLS clears text and preserves status row')
                for text in ['cls.com', 'sysdisk.com', 'a:cls', 'a:sysdisk', 'clsx']:
                    type_command(text + '\n')
                    time.sleep(.5)
                    result = wait(prompt)
                    rows = [line.strip() for line in result.splitlines()[:24] if line.strip()]
                    if rows[-3:] != ["Command error: Command not found. Use ? for help", 'Command = ' + text, 'A>']:
                        raise AssertionError(result)
                    results.append('PASS: external lookup for ' + text)
                builtin('pcprobe', 'PCMODE DOS PROBE OK', 'builtins-dos-return')
            if help_checks or help_tail_checks:
                from pchelpcheck import help_commands
                results.extend(help_commands(hmp, screen, wait, type_command,
                                             prompt, output, help_tail_checks))
            if shell_checks:
                # Exercise the inherited BDOS 10 editor at the PCTMP prompt.
                def edited_command(prefix, keys, suffix, name):
                    type_command(prefix)
                    for key in keys:
                        hmp('sendkey ' + key + ' 50')
                        time.sleep(.15)
                    type_command(suffix + '\n')
                    time.sleep(1)
                    result = wait(prompt)
                    rows = [line.strip() for line in result.splitlines()[:24] if line.strip()]
                    if rows[-2:] != ['PCMODE DOS PROBE OK', 'A>'] or 'Console=0' not in result:
                        raise AssertionError(result)
                    (output / (name + '.txt')).write_text(result)
                    results.append('PASS: ' + name)
                edited_command('pcproxe', ['ctrl-a', 'ctrl-a', 'ctrl-g'], 'b', 'edit-middle')
                edited_command('xpcprobe', ['home', 'ctrl-g', 'end'], '', 'edit-home-end')
                edited_command('', ['ctrl-w'], '', 'edit-recall')
                for text, marker, name in [
                        ('setdef', 'Drive Search Path:', 'setdef-query'),
                        ('setdef [order=(cmd,sub)]', 'CMD, SUB', 'setdef-order')]:
                    type_command(text + '\n')
                    time.sleep(1)
                    result = wait(prompt)
                    if marker not in result:
                        raise AssertionError(result)
                    (output / (name + '.txt')).write_text(result)
                    results.append('PASS: ' + name)
                # Record lookup separately: SETDEF's API working does not prove
                # that the old PCTMP pre-search reaches the enhanced CLI policy.
                type_command('pctest\n')
                time.sleep(1)
                result = wait(prompt)
                (output / 'setdef-sub-lookup.txt').write_text(result)
                if 'PCMODE SUB TEST OK' in result:
                    results.append('PASS: bare SUB command executed')
                else:
                    results.append('LIMIT: bare SUB command did not execute; see setdef-sub-lookup.txt')
                type_command('submit pctest\n')
                time.sleep(1)
                result = wait(lambda s: prompt(s) and 'PCMODE SUB TEST OK' in s)
                (output / 'setdef-explicit-submit.txt').write_text(result)
                results.append('PASS: explicit SUBMIT executes the same SUB fixture')
                type_command('a:setdef [order=(cmd,dos)]\n')
                restored = wait(prompt)
                if 'CMD, DOS' not in restored:
                    raise AssertionError(restored)
                results.append('PASS: restore automatic DOS search after native shell checks')
            if loader_checks:
                # Repeated EXE/COM alternation checks relocation and cleanup,
                # rather than accepting one successful load in a fresh system.
                for attempt in range(3):
                    type_command('pcexe.exe token\n')
                    time.sleep(1)
                    result = wait(prompt)
                    rows = [line.strip() for line in result.splitlines()[:24] if line.strip()]
                    if rows[-2:] != ['PCMODE EXE TEST OK', 'A>']:
                        raise AssertionError(result)
                    (output / f'exe{attempt}.txt').write_text(result)
                    type_command('pcprobe\n')
                    time.sleep(1)
                    result = wait(prompt)
                    rows = [line.strip() for line in result.splitlines()[:24] if line.strip()]
                    if rows[-2:] != ['PCMODE DOS PROBE OK', 'A>']:
                        raise AssertionError(result)
                    results.append(f'PASS: EXE relocation/PSP/stack/tail and COM return, cycle {attempt + 1}')
            if failure_checks:
                bad_files = {'memory': 'pcnomem.exe', 'empty': 'pcempty.exe', 'short': 'pcshort.exe'}
                names = bad_files.values() if failure_case == 'all' else ([bad_files[failure_case]] if failure_case in bad_files else [])
                for name in names:
                    type_command(name + '\n')
                    time.sleep(1)
                    result = wait(prompt)
                    (output / (name + '.txt')).write_text(result)
                    if 'TRUNCATED EXE EXECUTED' in result or 'PCMODE EXE TEST FAILED' in result:
                        raise AssertionError('Invalid EXE was executed:\n' + result)
                    rows = [line.strip() for line in result.splitlines()[:24] if line.strip()]
                    error = 'Not enough memory' if name == 'pcnomem.exe' else 'Invalid or unreadable executable file'
                    if rows[-3:] != ['Command error: ' + error, 'Command = ' + name, 'A>']:
                        raise AssertionError('Missing expected loader error:\n' + result)
                    type_command('pcexe.exe token\n')
                    time.sleep(1)
                    result = wait(prompt)
                    rows = [line.strip() for line in result.splitlines()[:24] if line.strip()]
                    if rows[-2:] != ['PCMODE EXE TEST OK', 'A>']:
                        raise AssertionError(result)
                    results.append('PASS: reject ' + name + ' and load valid EXE afterward')
                for attempt in range(2 if failure_case in ('all', 'break') else 0):
                    type_command('pcbreak\n')
                    wait(lambda s: 'PCMODE BREAK READY' in s and not prompt(s))
                    hmp('sendkey ctrl-c 100')
                    result = wait(prompt)
                    (output / f'break{attempt}.txt').write_text(result)
                    if 'CTRL-C RETURNED TO PROGRAM' in result:
                        raise AssertionError(result)
                    hmp(f'pmemsave 0x449 1 "{video}"')
                    if video.read_bytes() != b'\x03':
                        raise AssertionError('Ctrl-C left physical mode altered')
                    type_command('pcstate\n')
                    wait(lambda s: 'PCMODE STATE READY' in s and not prompt(s))
                    hmp('sendkey esc 50')
                    wait(prompt)
                    type_command('pcfile\n')
                    time.sleep(1)
                    result = wait(prompt)
                    rows = [line.strip() for line in result.splitlines()[:24] if line.strip()]
                    if rows[-2:] != ['PCMODE FCB TEST OK', 'A>']:
                        raise AssertionError(result)
                    results.append(f'PASS: Ctrl-C cleanup, mode/cursor and FCB recovery, cycle {attempt + 1}')
            if break_vector_checks:
                for name, marker in [('pc23exit', 'PCMODE HANDLER EXIT'),
                                     *[(n, 'PCMODE HANDLER IRET') for n in ('pc23ret', 'pc23in8', 'pc23line', 'pc23fl1', 'pc23fl8', 'pc23fla')]]:
                    type_command(name + '\n')
                    wait(lambda s: 'PCMODE INT23 READY' in s and not prompt(s))
                    hmp('sendkey ctrl-c 100')
                    if name != 'pc23exit':
                        result = wait(lambda s: [line.strip() for line in s.splitlines()[:24] if line.strip()][-1:] == [marker] or prompt(s))
                        if prompt(result):
                            raise AssertionError('Returning INT 23h handler terminated the process: ' + result)
                        hmp('sendkey x 100')
                        if name in ('pc23line', 'pc23fla'):
                            time.sleep(0.3)
                            hmp('sendkey ret 100')
                    result = wait(prompt)
                    (output / (name + '.txt')).write_text(result)
                    rows = [line.strip() for line in result.splitlines()[:24] if line.strip()]
                    if name != 'pc23exit':
                        if not rows[-2].endswith('PCMODE INT23 RESUMED') or rows[-1] != 'A>':
                            raise AssertionError(result)
                    elif rows[-2:] != [marker, 'A>']:
                        raise AssertionError(result)
                    # A fresh process must use the default break handler, not
                    # the vector left by the exited application.
                    type_command('pcbreak\n')
                    wait(lambda s: 'PCMODE BREAK READY' in s and not prompt(s))
                    hmp('sendkey ctrl-c 100')
                    result = wait(prompt)
                    (output / (name + '-restore.txt')).write_text(result)
                    rows = [line.strip() for line in result.splitlines()[:24] if line.strip()]
                    if any('PCMODE HANDLER' in row or 'CTRL-C RETURNED' in row for row in rows[-3:]):
                        raise AssertionError('Custom vector leaked into next process: ' + result)
                    type_command('pcfile\n')
                    time.sleep(1)
                    result = wait(prompt)
                    rows = [line.strip() for line in result.splitlines()[:24] if line.strip()]
                    if rows[-2:] != ['PCMODE FCB TEST OK', 'A>']:
                        raise AssertionError(result)
                    results.append('PASS: custom INT 23h dispatch, default-vector restoration and FCB recovery: ' + name)
            if memory_map:
                manifest = json.loads(memory_map.read_text())
                if manifest['version'] != '1431':
                    raise AssertionError('Memory layout requires the inspected CCP/M 3.1 build')
                def measure(tag):
                    time.sleep(.5)  # allow the exited process to finish cleanup
                    dump = output / (tag + '.bin')
                    command('stop')
                    try:
                        hmp(f'pmemsave 0 655360 "{dump}"')
                        state = accounting(dump.read_bytes(), manifest['data_base'])
                    finally:
                        command('cont')
                    (output / (tag + '.json')).write_text(json.dumps(state, indent=2))
                    return state
                # Warm the loader once before comparing steady-state accounting.
                type_command('pcexe.exe token\n')
                time.sleep(1)
                wait(prompt)
                baseline = measure('memory-baseline')
                for cycle in range(3):
                    for name in ('pcexe.exe token', 'pcsswrap.exe', 'pcfixbad.exe', 'pcbreak'):
                        type_command(name + '\n')
                        if name == 'pcbreak':
                            wait(lambda s: 'PCMODE BREAK READY' in s and not prompt(s))
                            active = measure(f'memory-{cycle}-active')
                            if not (active['free_paragraphs'] < baseline['free_paragraphs']
                                    and active['allocated_paragraphs'] > baseline['allocated_paragraphs']):
                                raise AssertionError('Accounting did not detect the live DOS allocation')
                            hmp('sendkey ctrl-c 100')
                        else:
                            time.sleep(1)
                        result = wait(prompt)
                        rows = [line.strip() for line in result.splitlines()[:24] if line.strip()]
                        if name in ('pcsswrap.exe', 'pcfixbad.exe'):
                            if rows[-3:] != ['Command error: Invalid or unreadable executable file', 'Command = ' + name, 'A>']:
                                raise AssertionError(result)
                        elif name == 'pcexe.exe token' and rows[-2:] != ['PCMODE EXE TEST OK', 'A>']:
                            raise AssertionError(result)
                        state = measure(f'memory-{cycle}-{name.split()[0]}')
                        for field in ('free_paragraphs', 'allocated_paragraphs', 'unused_descriptors'):
                            if state[field] != baseline[field]:
                                raise AssertionError(f'Memory accounting drift after {name}: {field}: {baseline[field]} -> {state[field]}')
                        results.append(f'PASS: memory accounting unchanged after {name}, cycle {cycle + 1}')
            if mz_checks or header_checks or relocation_checks or size_checks:
                names = ('pcstbss', 'pcstzero', 'pcstmax', 'pcsswrap', 'pcssout', 'pcstend', 'pch128', 'pch256', 'pch384', 'pch48', 'pcround', 'pcfixbad', 'pchdrbad', 'pcovflow', 'pcentbad', 'pctblbad') if mz_checks else ()
                if header_checks:
                    if not tools_image:
                        raise AssertionError('--headers requires the matching --tools-image')
                    names += tuple('b:' + n for n in ('pch32', 'pch48', 'pch64', 'pch80', 'pch96', 'pch112', 'pch144', 'pch496', 'pch528', 'pclarge'))
                if relocation_checks:
                    if not tools_image:
                        raise AssertionError('--relocations requires the matching --tools-image')
                    names += tuple('b:' + n for n in ('pcrlate', 'pcrpost', 'pcr124', 'pcr125', 'pcr126', 'pcr127', 'pcr129', 'pcr255', 'pcrmany', 'pcrbig', 'pcrnone'))
                if size_checks:
                    if not tools_image:
                        raise AssertionError('--sizes requires the matching size-fixture --tools-image')
                    names += tuple('b:' + n for n in ('pcs128', 'pcs129', 'pcs255', 'pcs256', 'pcs257', 'pcs512', 'pcs1k', 'pcs384', 'pcpgzero', 'pcpgbig', 'pcbl512', 'pcminhdr', 'pcsig', 'pcipend', 'pcfxend', 'pcnone'))
                for name in names:
                    type_command(name + '.exe\n')
                    time.sleep(1)
                    result = wait(prompt)
                    (output / (name + '.txt')).write_text(result)
                    rows = [line.strip() for line in result.splitlines()[:24] if line.strip()]
                    fixture_name = name.removeprefix('b:')
                    if fixture_name in ('pcs128', 'pcs129', 'pcs255', 'pcs256', 'pcs257', 'pcs512', 'pcs1k', 'pcs384'):
                        if rows[-2:] != ['PCMODE SIZE TEST OK', 'A>']:
                            raise AssertionError(result)
                    elif fixture_name.startswith('pcr') and fixture_name != 'pcround':
                        if rows[-2:] != ['PCMODE FIXUP TEST OK', 'A>']:
                            raise AssertionError(result)
                    elif fixture_name == 'pclarge':
                        if rows[-2:] != ['PCMODE LARGE TEST OK', 'A>']:
                            raise AssertionError(result)
                    elif fixture_name in ('pcstbss', 'pcstzero', 'pcstmax'):
                        if rows[-2:] != ['PCMODE STACK TEST OK', 'A>']:
                            raise AssertionError(result)
                    elif fixture_name in ('pcround', 'pch128', 'pch256', 'pch384', 'pch32', 'pch48', 'pch64', 'pch80', 'pch96', 'pch112', 'pch144', 'pch496', 'pch528'):
                        if rows[-2:] != ['PCMODE ROUND TEST OK', 'A>']:
                            raise AssertionError(result)
                    elif rows[-3:] != ['Command error: Invalid or unreadable executable file', 'Command = ' + name + '.exe', 'A>']:
                        raise AssertionError(result)
                    type_command('pcexe.exe token\n')
                    time.sleep(1)
                    result = wait(prompt)
                    rows = [line.strip() for line in result.splitlines()[:24] if line.strip()]
                    if rows[-2:] != ['PCMODE EXE TEST OK', 'A>']:
                        raise AssertionError(result)
                    results.append('PASS: MZ bounds/rounding ' + name + ' and valid EXE recovery')
            if toolchain_checks:
                if not tools_image:
                    raise ValueError('--toolchain requires --tools-image with the optional DOS tools')
                commands = [
                    ('b:msmasm b:hellodos,b:hellodos,b:hellodos,b:hellodos', 'masm'),
                    ('b:link b:hellodos,b:hellodos,b:hellodos,b:hellodos,', 'link'),
                    ('b:hellodos.exe', 'hello-exe'),
                    ('b:exe2bin b:hellodos.exe b:hellodos.com', 'exe2bin'),
                    ('b:hellodos.com', 'hello-com')]
                for text, name in commands:
                    type_command(text + '\n')
                    time.sleep(1)
                    result = wait(prompt)
                    (output / (name + '.txt')).write_text(result)
                    if name == 'masm' and not re.search(r'Errors\s+Errors\s*\n\s*0\s+0\b', result):
                        raise AssertionError('Assembler did not report zero errors:\n' + result)
                    if name.startswith('hello'):
                        rows = [line.strip() for line in result.splitlines()[:24] if line.strip()]
                        if rows[-2:] != ['PCMODE MASM HELLO OK', 'A>']:
                            raise AssertionError(result)
                    results.append('PASS: DOS toolchain stage ' + name)
            # Current kernel's explicit next-console shortcut.
            hmp('sendkey ctrl-alt-tab 100')
            result = wait(lambda s: 'Console=1' in s and prompt(s))
            type_command('pcprobe\n')
            time.sleep(1)
            result = wait(lambda s: prompt(s) and 'PCMODE DOS PROBE OK' in s)
            (output / 'console1.txt').write_text(result)
            results.append('PASS: console 1 DOS probe and return')
            type_command('pcvideo\n')
            time.sleep(1)
            result = wait(lambda s: prompt(s) and 'PCMODE VIDEO TEST OK' in s)
            (output / 'console1-video.txt').write_text(result)
            results.append('PASS: console 1 BIOS text operations and return')
            if header_checks or relocation_checks:
                # Exercise every nonzero console's scratch/state slot. The
                # complete layout matrix above ran on console zero.
                console_cases = []
                if header_checks:
                    console_cases += [('pch48', 'PCMODE ROUND TEST OK'),
                                      ('pclarge', 'PCMODE LARGE TEST OK')]
                if relocation_checks:
                    console_cases += [('pcr127', 'PCMODE FIXUP TEST OK'),
                                      ('pcrmany', 'PCMODE FIXUP TEST OK')]
                for console in (1, 2, 3):
                    if console != 1:
                        hmp(f'sendkey ctrl-alt-{console + 1} 100')
                        wait(lambda s: f'Console={console}' in s and prompt(s))
                    for name, marker in console_cases:
                        type_command('b:' + name + '.exe\n')
                        time.sleep(1)
                        result = wait(prompt)
                        rows = [line.strip() for line in result.splitlines()[:24] if line.strip()]
                        if rows[-2:] != [marker, 'A>']:
                            raise AssertionError(result)
                        (output / f'console{console}-{name}.txt').write_text(result)
                        results.append(f'PASS: console {console} EXE layout: {name}')
                hmp('sendkey ctrl-alt-2 100')
                wait(lambda s: 'Console=1' in s and prompt(s))
            if enhanced_keyboard_checks:
                from pckeycheck import enhanced_keyboard
                results.extend(enhanced_keyboard(command, hmp, screen, wait,
                                                 type_command, prompt, output))
            if keyboard_checks:
                # Start on console 1 after the normal suite. Test actual QMP
                # make/break events, not synthetic writes to driver variables.
                def key_event(key, down):
                    command('input-send-event', {'events': [
                        {'type': 'key', 'data': {'down': down,
                         'key': {'type': 'qcode', 'data': key}}}]})
                    time.sleep(.15)
                def shift_value(value):
                    def matches(text):
                        rows = [line.strip() for line in text.splitlines()[:24]
                                if line.strip()]
                        # Old SHIFT lines remain above the command prompt.
                        # Accept only the running probe's final output line.
                        return bool(rows) and rows[-1] == 'SHIFT=' + value
                    return wait(matches)
                type_command('pckey\n')
                shift_value('00')
                for key, value in [('shift_r', '01'), ('shift', '02'),
                                   ('ctrl', '04'), ('alt', '08')]:
                    key_event(key, True)
                    shift_value(value)
                    key_event(key, False)
                    shift_value('00')
                    results.append('PASS: BIOS modifier ' + key)
                for key, value in [('caps_lock', '40'), ('num_lock', '20'),
                                   ('insert', '80')]:
                    key_event(key, True)
                    key_event(key, False)
                    shift_value(value)
                    key_event(key, True)
                    key_event(key, False)
                    shift_value('00')
                    results.append('PASS: BIOS toggle ' + key)
                # Native keypad semantics: Num Lock makes keypad 0 numeric;
                # holding Shift reverses it to Insert. Verify status follows.
                key_event('num_lock', True)
                key_event('num_lock', False)
                shift_value('20')
                key_event('kp_0', True)
                key_event('kp_0', False)
                shift_value('20')
                key_event('shift', True)
                shift_value('22')
                key_event('kp_0', True)
                key_event('kp_0', False)
                shift_value('A2')
                key_event('shift', False)
                shift_value('A0')
                key_event('num_lock', True)
                key_event('num_lock', False)
                shift_value('80')
                key_event('insert', True)
                key_event('insert', False)
                shift_value('00')
                results.append('PASS: BIOS Insert follows native keypad Num/Shift semantics')
                key_event('scroll_lock', True)
                key_event('scroll_lock', False)
                # Scroll Lock retains native Ctrl-S output flow control.
                hmp('sendkey ctrl-q 50')
                time.sleep(.2)
                shift_value('10')
                key_event('insert', True)
                key_event('insert', False)
                result = shift_value('90')
                (output / 'keyboard-console1.txt').write_text(result)
                hmp('sendkey esc 50')
                wait(prompt)
                hmp('sendkey ctrl-alt-1 100')
                wait(lambda s: 'Console=0' in s and prompt(s))
                type_command('pckey\n')
                result = shift_value('00')
                (output / 'keyboard-console0.txt').write_text(result)
                hmp('sendkey esc 50')
                wait(prompt)
                hmp('sendkey ctrl-alt-2 100')
                wait(lambda s: 'Console=1' in s and prompt(s))
                type_command('pckey\n')
                shift_value('90')
                key_event('scroll_lock', True)
                key_event('scroll_lock', False)
                hmp('sendkey ctrl-q 50')
                time.sleep(.2)
                shift_value('80')
                key_event('insert', True)
                key_event('insert', False)
                shift_value('00')
                hmp('sendkey esc 50')
                wait(prompt)
                results.append('PASS: BIOS Scroll/Insert latches persist per console')
            if state_checks:
                def bios_mode(expected):
                    hmp(f'pmemsave 0x449 1 "{video}"')
                    if video.read_bytes() != bytes([expected]):
                        raise AssertionError('BIOS physical mode did not restore: ' + video.read_bytes().hex())
                type_command('pcstate\n')
                result = wait(lambda s: 'PCMODE STATE READY' in s and not prompt(s))
                bios_mode(2)
                (output / 'state-running.txt').write_text(result)
                # Switch away while a DOS program is blocked in BIOS keyboard input.
                hmp('sendkey ctrl-alt-1 100')
                wait(lambda s: 'Console=0' in s and prompt(s))
                type_command('pcprobe\n')
                time.sleep(1)
                wait(lambda s: prompt(s) and 'PCMODE DOS PROBE OK' in s)
                hmp('sendkey ctrl-alt-2 100')
                wait(lambda s: 'Console=1' in s and 'PCMODE STATE READY' in s)
                hmp('sendkey esc 50')
                result = wait(prompt)
                bios_mode(3)
                (output / 'state-restored.txt').write_text(result)
                results.append('PASS: text mode restores after switching during DOS execution')
                # Entry also reads the hardware cursor disable bit, proving that
                # the previous DOS program did not leave the native cursor hidden.
                for mode in (4, 5, 6):
                    type_command(f'pcstate g{mode}\n')
                    def physical_mode_ready(text):
                        hmp(f'pmemsave 0x449 1 "{video}"')
                        return video.read_bytes() == bytes([mode])
                    wait(physical_mode_ready)
                    hmp('sendkey esc 50')
                    wait(prompt)
                    bios_mode(3)
                    results.append(f'PASS: graphics mode {mode} exit restores physical text mode')
                type_command('pcstate\n')
                wait(lambda s: 'PCMODE STATE READY' in s and not prompt(s))
                hmp('sendkey esc 50')
                wait(prompt)
                bios_mode(3)
                results.append('PASS: repeated DOS entry restores cursor and video mode')
                for console in (2, 3):
                    hmp(f'sendkey ctrl-alt-{console + 1} 100')
                    wait(lambda s: f'Console={console}' in s and prompt(s))
                    for name, marker in [('pcprobe', 'PCMODE DOS PROBE OK'),
                                         ('pcvideo', 'PCMODE VIDEO TEST OK')]:
                        type_command(name + '\n')
                        time.sleep(1)
                        result = wait(prompt)
                        if marker not in result:
                            raise AssertionError(result)
                    (output / f'state-console{console}.txt').write_text(result)
                    results.append(f'PASS: console {console} DOS and BIOS text operations')
            if pixel_read_checks:
                if not tools_image:
                    raise AssertionError('--pixel-read requires the matching pixel-read companion')
                for console in (0, 1, 2, 3):
                    hmp(f'sendkey ctrl-alt-{console + 1} 100')
                    wait(lambda s: f'Console={console}' in s and prompt(s))
                    type_command('b:pcread\n')
                    time.sleep(.5)
                    result = wait(prompt)
                    rows = [line.strip() for line in result.splitlines()[:24] if line.strip()]
                    if rows[-2:] != ['PCMODE PIXEL READ TEST OK', 'A>']:
                        raise AssertionError(result)
                    (output / f'pixel-read-c{console}.txt').write_text(result)
                    type_command('pcstate\n')
                    wait(lambda s: 'PCMODE STATE READY' in s and not prompt(s))
                    hmp('sendkey esc 50')
                    wait(prompt)
                    type_command('pcfile\n')
                    time.sleep(.5)
                    result = wait(prompt)
                    rows = [line.strip() for line in result.splitlines()[:24] if line.strip()]
                    if rows[-2:] != ['PCMODE FCB TEST OK', 'A>']:
                        raise AssertionError(result)
                    results.append(f'PASS: CGA pixel read, banks/fields/edges/rejection and recovery, console {console}')
            if equipment_checks:
                if not tools_image:
                    raise AssertionError('--equipment requires the matching equipment companion')
                for console in (0, 1):
                    hmp(f'sendkey ctrl-alt-{console + 1} 100')
                    wait(lambda s: f'Console={console}' in s and prompt(s))
                    for cycle in range(2):
                        type_command('b:pcequip\n')
                        time.sleep(.5)
                        result = wait(prompt)
                        rows = [line.strip() for line in result.splitlines()[:24] if line.strip()]
                        if rows[-2:] != ['PCMODE EQUIPMENT TEST OK', 'A>']:
                            raise AssertionError(result)
                        (output / f'equipment-c{console}-{cycle}.txt').write_text(result)
                        # Check restored mode/cursor, BIOS input and FCB services
                        # after repeated graphics transitions and INT 11h calls.
                        type_command('pcstate\n')
                        wait(lambda s: 'PCMODE STATE READY' in s and not prompt(s))
                        hmp('sendkey esc 50')
                        wait(prompt)
                        type_command('pcfile\n')
                        time.sleep(.5)
                        result = wait(prompt)
                        rows = [line.strip() for line in result.splitlines()[:24] if line.strip()]
                        if rows[-2:] != ['PCMODE FCB TEST OK', 'A>']:
                            raise AssertionError(result)
                        results.append(f'PASS: equipment word/registers across modes 2..6 and restored input/FCB, console {console}, cycle {cycle + 1}')
            if recovery_checks:
                if not tools_image or not memory_map:
                    raise AssertionError('--recovery requires the recovery companion and matching --memory-map')
                # Graphics currently owns the hardware exclusively. Switching
                # must be refused, not silently discard its unsaved context.
                def read_physical(address, length):
                    hmp(f'pmemsave {address} {length} "{video}"')
                    return video.read_bytes()
                pattern = bytes.fromhex('5aa5') * 8192
                baseline = measure('recovery-baseline')
                for console in (0, 1):
                    hmp(f'sendkey ctrl-alt-{console + 1} 100')
                    wait(lambda s: f'Console={console}' in s and prompt(s))
                    for mode in (4, 5, 6):
                        for kind, key in (('i', 'esc'), ('d', 'esc'), ('b', 'ctrl-c')):
                            tag = f'recovery-c{console}-m{mode}-{kind}'
                            type_command(f'b:pcrecov {mode}{kind}\n')
                            def painted(text):
                                return (read_physical('0x449', 1) == bytes([mode])
                                        and read_physical('0xb8000', 16384) == pattern)
                            wait(painted)
                            active = measure(tag + '-active')
                            if active['free_paragraphs'] >= baseline['free_paragraphs']:
                                raise AssertionError('Recovery fixture did not hold a live allocation')
                            # Both direct selection and next-console navigation
                            # must leave the active CGA image untouched.
                            for shortcut in (f'ctrl-alt-{2 if console == 0 else 1}', 'ctrl-alt-tab'):
                                hmp('sendkey ' + shortcut + ' 100')
                                time.sleep(.4)
                                if not painted(''):
                                    raise AssertionError('Graphics switch altered mode or image: ' + tag)
                            hmp(f'screendump "{output.resolve() / (tag + ".ppm")}"')
                            hmp('sendkey ' + key + ' 100')
                            result = wait(lambda s: f'Console={console}' in s and prompt(s))
                            if read_physical('0x449', 1) != b'\x03':
                                raise AssertionError('Text mode not restored: ' + tag)
                            (output / (tag + '.txt')).write_text(result)
                            state = measure(tag + '-returned')
                            for field in ('free_paragraphs', 'allocated_paragraphs', 'unused_descriptors'):
                                if state[field] != baseline[field]:
                                    raise AssertionError('Recovery memory drift: ' + tag + ': ' + field)
                            # A new DOS entry checks the physical cursor-enable
                            # bit; BIOS input and FCB I/O must work after exit.
                            type_command('pcstate\n')
                            wait(lambda s: 'PCMODE STATE READY' in s and not prompt(s))
                            hmp('sendkey esc 50')
                            wait(prompt)
                            type_command('pcfile\n')
                            time.sleep(.5)
                            result = wait(prompt)
                            rows = [line.strip() for line in result.splitlines()[:24] if line.strip()]
                            if rows[-2:] != ['PCMODE FCB TEST OK', 'A>']:
                                raise AssertionError(result)
                            # Switching is allowed again after graphics cleanup.
                            other = 1 - console
                            hmp(f'sendkey ctrl-alt-{other + 1} 100')
                            wait(lambda s: f'Console={other}' in s and prompt(s))
                            hmp(f'sendkey ctrl-alt-{console + 1} 100')
                            wait(lambda s: f'Console={console}' in s and prompt(s))
                            results.append('PASS: exclusive CGA ownership, exit/cursor/FCB/memory recovery: ' + tag)
            if graphics_checks:
                type_command('pcpixel\n')
                time.sleep(1)
                result = wait(lambda s: prompt(s) and 'PCMODE PIXEL TEST OK' in s)
                (output / 'pixels.txt').write_text(result)
                results.append('PASS: CGA pixel write, XOR and coordinate bounds')
                for demo, expected in [('ballc.com', 4), ('ballc.cmd', 4), ('sine.cmd', 16)]:
                    type_command(demo + '\n')
                    def demo_ready(text):
                        hmp(f'pmemsave 0x449 1 "{video}"')
                        return video.read_bytes() == bytes([expected])
                    wait(demo_ready)
                    time.sleep(2)
                    frame = output.resolve() / (demo + '.ppm')
                    hmp(f'screendump "{frame}"')
                    if demo == 'ballc.com':
                        first = frame.read_bytes()
                        time.sleep(.7)
                        hmp(f'screendump "{frame}"')
                        if first == frame.read_bytes():
                            raise AssertionError('BALLC.COM animation did not advance')
                        results.append('PASS: BALLC.COM animation advances using BIOS clock')
                    hmp('sendkey esc 50')
                    time.sleep(1)
                    result = wait(prompt)
                    (output / (demo + '.txt')).write_text(result)
                    results.append('PASS: ' + demo + ' enters graphics and returns to prompt')
            hmp(f'screendump "{output.resolve() / "screen.ppm"}"')
            print('\n'.join(results))
            (output / 'results.txt').write_text('\n'.join(results) + '\n')
        except Exception:
            (output / 'failure.txt').write_text(screen())
            (output / 'registers.txt').write_text(hmp('info registers'))
            hmp(f'pmemsave 0 655360 "{output.resolve() / "memory.bin"}"')
            raise
        finally:
            process.terminate()
            process.wait(timeout=5)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--image', type=Path, default=ROOT / 'build/pcmode/pc-text.img')
    parser.add_argument('--output', type=Path, default=ROOT / 'build/pcmode/tests')
    parser.add_argument('--command-errors', choices=['pc', 'native', 'diag'], help='test parser/CLI diagnostics and error recovery on matching fixtures')
    parser.add_argument('--drive-checks', action='store_true', help='compare TMP drive validation; requires a B: companion, skips DOS smoke cases')
    parser.add_argument('--builtins', action='store_true', help='test experimental CLS/SYSDISK parity; requires B: companion')
    parser.add_argument('--search', action='store_true', help='test four-way SETDEF search with isolated search fixture disks')
    parser.add_argument('--search-native', action='store_true', help='compare the same policy on the native TMP fixture image')
    parser.add_argument('--native-help', action='store_true', help='check native TMP help on a normal CCP/M image, skipping DOS smoke cases')
    parser.add_argument('--help-commands', action='store_true', help='test quick help and real native HELP on B: companion')
    parser.add_argument('--help-tail', action='store_true', help='test exact topic tails using the isolated HELP.COM fixture companion')
    parser.add_argument('--shell', action='store_true', help='test editor/SETDEF with the build-input fixtures')
    parser.add_argument('--enhanced-keyboard', action='store_true', help='test physical E0/E1 modifiers, Insert, Print/Pause; requires new companion')
    parser.add_argument('--keyboard', action='store_true', help='test BIOS shift status with PCKEY.COM')
    parser.add_argument('--state', action='store_true', help='test video lifecycle and active DOS console switching')
    parser.add_argument('--graphics', action='store_true', help='test CGA pixels and supplied graphics demos')
    parser.add_argument('--loader', action='store_true', help='test MZ relocation and repeated EXE/COM loading')
    parser.add_argument('--toolchain', action='store_true', help='also run optional MASM/LINK/EXE2BIN tools on drive B')
    parser.add_argument('--tools-image', type=Path, help='DOS MASM/LINK/EXE2BIN data disk in drive B')
    parser.add_argument('--failures', action='store_true', help='test failed EXE loads and Ctrl-C cleanup')
    parser.add_argument('--failure-case', choices=['all', 'memory', 'empty', 'short', 'break'], default='all', help='isolate one failure regression when --failures is selected')
    parser.add_argument('--mz', action='store_true', help='test image rounding and malformed MZ bounds')
    parser.add_argument('--break-vectors', action='store_true', help='characterize custom INT 23h and verify restoration')
    parser.add_argument('--memory-map', type=Path, help='matching generated SYS manifest for read-only memory accounting')
    parser.add_argument('--headers', action='store_true', help='test sub-record MZ headers on the companion disk')
    parser.add_argument('--relocations', action='store_true', help='test arbitrary MZ relocation-table layouts')
    parser.add_argument('--sizes', action='store_true', help='test MZ size and exact-boundary fields on size-fixture disk')
    parser.add_argument('--recovery', action='store_true', help='test exclusive CGA ownership and termination cleanup; requires recovery B: disk and matching memory map')
    parser.add_argument('--equipment', action='store_true', help='test INT 11h across supported color modes on two consoles; requires equipment companion')
    parser.add_argument('--pixel-read', action='store_true', help='test bounded CGA pixel read on all four consoles; requires matching companion')
    args = parser.parse_args()
    check(args.image, args.output, args.shell, args.keyboard, args.state, args.graphics, args.loader, args.tools_image, args.failures, args.failure_case, args.mz, args.break_vectors, args.memory_map, args.headers, args.relocations, args.sizes, args.recovery, args.equipment, args.pixel_read, args.builtins, args.enhanced_keyboard, args.help_commands, args.help_tail, args.native_help, args.search, args.search_native, args.toolchain, args.drive_checks, args.command_errors)
