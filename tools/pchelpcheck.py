"""Quick-help and topic-tail checks on disposable PC-MODE snapshots."""
import re
import time


def setdef_help(hmp, wait, type_command, prompt, output):
    """Read the indexed SETDEF topic, including pages above the final screen."""
    type_command('sysdisk b:\n')
    wait(prompt)
    type_command('cls\n')
    wait(prompt)
    type_command('? setdef\n')
    pages = []
    for index in range(12):
        def ready(text):
            rows = [line.strip() for line in text.splitlines()[:24] if line.strip()]
            return bool(rows) and (rows[-1] == 'HELP>' or
                                   rows[-1] == 'Press RETURN to continue')
        text = wait(ready)
        pages.append(text)
        (output / f'setdef-help-{index}.txt').write_text(text)
        rows = [line.strip() for line in text.splitlines()[:24] if line.strip()]
        type_command('\n')
        time.sleep(.3)
        if rows[-1] == 'HELP>':
            wait(prompt)
            break
    else:
        raise AssertionError('SETDEF help did not finish')
    topic = '\n'.join(pages)
    for marker in ('CMD only;', 'CMD then SUB;', 'CMD then COM, EXE, BAT',
                   'CMD then SUB, COM, EXE, BAT', 'Native TMP ignores DOS'):
        if marker not in topic:
            raise AssertionError('Missing indexed SETDEF help: ' + marker)
    type_command('sysdisk a:\n')
    wait(prompt)
    return ['PASS: indexed SETDEF help lists four orders and native TMP DOS rule']


def help_commands(hmp, screen, wait, type_command, prompt, output, tail_fixture=False):
    results = []

    def run(text, name):
        type_command('cls\n')
        wait(prompt)
        type_command(text + '\n')
        time.sleep(.5)
        result = wait(prompt)
        (output / (name + '.txt')).write_text(result)
        return result

    for index, text in enumerate(['?', '?   ', '?\t', '   ?  \t  ']):
        result = run(text, 'help-quick-' + str(index))
        if ('Common commands:' not in result or
                'Shell commands: CLS  SYSDISK  USER  MEM  PRINTER' not in result or
                '? topic opens HELP' not in result or "Can't Find" in result):
            raise AssertionError(result)
        results.append('PASS: quick help whitespace case ' + str(index))

    result = run('? xyzmissing', 'help-missing')
    if "Command error: Command not found. Use ? for help" not in result or not re.search(r'^Command = HELP\s*$', result, re.M):
        raise AssertionError(result)
    results.append('PASS: missing HELP uses the normal command error for HELP')

    for text in ['?name', '?.com', 'a:?', '?xyz.cmd']:
        result = run(text, 'help-lookup-' + str(len(results)))
        if 'Common commands:' in result or '? topic opens HELP' in result:
            raise AssertionError(result)
        results.append('PASS: ordinary filename lookup preserved for ' + text)

    # No helper disk is searched until SYSDISK B: selects the system fallback.
    result = run('sysdisk b:', 'help-system-b')
    if 'System disk is B:' not in result:
        raise AssertionError(result)
    if tail_fixture:
        for index, tail in enumerate([' pip', '\tMiXeD [n]  topic',
                                      '    pip   examples  ', ' ' + 'x' * 122,
                                      ' ' + 'x' * 123]):
            direct = run('help' + tail, 'help-direct-' + str(index))
            expanded = run('?' + tail, 'help-expanded-' + str(index))
            def words(text):
                if 'PCMODE HELP TAIL OK' not in text:
                    raise AssertionError(text)
                return ''.join(re.findall(r'^TAIL=([0-9A-F]*)\s*$', text, re.M))
            if not words(direct) or words(direct) != words(expanded):
                raise AssertionError('HELP topic tail changed:\n' + expanded)
            results.append('PASS: exact HELP tail matches direct invocation, case ' + str(index))
        for size in (126, 128):
            result = run('?' + ' ' + 'x' * (size - 2), 'help-overlong-' + str(size))
            if 'Help request too long (max 125 characters)' not in result:
                raise AssertionError(result)
            results.append('PASS: reject help input length ' + str(size))
    else:
        # Native HELP is interactive. [N] disables paging, then a blank HELP>
        # line exits. Its own fallback finds HELP.HLP on the selected system disk.
        type_command('cls\n')
        wait(prompt)
        type_command('? pip [n]\n')
        result = wait(lambda s: 'HELP>' in s and 'PIP' in s)
        if 'Syntax:' not in result and 'PIP' not in result:
            raise AssertionError(result)
        (output / 'help-native-pip.txt').write_text(result)
        type_command('\n')
        wait(prompt)
        results.append('PASS: native HELP.CMD displays PIP topic from system-disk HELP.HLP')

    result = run('sysdisk a:', 'help-system-restore')
    if 'System disk is A:' not in result:
        raise AssertionError(result)
    result = run('pcprobe', 'help-dos-recovery')
    if 'PCMODE DOS PROBE OK' not in result:
        raise AssertionError(result)
    results.append('PASS: help returns to prompt and DOS execution')
    return results


def native_help(hmp, wait, type_command, prompt, output):
    """Compare native TMP help boundaries without changing its source/media."""
    results = []
    # The native search fixture keeps HELP.CMD/HLP on the B: companion.
    type_command('sysdisk b:\n')
    wait(prompt)
    for index, text in enumerate(['?', '?   ', '?\t', '   ?  \t  ']):
        type_command('cls\n')
        wait(prompt)
        type_command(text + '\n')
        result = wait(lambda s: prompt(s) and 'Common commands:' in s)
        if '? topic opens HELP' not in result:
            raise AssertionError(result)
        (output / f'native-quick-{index}.txt').write_text(result)
        results.append('PASS: native TMP quick-help whitespace case ' + str(index))
    for index, text in enumerate(['? pip [n]', '? pip [n]'.ljust(125)]):
        type_command('cls\n')
        wait(prompt)
        type_command(text + '\n')
        result = wait(lambda s: 'HELP>' in s and 'PIP' in s)
        (output / f'native-topic-{index}.txt').write_text(result)
        type_command('\n')
        wait(prompt)
        results.append('PASS: native TMP topic expansion, length ' + str(len(text)))
    for size in (126, 128):
        type_command('cls\n')
        wait(prompt)
        type_command('? ' + 'x' * (size - 2) + '\n')
        result = wait(lambda s: prompt(s) and 'Help request too long' in s)
        (output / f'native-overlong-{size}.txt').write_text(result)
        results.append('PASS: native TMP rejects help input length ' + str(size))
    type_command('sysdisk a:\n')
    wait(prompt)
    return results
