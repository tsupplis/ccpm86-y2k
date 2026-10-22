"""Build isolated, distinguishable CMD/SUB/DOS search fixtures."""
from pathlib import Path
import shutil


def fixtures(output, root, base, run):
    work = output / 'search-fixtures'
    work.mkdir()
    media = root / 'media'
    def cmd(args):
        return run(args, media)
    for letter in ('A', 'B'):
        source = work / ('n' + letter.lower() + '.a86')
        source.write_bytes(('''cseg
org 0
    mov cl,9 ! mov dx,offset message ! int 0e0h
    mov cl,25 ! int 0e0h
    mov dl,al ! add dl,'A' ! mov cl,2 ! int 0e0h
    mov cl,9 ! mov dx,offset ending ! int 0e0h
    mov cl,0 ! int 0e0h
dseg
org 100h
message db 'SEARCH CMD %s DEFAULT $'
ending db 13,10,'$'
end
''' % letter).replace('\n', '\r\n').encode())
        run(['cpm86_asm86', source.name], work, work / (letter + '-asm.log'))
        run(['cpm86_gencmd', source.with_suffix('.h86').name], work)
        source = work / ('d' + letter.lower() + '.asm')
        source.write_text('''bits 16
cpu 8086
org 100h
    push cs
    pop ds
    mov dx,message
    mov ah,9
    int 21h
    mov ah,19h
    int 21h
    mov dl,al
    add dl,'A'
    mov ah,2
    int 21h
    mov dx,ending
    mov ah,9
    int 21h
    int 20h
message db 'SEARCH COM %s DEFAULT $'
ending db 13,10,'$'
''' % letter)
        run(['nasm', '-f', 'bin', source.name, '-o', 'd' + letter.lower() + '.com'], work)
    shutil.copy2(base / 'sdapi.a86', work / 'sdapi.a86')
    run(['cpm86_asm86', 'sdapi.a86'], work, work / 'sdapi-build.log')
    run(['cpm86_gencmd', 'sdapi.h86'], work)
    for letter in ('A', 'B'):
        logical = work / (letter.lower() + '-logical.img')
        shutil.copy2(output / ('logical.img' if letter == 'A' else 'equipment-logical.img'), logical)
        if letter == 'A':
            # Free generated fixture entries only in this disposable search copy.
            cmd(['cpmrm', '-f', 'ccpm86-320', str(logical), '0:pc*.exe', '0:pc23*.com'])
        native = work / ('n' + letter.lower() + '.cmd')
        dos = work / ('d' + letter.lower() + '.com')
        for name in ['native', 'sysfall'] + (['pick'] if letter == 'B' else ['batch']):
            cmd(['cpmcp', '-f', 'ccpm86-320', str(logical), str(native), '0:' + name + '.cmd'])
        for name in ['pick', 'native', 'scriptd'] + (['dosonly', 'badn'] if letter == 'B' else []):
            cmd(['cpmcp', '-f', 'ccpm86-320', str(logical), str(dos), '0:' + name + '.com'])
        if letter == 'A':
            cmd(['cpmcp', '-f', 'ccpm86-320', str(logical), str(work / 'sdapi.cmd'), '0:sdapi.cmd'])
            cmd(['cpmchattr', '-f', 'ccpm86-320', str(logical), 's', '0:batch.cmd'])
        else:
            for name in ('pick', 'exeonly'):
                cmd(['cpmcp', '-f', 'ccpm86-320', str(logical), str(output / 'pcexe.exe'), '0:' + name + '.exe'])
            empty = work / 'empty.cmd'
            empty.write_bytes(b'')
            cmd(['cpmcp', '-f', 'ccpm86-320', str(logical), str(empty), '0:badn.cmd'])
        scripts = {'onlysub': 'ECHO SEARCH SUB ' + letter + ' $1 $2',
                   'scriptd': 'ECHO SEARCH SUB ' + letter,
                   'badn': 'ECHO WRONG BAD NATIVE FALLBACK',
                   'sysbatch': 'ECHO SEARCH SYS SUB ' + letter}
        if letter == 'B':
            scripts['pick'] = 'ECHO WRONG SUB SHADOW'
        for name, text in scripts.items():
            source = work / (name + '.sub')
            source.write_bytes(text.encode() + b'\r\n\x1a')
            cmd(['cpmcp', '-f', 'ccpm86-320', str(logical), str(source), '0:' + name + '.sub'])
        source = work / 'batonly.bat'
        source.write_bytes(b'REM route fixture\r\n\x1a')
        cmd(['cpmcp', '-f', 'ccpm86-320', str(logical), str(source), '0:batonly.bat'])
        cmd(['cpmchattr', '-f', 'ccpm86-320', str(logical), 's', '0:sysfall.cmd', '0:sysbatch.sub'])
        run(['python3', str(root / 'tools/cpm86twist.py'), 'twist', str(logical),
             str(output / ('pc-search' + ('b' if letter == 'B' else '') + '.img'))], root)
    # Build native TMP from matching sources; repository binaries may be older.
    native = work / 'native-rsp'
    native.mkdir()
    for name in ('tmp.a86', 'Makefile'):
        shutil.copy2(root / 'kernel/rsp1' / name, native / name)
    log = run(['make', 'tmp.rsp'], native, work / 'native-tmp-build.log')
    if b'NUMBER OF ERRORS:   0' not in log:
        raise RuntimeError('inspect native-tmp-build.log')
    # The same new SUP/SETDEF policy with the native TMP personality.
    gen = work / 'native-gen'
    gen.mkdir()
    for source in (output / 'sysgen').iterdir():
        if source.suffix in ('.con', '.rsp') and source.name != 'pctmp.rsp':
            shutil.copy2(source, gen / source.name)
    shutil.copy2(native / 'tmp.rsp', gen / 'tmp.rsp')
    data = (output / 'sysgen/genccpm.in').read_bytes()
    run(['emu2', str(root / 'commands/genccpm/genccpm.cmd')], gen, work / 'native-gen.log', data)
    native_disk = work / 'native-logical.img'
    shutil.copy2(work / 'a-logical.img', native_disk)
    cmd(['cpmrm', '-f', 'ccpm86-320', str(native_disk), '0:ccpm.sys'])
    cmd(['cpmcp', '-f', 'ccpm86-320', str(native_disk), str(gen / 'ccpm.sys'), '0:'])
    run(['python3', str(root / 'tools/cpm86twist.py'), 'twist', str(native_disk),
         str(output / 'pc-native.img')], root)
