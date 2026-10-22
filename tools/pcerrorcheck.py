"""Exercise actual parser/CLI/loader errors and prompt recovery in snapshots."""
import time


def diagnostic_checks(wait, type_command, prompt, output):
    cases = [(0, 'Unknown command error, code 0'),
             (12345, 'Unknown command error, code 12345'),
             (65535, 'Unknown command error, code 65535'),
             (15, 'Command queue full; retry'),
             (18, 'Memory allocation table is full'),
             (17, 'Command requires an 8087 coprocessor'),
             (33, 'Invalid CMD file: missing code group'),
             (41, 'Invalid CMD relocation data'),
             (37, 'Invalid printer/list device'),
             (44, 'SUBMIT.CMD not found'),
             (43, 'Command tail too long for SUBMIT'),
             (61, 'BATCH.CMD not found'), (31, None)]
    results = []
    for code, message in cases:
        type_command('cls\n')
        time.sleep(.4)
        wait(prompt)
        type_command('b:errdiag ' + str(code) + '\n')
        time.sleep(.5)
        text = wait(prompt)
        (output / f'diagnostic-{code}.txt').write_text(text)
        assert 'ERROR DIAGNOSTIC TEST OK' in text, text
        if message:
            assert 'Command error: ' + message in text, text
            assert 'Command = ERROR DIAGNOSTIC' in text, text
        else:
            assert 'Command error:' not in text, text
            assert 'Command = ' not in text, text
        results.append(f'PASS: helper diagnostic {code}')
    return results


def error_checks(wait, type_command, prompt, output, native=False):
    cases = [
        ('abcdefghi', 'Invalid name (max 8 characters)'),
        ('foo.abcd', 'Invalid extension (max 3 characters)'),
        ('foo;abcdefghi', 'Invalid password (max 8 characters)'),
        ('q:foo', 'Invalid drive (use A: through P:)'),
        ('missing', 'Command not found. Use ? for help'),
        ('b:missing.cmd', 'Command not found. Use ? for help'),
        ('b:badn.cmd', 'Invalid or empty CMD file'),
    ]
    if not native:
        cases += [
            ('foo.zzz', 'Unsupported file type'),
            ('pcshort.exe', 'Invalid or unreadable executable file'),
            ('pcnomem.exe', 'Not enough memory'),
        ]
    results = []
    for index, (command, message) in enumerate(cases):
        type_command('cls\n')
        time.sleep(.4)
        wait(prompt)
        type_command(command + '\n')
        time.sleep(.5)
        text = wait(prompt)
        (output / f'error-{index:02}.txt').write_text(text)
        rows = [row.strip() for row in text.splitlines()[:24] if row.strip()]
        assert rows[-3:] == ['Command error: ' + message, 'Command = ' + command, 'A>'], text
        results.append('PASS: ' + command + ' diagnostic and command context')
    type_command(('b:native.cmd' if native else 'pcprobe') + '\n')
    time.sleep(.5)
    text = wait(prompt)
    assert ('SEARCH CMD B DEFAULT A' if native else 'PCMODE DOS PROBE OK') in text, text
    (output / 'error-recovery.txt').write_text(text)
    results.append('PASS: command execution recovers after errors')
    return results
