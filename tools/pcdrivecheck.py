"""Compare TMP drive-command behavior on disposable emulator snapshots."""
import re
import time


def drive_checks(wait, type_command, output):
    def prompt(text, drive='A'):
        rows = [row.strip() for row in text.splitlines()[:24] if row.strip()]
        return bool(rows) and rows[-1] == drive + '>'

    results = []
    cases = [
        ('q:', 'A', 'Command error: Invalid drive (use A: through P:)'),
        ('1:', 'A', 'Command error: Invalid drive (use A: through P:)'),
        ('p:', 'A', 'Drive unavailable: P:'),
        ('p:missing', 'A', 'Drive unavailable: P:'),
        ('p:missing.cmd', 'A', 'Drive unavailable: P:'),
        ('a :', 'A', None),
        ('b:', 'B', None),
        ('p:', 'B', 'Drive unavailable: P:'),
        ('p:missing.com', 'B', 'Drive unavailable: P:'),
        ('a:', 'A', None),
        ('b :', 'B', None),
        ('  a :', 'A', None),
        ('a :', 'A', None),
    ]
    current = 'A'
    for index, (command, drive, marker) in enumerate(cases):
        type_command('cls\n')
        time.sleep(.4)
        wait(lambda text: prompt(text, current))
        type_command(command + '\n')
        time.sleep(.4)
        text = wait(lambda text: prompt(text, drive))
        (output / f'drive-{index:02}.txt').write_text(text)
        rows = [row.strip() for row in text.splitlines()[:24] if row.strip()]
        if marker:
            assert rows[-2:] == [marker, drive + '>'], text
        else:
            assert len(rows) == 2, text
            assert re.fullmatch(r'[AB]> *[ab] ?: ?', rows[0]), text
        results.append('PASS: ' + command + ' -> ' + drive + '>')
        current = drive
    return results
