"""Verify the four SETDEF policies and drive/type precedence on guest disks."""
import re
import time


def search_checks(hmp, wait, type_command, prompt, output, native=False):
    results = []
    def run(text, expected, name):
        type_command('cls\n')
        wait(prompt)
        type_command(text + '\n')
        time.sleep(.5)
        result = wait(prompt)
        (output / (name + '.txt')).write_text(result)
        if isinstance(expected, tuple):
            order = re.search(r'^Search Order +-[ ]+([A-Z, ]+?)\s*$', result, re.M)
            ok = order and order.group(1).replace(' ', '') == expected[0]
        else:
            ok = expected in result
        if not ok or 'WRONG' in result:
            raise AssertionError(result)
        results.append('PASS: ' + name)
        return result
    def settings(order, path='b:,a:'):
        run('a:setdef ' + path + ' [order=(' + order + ')]',
            (order.upper(),), 'order-' + order.replace(',', '-') + '-' + path.replace(':', '').replace(',', '-'))
    run('a:setdef', ('CMD,DOS',), 'default-order')
    run('a:sdapi', 'SEARCH API COMPAT OK', 'api-v1-v2-v3-compatibility')
    run('a:setdef', ('CMD,DOS',), 'api-restores-policy')
    for order in ('cmd', 'cmd,sub', 'cmd,dos', 'cmd,sub,dos'):
        settings(order)
        run('native', 'SEARCH CMD B DEFAULT A', 'native-path-' + order.replace(',', '-'))
        if 'sub' in order:
            run('onlysub red blue', 'SEARCH SUB B red blue'.upper(),
                'sub-enabled-' + order.replace(',', '-'))
        else:
            run('onlysub red blue', 'Command not found',
                'sub-disabled-' + order.replace(',', '-'))
        # Explicit SUB bypasses ORDER and still uses the configured drive path.
        run('onlysub.sub red blue', 'SEARCH SUB B red blue'.upper(),
            'explicit-sub-' + order.replace(',', '-'))
        if not native:
            if 'dos' in order:
                run('dosonly', 'SEARCH COM B DEFAULT A', 'dos-enabled-' + order.replace(',', '-'))
            else:
                run('dosonly', 'Command not found', 'dos-disabled-' + order.replace(',', '-'))
            run('b:dosonly.com', 'SEARCH COM B DEFAULT A', 'explicit-dos-' + order.replace(',', '-'))
    settings('cmd,sub,dos')
    run('pick', 'SEARCH CMD B DEFAULT A', 'cmd-before-sub-and-dos-on-one-drive')
    if not native:
        run('a:pick', 'SEARCH COM A DEFAULT A', 'explicit-drive-overrides-path')
    if not native:
        settings('cmd,sub,dos', 'a:,b:')
        run('pick', 'SEARCH COM A DEFAULT A', 'earlier-drive-dos-before-later-drive-cmd')
        settings('cmd,sub,dos')
        run('scriptd', 'SEARCH SUB B', 'earlier-drive-sub-before-later-drive-dos')
        settings('cmd,dos')
        run('scriptd', 'SEARCH COM B DEFAULT A', 'sub-disabled-selects-com-same-drive')
        run('b:pick.exe token', 'PCMODE EXE TEST OK', 'explicit-exe-overrides-com')
        run('exeonly token', 'PCMODE EXE TEST OK', 'untyped-exe-from-path')
        run('b:batonly.bat', 'SEARCH CMD A DEFAULT A', 'explicit-bat-keeps-batch-route')
        run('batonly', 'SEARCH CMD A DEFAULT A', 'untyped-bat-keeps-batch-route')
        result = run('badn', 'Invalid or empty CMD file', 'bad-native-does-not-fall-through-to-sub-or-com')
        if 'SEARCH COM' in result:
            raise AssertionError(result)
    # Skip an unavailable configured drive, deduplicate paths, then SYS fallback.
    settings('cmd,sub,dos', 'c:,b:,b:,*')
    run('native', 'SEARCH CMD B DEFAULT A', 'unavailable-and-duplicate-drives')
    settings('cmd,sub,dos', 'c:')
    run('sysfall', 'SEARCH CMD A DEFAULT A', 'system-cmd-fallback')
    run('sysbatch', 'SEARCH SYS SUB A', 'system-sub-fallback')
    run('sysdisk b:', 'System disk is B:', 'change-system-disk')
    run('sysfall', 'SEARCH CMD B DEFAULT A', 'live-system-disk-fallback')
    run('sysdisk a:', 'System disk is A:', 'restore-system-disk')
    settings('cmd,sub,dos')
    for bad in ('(dos)', '(sub,cmd)', '(cmd,dos,sub)', '(cmd,sub,sub)', '(cmd,exe)', '(cmd,dos,dos)'):
        run('a:setdef [order=' + bad + ']', 'Usage: SETDEF', 'reject-' + bad.replace(',', '-'))
        run('a:setdef', ('CMD,SUB,DOS',), 'preserve-after-' + bad.replace(',', '-'))
    # Settings are per console. Console 1 starts with the default, independently.
    settings('cmd')
    hmp('sendkey ctrl-alt-2 100')
    wait(lambda s: 'Console=1' in s and prompt(s))
    run('a:setdef', ('CMD,DOS',), 'console1-default')
    settings('cmd,sub,dos')
    run('onlysub red blue', 'SEARCH SUB B red blue'.upper(), 'console1-sub')
    hmp('sendkey ctrl-alt-1 100')
    wait(lambda s: 'Console=0' in s and prompt(s))
    run('a:setdef', ('CMD',), 'console0-policy-isolated')
    settings('cmd,dos', '*')
    if not native:
        run('pcprobe', 'PCMODE DOS PROBE OK', 'dos-recovery-after-policy-reset')
    from pchelpcheck import setdef_help
    results.extend(setdef_help(hmp, wait, type_command, prompt, output))
    return results
