"""Physical QMP keyboard review shared by the PC-MODE snapshot harness.

Collect every case before failing, preserving independent baseline defects.
No driver variables, BIOS flags or input queues are written by this test.
"""
import re
import time


def enhanced_keyboard(command, hmp, screen, wait, type_command, prompt, output):
    outcomes = []
    held = set()

    def event(key, down):
        command('input-send-event', {'events': [
            {'type': 'key', 'data': {'down': down,
             'key': {'type': 'qcode', 'data': key}}}]})
        (held.add if down else held.discard)(key)
        time.sleep(.2)

    def tap(key):
        event(key, True)
        event(key, False)

    def check(name, shift, words=None):
        text = screen()
        (output / (name + '.txt')).write_text(text)
        states = re.findall(r'^SHIFT=([0-9A-F]{2})\s*$', text, re.M)
        keys = re.findall(r'^KEY=([0-9A-F]{4})\s*$', text, re.M)
        ok = bool(states) and states[-1] == shift and (words is None or keys == words)
        detail = f'shift={states[-1] if states else "missing"}, words={keys}'
        outcomes.append(('PASS: ' if ok else 'FAIL: ') + name + ': ' + detail)

    def start():
        type_command('cls\n')
        wait(prompt)
        type_command('b:pckword\n')
        wait(lambda s: 'PCMODE KEY WORD READY' in s and 'SHIFT=' in s)

    def stop():
        for key in tuple(held):
            event(key, False)
        tap('esc')
        wait(prompt)

    try:
        for name, left, right, flag in [('ctrl', 'ctrl', 'ctrl_r', '04'),
                                        ('alt', 'alt', 'alt_r', '08')]:
            for first, second in [(left, right), (right, left)]:
                start()
                event(first, True)
                check(name + '-' + first + '-down', flag, [])
                event(second, True)
                check(name + '-' + first + '-both', flag, [])
                event(first, False)
                check(name + '-' + first + '-overlap', flag, [])
                event(second, False)
                check(name + '-' + first + '-released', '00', [])
                stop()

        # Input words deliberately follow traced native XIOS raw-PFK encoding:
        # Insert = 'R' | 80h = D2h, numeric keypad zero = ASCII 30h.
        for num, shifted, key, flag, word in [
                (False, False, 'insert', '80', '00D2'),
                (True, False, 'insert', 'A0', '00D2'),
                (False, True, 'insert', '82', '00D2'),
                (True, True, 'insert', 'A2', '00D2'),
                (False, False, 'kp_0', '80', '00D2'),
                (True, False, 'kp_0', '20', '0030'),
                (False, True, 'kp_0', '02', '0030'),
                (True, True, 'kp_0', 'A2', '00D2')]:
            start()
            if num:
                tap('num_lock')
            if shifted:
                event('shift', True)
            tap(key)
            check(f'{key}-num{int(num)}-shift{int(shifted)}', flag, [word])
            if shifted:
                event('shift', False)
            if num:
                tap('num_lock')
            # Toggle Insert off only if it was actually set on this runtime.
            states = re.findall(r'^SHIFT=([0-9A-F]{2})\s*$', screen(), re.M)
            if states and int(states[-1], 16) & 0x80:
                tap('insert')
            stop()

        # Dedicated navigation remains a native raw PFK under Num+Shift.
        for key, word in [('home', '00C7'), ('up', '00C8'), ('pgup', '00C9'),
                          ('left', '00CB'), ('right', '00CD'), ('end', '00CF'),
                          ('down', '00D0'), ('pgdn', '00D1'), ('delete', '00D3'),
                          ('kp_enter', '000D'), ('kp_divide', '002F')]:
            start()
            tap('num_lock')
            event('shift', True)
            tap(key)
            check('dedicated-' + key + '-num-shift', '22', [word])
            event('shift', False)
            tap('num_lock')
            stop()

        start()
        event('insert', True)
        check('insert-independent-first', '80', ['00D2'])
        event('kp_0', True)
        check('insert-independent-both', '00', ['00D2', '00D2'])
        event('insert', False)
        event('kp_0', False)
        tap('insert')
        check('insert-independent-next-edge', '80', ['00D2', '00D2', '00D2'])
        states = re.findall(r'^SHIFT=([0-9A-F]{2})\s*$', screen(), re.M)
        if states and int(states[-1], 16) & 0x80:
            tap('insert')
        stop()

        for key in ('print', 'pause'):
            for real_shift in (False, True):
                start()
                if real_shift:
                    event('shift', True)
                tap(key)
                check(f'{key}-real-shift{int(real_shift)}',
                      '02' if real_shift else '00', [])
                if real_shift:
                    event('shift', False)
                # A normal key after the sequence must still be delivered.
                tap('a')
                text = screen()
                words = re.findall(r'^KEY=([0-9A-F]{4})\s*$', text, re.M)
                outcomes.append(('PASS: ' if words and words[-1] == '0061' else 'FAIL: ')
                                + key + '-subsequent-input')
                # Baseline Pause can toggle Num Lock: restore it for the next case.
                states = re.findall(r'^SHIFT=([0-9A-F]{2})\s*$', text, re.M)
                if states and int(states[-1], 16) & 0x20:
                    tap('num_lock')
                stop()
    finally:
        for key in tuple(held):
            event(key, False)
        (output / 'keyboard-results.txt').write_text('\n'.join(outcomes) + '\n')
    if any(line.startswith('FAIL:') for line in outcomes):
        raise AssertionError('\n'.join(outcomes))
    return outcomes
