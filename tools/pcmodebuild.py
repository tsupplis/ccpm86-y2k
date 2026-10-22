#!/usr/bin/env python3
"""Build the opt-in PC-MODE personality in a fresh, isolated output directory.

DOS FAT support remains disabled: its 15 external filesystem routines are
missing. The optional audit records that link failure rather than installing it.
"""
import argparse
import hashlib
import json
from pathlib import Path
import shutil
import subprocess

ROOT = Path(__file__).resolve().parent.parent
BASE = ROOT / 'kernel/pcmode'
SOURCE = ROOT / 'kernel/rsp3'
PROBES = BASE / 'tests/probes'


def run(args, cwd, log=None, data=None):
    result = subprocess.run(args, cwd=cwd, input=data, stdout=subprocess.PIPE,
                            stderr=subprocess.STDOUT, timeout=120)
    if log:
        log.write_bytes(result.stdout)
    result.check_returncode()
    return result.stdout


def record_build(output, enhanced_keyboard, extras):
    """Pin source and result bytes so later validation can identify its build."""
    def describe(path):
        data = path.read_bytes()
        return {'bytes': len(data), 'sha256': hashlib.sha256(data).hexdigest()}
    sources = sorted(path for folder in (BASE, SOURCE) for path in folder.rglob('*') if path.is_file())
    sources += sorted((ROOT / 'tools').glob('pc*.py'))
    sources += [ROOT / 'tools/sysmodules.py', ROOT / 'commands/setdef/setdef.a86',
                ROOT / 'commands/help/help.dat', ROOT / 'kernel/rsp1/tmp.a86',
                ROOT / 'kernel/rsp1/Makefile']
    sources += sorted(path for path in (output / 'con').iterdir()
                      if path.suffix.lower() in ('.a86', '.lib', '.bdo', '.def',
                         '.dat', '.fmt', '.sup', '.rtm', '.mem', '.cio', '.cic', '.cod'))
    record = {
        'personality': 'PC-MODE', 'consoles': 4,
        'enhanced_keyboard': enhanced_keyboard, 'optional_extras': bool(extras),
        'sources': {str(path.relative_to(ROOT) if path.is_relative_to(ROOT)
                        else path.relative_to(output)): describe(path) for path in sources},
        'outputs': {str(path.relative_to(output)): describe(path)
                    for path in sorted(output.glob('*.img'))},
        'con_inputs': {path.name: describe(path) for path in sorted((output / 'sysgen').glob('*.con'))},
        'rsp_inputs': {path.name: describe(path) for path in sorted((output / 'sysgen').glob('*.rsp'))},
        'optional_inputs': {name: describe(extras / name) for name in (
            'msmasm.exe', 'link.exe', 'exe2bin.exe', 'hellomas.asm', 'hellodos.asm',
            'ballc.com', 'ballc.cmd', 'sine.cmd') if extras and (extras / name).is_file()},
    }
    (output / 'build-manifest.json').write_text(json.dumps(record, indent=2) + '\n')


def build(output, audit, enhanced_keyboard=True, extras=None):
    from pcmodeverify import verify
    verify()
    output.mkdir(parents=True, exist_ok=False)
    setdef = output / 'setdef'
    setdef.mkdir()
    for name in ('setdef.a86', 'Makefile'):
        shutil.copy2(ROOT / 'commands/setdef' / name, setdef / name)
    run(['make', 'all'], setdef, output / 'setdef-build.log')
    con = output / 'con'
    con.mkdir()
    for path in (ROOT / 'kernel/con').iterdir():
        if path.suffix.lower() in ('.a86', '.lib', '.bdo', '.def', '.dat', '.fmt', '.sup', '.rtm', '.mem', '.cio', '.cic', '.cod') or path.name == 'Makefile':
            shutil.copy2(path, con / path.name)
    if audit:
        path = con / 'bdos.a86'
        original = path.read_bytes()
        assert b'PCMODE\tequ\tfalse' in original
        path.write_bytes(original.replace(b'PCMODE\tequ\tfalse', b'PCMODE\tequ\ttrue'))
        log = run(['make', 'bdos.con'], con, output / 'flag-audit.log')
        assert b'Undefined' in log, 'PCMODE dependencies changed; reassess before continuing'
        path.write_bytes(original)
        for name in ('bdos.obj', 'bdos.con'):
            (con / name).unlink(missing_ok=True)
    # Select the tracked XIOS conditionals; never rewrite hardware routines.
    path = con / 'pcmode.lib'
    config = path.read_bytes()
    assert config.count(b'PCMODE_XIOS equ 0\r\n') == 1
    config = config.replace(b'PCMODE_XIOS equ 0\r\n', b'PCMODE_XIOS equ 0ffffh\r\n')
    if not enhanced_keyboard:
        config = config.replace(b'PCMODE_ENHANCED_KEYS equ 0ffffh', b'PCMODE_ENHANCED_KEYS equ 0')
    path.write_bytes(config)
    log = run(['make', 'all'], con, output / 'build.log')
    if b'Undefined' in log or b'Number of errors:   0' not in log:
        raise RuntimeError('inspect build.log before using this build')
    gen = output / 'sysgen'
    gen.mkdir()
    for path in con.glob('*.con'):
        shutil.copy2(path, gen / path.name)
    for folder, names in [('rsp1', ('pin', 'vout', 'serin', 'serout')),
                          ('rsp2', ('abort', 'clock', 'dir', 'echo'))]:
        for name in names:
            shutil.copy2(ROOT / 'kernel' / folder / (name + '.rsp'), gen / (name + '.rsp'))
    # Preserve the recovered original and build the explicitly selected safe
    # loader variant. Replace only CODE; DATA layout/console stride is unchanged.
    rsp = output / 'rsp3'
    rsp.mkdir()
    for path in SOURCE.iterdir():
        if path.suffix.lower() in ('.a86', '.lib') or path.name == 'Makefile':
            shutil.copy2(path, rsp / path.name)
    run(['make', 'all'], rsp, output / 'pctmp-build.log')
    from pctmpcheck import check as check_pctmp
    check_pctmp(rsp / 'pctmp.rsp')
    shutil.copy2(rsp / 'pctmp.rsp', gen / 'pctmp.rsp')
    data = (ROOT / 'kernel/sysgen/genccpm.in').read_bytes().replace(
        b'CP/M-Plus Personality', b'PCMODE text-console experiment')
    # Recovered PCTMP and the experimental bridge need more than the native
    # 8008h placement leaves below VGA memory. GENCCPM also allocates console
    # images/map after the SYS payload. Keep the entire allocation below A0000h.
    assert b'osstart=8008' in data
    data = data.replace(b'osstart=8008', b'osstart=7008')
    (gen / 'genccpm.in').write_bytes(data)
    run(['emu2', str(ROOT / 'commands/genccpm/genccpm.cmd')], gen, output / 'genccpm.log', data)
    run(['python3', str(ROOT / 'tools/sysmodules.py'), str(gen / 'ccpm.sys'), str(output / 'map')], ROOT, output / 'verify.log')
    assert (output / 'map/recovered/pctmp.rsp').read_bytes() == (gen / 'pctmp.rsp').read_bytes()
    assert json.loads((output / 'map/manifest.json').read_text())['consoles'] == 4
    xios = (output / 'map/raw/core/xios.bin').read_bytes()
    buffer_segment = int.from_bytes(xios[0x38:0x3a], 'little')
    # XIOS uses four 24x80 word images and one byte ownership map.
    buffer_end = buffer_segment * 16 + 24 * 80 * (4 * 2 + 1)
    if buffer_end > 0xa0000:
        raise RuntimeError(f'console buffers overlap video memory: end={buffer_end:05x}')
    (output / 'memory-layout.txt').write_text(
        f'Console buffers: {buffer_segment * 16:05X}..{buffer_end - 1:05X}\n'
        'Video aperture starts at A0000; production osstart is unchanged.\n')
    for name in ('pcprobe', 'pcfile', 'pcvideo', 'pckey', 'pcstate', 'pcpixel', 'pcbreak', 'pcrecov', 'pcequip', 'pcread', 'pcbusy', 'pckword', 'pchecho'):
        run(['nasm', '-f', 'bin', str(PROBES / (name + '.asm')), '-o', str(output / (name + '.com'))], SOURCE)
    for name, value, service in [('pc23exit', 1, '0100h'), ('pc23ret', 0, '0100h'),
                                 ('pc23in8', 0, '0800h'), ('pc23line', 0, '0a00h'),
                                 ('pc23fl1', 0, '0c01h'), ('pc23fl8', 0, '0c08h'),
                                 ('pc23fla', 0, '0c0ah')]:
        run(['nasm', '-f', 'bin', '-DEXIT_CASE=' + str(value), '-DINPUT_CALL=' + service,
             str(PROBES / 'pcint23.asm'), '-o', str(output / (name + '.com'))], SOURCE)
    run(['nasm', '-f', 'bin', str(PROBES / 'pcexe.asm'), '-o', str(output / 'pcexe.exe')], SOURCE)
    run(['nasm', '-f', 'bin', str(PROBES / 'pcshort.asm'), '-o', str(output / 'pcshort.exe')], SOURCE)
    # A complete, otherwise valid EXE whose minimum allocation cannot fit.
    no_memory = bytearray((output / 'pcexe.exe').read_bytes())
    no_memory[10:12] = (0xf000).to_bytes(2, 'little')
    (output / 'pcnomem.exe').write_bytes(no_memory)
    (output / 'pcempty.exe').write_bytes(b'')
    run(['nasm', '-f', 'bin', str(PROBES / 'pcround.asm'), '-o', str(output / 'pcround.exe')], SOURCE)
    # Compact copies exercise all paragraph positions inside a record and
    # later header records. Each retains the final-word relocation test.
    original_round = (output / 'pcround.exe').read_bytes()
    for name, size in [('pch128', 128), ('pch256', 256), ('pch384', 384), ('pch32', 32), ('pch48', 48), ('pch64', 64), ('pch80', 80), ('pch96', 96), ('pch112', 112), ('pch144', 144), ('pch496', 496), ('pch528', 528)]:
        compact = bytearray(original_round[:32] + bytes(size - 32) + original_round[512:])
        compact[2:4] = (len(compact) % 512).to_bytes(2, 'little')
        compact[4:6] = ((len(compact) + 511) // 512).to_bytes(2, 'little')
        compact[8:10] = (size // 16).to_bytes(2, 'little')
        (output / (name + '.exe')).write_bytes(compact)
    run(['nasm', '-f', 'bin', str(PROBES / 'pclarge.asm'), '-o', str(output / 'pclarge.exe')], SOURCE)
    # Table-byte alignment, later records, final-record boundary and a table
    # that crosses a 64 KiB file offset. Store these on the companion floppy.
    for name, offset, count in [('pcrlate',128,1), ('pcrpost',256,1),
                               ('pcr124',124,1), ('pcr125',125,1),
                               ('pcr126',126,1), ('pcr127',127,1),
                               ('pcr129',129,1), ('pcr255',255,1),
                               ('pcrmany',125,70), ('pcrbig',125,16385),
                               ('pcrnone',65535,0)]:
        run(['nasm','-f','bin','-DTABLE_AT=' + str(offset), '-DFIXUPS=' + str(count), '-DSHARED_TARGET=' + str(int(name == 'pcrbig')),
             str(PROBES / 'pcfix.asm'), '-o', str(output / (name + '.exe'))], SOURCE)
    # Exact record/page lengths and odd-byte image tails. This set has its
    # own data floppy, keeping room for the original DOS tools and outputs.
    for name, header, image in [('pcs128',32,128), ('pcs129',48,129),
                                ('pcs255',48,255), ('pcs256',32,256),
                                ('pcs257',48,257), ('pcs512',32,480),
                                ('pcs1k',48,976), ('pcs384',48,336)]:
        run(['nasm','-f','bin','-DHEADER_BYTES=' + str(header),
             '-DIMAGE_BYTES=' + str(image), str(PROBES / 'pcsize.asm'),
             '-o', str(output / (name + '.exe'))], SOURCE)
    boundary_base = (output / 'pcs512.exe').read_bytes()
    for name, offset, value in [('pcpgzero',4,0), ('pcpgbig',4,0x4000),
                                ('pcbl512',2,512), ('pcminhdr',8,1),
                                ('pcsig',0,0x4d5a), ('pcipend',20,480),
                                ('pcfxend',28,479)]:
        invalid = bytearray(boundary_base)
        invalid[offset:offset + 2] = value.to_bytes(2, 'little')
        (output / (name + '.exe')).write_bytes(invalid)
    empty_image = bytearray(boundary_base[:32])
    empty_image[2:4] = (32).to_bytes(2, 'little')
    empty_image[6:8] = bytes(2)
    (output / 'pcnone.exe').write_bytes(empty_image)
    # Positive initial-stack conventions and unsafe launch frames. Negative
    # variants run only with the guard, before any child stack writes occur.
    for name, defines in [
            ('pcstbss', []),
            ('pcstzero', ['STACK_SEG=10h', 'STACK_TOP=0', 'EXTRA_MIN=1100h']),
            ('pcstmax', ['STACK_SEG=20h', 'EXTRA_MAX=80h']),
            ('pcsswrap', ['STACK_SEG=0ffffh']),
            ('pcssout', ['STACK_SEG=0f000h']),
            ('pcstend', ['STACK_SEG=0', 'STACK_TOP=0ffffh', 'EXTRA_MAX=0'])]:
        run(['nasm', '-f', 'bin', *['-D' + d for d in defines],
             str(PROBES / 'pcstack.asm'), '-o', str(output / (name + '.exe'))], SOURCE)
    # Invalid targets/headers are generated from the known-valid small EXE.
    # They are never run against an unguarded kernel: a bad fixup could write
    # outside the process. The old-path baseline uses only the safe ROUND case.
    for name, offset, value in [('pcfixbad', 0x1e, 0xffff),
                                ('pchdrbad', 8, 0xffff),
                                ('pcovflow', 10, 0xffff),
                                ('pcentbad', 0x16, 0xffff),
                                ('pctblbad', 6, 0xffff)]:
        invalid = bytearray((output / 'pcexe.exe').read_bytes())
        invalid[offset:offset + 2] = value.to_bytes(2, 'little')
        (output / (name + '.exe')).write_bytes(invalid)

    (output / 'pctest.sub').write_bytes(b'ECHO PCMODE SUB TEST OK\r\n\x1a')
    logical = output / 'logical.img'
    run(['python3', str(ROOT / 'tools/cpm86twist.py'), 'untwist', str(ROOT / 'media/ccpm86base.img'), str(logical)], ROOT)
    run(['cpmcp', '-f', 'ccpm86-320', str(logical), str(gen / 'ccpm.sys'),
         str(output / 'pcprobe.com'), str(output / 'pcfile.com'), str(output / 'pcvideo.com'),
         str(output / 'pckey.com'), str(output / 'pcstate.com'), str(output / 'pcpixel.com'),
         *[str(output / (n + '.com')) for n in ('pc23exit', 'pc23ret', 'pc23in8', 'pc23line', 'pc23fl1', 'pc23fl8', 'pc23fla')],
         str(output / 'pcexe.exe'), str(output / 'pcshort.exe'),
         str(output / 'pcnomem.exe'), str(output / 'pcempty.exe'), str(output / 'pcbreak.com'),
         *[str(output / (name + '.exe')) for name in ('pcstbss', 'pcstzero', 'pcstmax', 'pcsswrap', 'pcssout', 'pcstend', 'pch128', 'pch256', 'pch384', 'pch48', 'pcround', 'pcfixbad', 'pchdrbad', 'pcovflow', 'pcentbad', 'pctblbad')],
         str(ROOT / 'commands/type/type.cmd'),
         str(setdef / 'setdef.cmd'),
         str(ROOT / 'commands/submit/submit.cmd'), str(output / 'pctest.sub'), '0:'], ROOT / 'media')
    # User-supplied integration demos retain their original names and bytes.
    demos = [extras / name for name in ('ballc.com', 'ballc.cmd', 'sine.cmd')] if extras else []
    if demos and all(path.exists() for path in demos):
        run(['cpmcp', '-f', 'ccpm86-320', str(logical),
             *map(str, demos), '0:'], ROOT / 'media')
    run(['cpmchattr', '-f', 'ccpm86-320', str(logical), 's', '0:setdef.cmd', '0:submit.cmd'], ROOT / 'media')
    run(['python3', str(ROOT / 'tools/cpm86twist.py'), 'twist', str(logical), str(output / 'pc-text.img')], ROOT)
    # Optional original DOS toolchain lives on a separate data floppy so
    # the boot disk retains space for its kernel and regression fixtures.
    tool_names = ('msmasm.exe', 'link.exe', 'exe2bin.exe', 'hellomas.asm', 'hellodos.asm')
    if extras and all((extras / name).exists() for name in tool_names):
        data_disk = output / 'tools-logical.img'
        # Retain the known 320K format-identification/boot track; clear every
        # filesystem record. An all-E5 reserved track is not accepted here by
        # the CP/M-86 image reader. No system file is installed on this disk.
        data_disk.write_bytes(logical.read_bytes()[:4096] + b'\xe5' * (327680 - 4096))
        run(['cpmcp', '-f', 'ccpm86-320', str(data_disk),
             *[str(extras / name) for name in tool_names],
             *[str(output / (name + '.exe')) for name in ('pch32', 'pch48', 'pch64', 'pch80', 'pch96', 'pch112', 'pch144', 'pch496', 'pch528', 'pclarge', 'pcrlate', 'pcrpost', 'pcr124', 'pcr125', 'pcr126', 'pcr127', 'pcr129', 'pcr255', 'pcrmany', 'pcrbig', 'pcrnone')], '0:'], ROOT / 'media')
        run(['python3', str(ROOT / 'tools/cpm86twist.py'), 'twist',
             str(data_disk), str(output / 'pc-tools.img')], ROOT)
        size_disk = output / 'size-logical.img'
        size_disk.write_bytes(logical.read_bytes()[:4096] + b'\xe5' * (327680 - 4096))
        run(['cpmcp', '-f', 'ccpm86-320', str(size_disk),
             *[str(extras / name) for name in tool_names],
             *[str(output / (name + '.exe')) for name in ('pcs128', 'pcs129', 'pcs255', 'pcs256', 'pcs257', 'pcs512', 'pcs1k', 'pcs384', 'pcpgzero', 'pcpgbig', 'pcbl512', 'pcminhdr', 'pcsig', 'pcipend', 'pcfxend', 'pcnone')], '0:'], ROOT / 'media')
        run(['python3', str(ROOT / 'tools/cpm86twist.py'), 'twist',
             str(size_disk), str(output / 'pc-size.img')], ROOT)
    # A dedicated small companion keeps new recovery fixtures independent
    # of the full header/relocation disk and preserves the published images.
    recovery_disk = output / 'recovery-logical.img'
    recovery_disk.write_bytes(logical.read_bytes()[:4096] + b'\xe5' * (327680 - 4096))
    recovery_files = [output / 'pcrecov.com']
    if extras and all((extras / name).exists() for name in tool_names):
        recovery_files += [extras / name for name in tool_names]
    run(['cpmcp', '-f', 'ccpm86-320', str(recovery_disk),
         *map(str, recovery_files), '0:'], ROOT / 'media')
    run(['python3', str(ROOT / 'tools/cpm86twist.py'), 'twist',
         str(recovery_disk), str(output / 'pc-recb.img')], ROOT)
    equipment_disk = output / 'equipment-logical.img'
    equipment_disk.write_bytes(logical.read_bytes()[:4096] + b'\xe5' * (327680 - 4096))
    equipment_files = [output / 'pcequip.com', output / 'pcrecov.com',
                       ROOT / 'commands/help/help.cmd', ROOT / 'commands/help/help.hlp']
    if extras and all((extras / name).exists() for name in tool_names):
        equipment_files += [extras / name for name in tool_names]
    run(['cpmcp', '-f', 'ccpm86-320', str(equipment_disk),
         *map(str, equipment_files), '0:'], ROOT / 'media')
    # Original PCTMP's other-drive fallback accepts only SYS commands.
    # Install HELP with that existing attribute rather than changing lookup.
    run(['cpmchattr', '-f', 'ccpm86-320', str(equipment_disk), 's', '0:help.cmd'], ROOT / 'media')
    run(['python3', str(ROOT / 'tools/cpm86twist.py'), 'twist',
         str(equipment_disk), str(output / 'pc-eqpb.img')], ROOT)
    run(['cpmcp', '-f', 'ccpm86-320', str(equipment_disk),
         str(output / 'pcread.com'), str(output / 'pcbusy.com'), str(output / 'pckword.com'), '0:'], ROOT / 'media')
    run(['python3', str(ROOT / 'tools/cpm86twist.py'), 'twist',
         str(equipment_disk), str(output / 'pc-readb.img')], ROOT)
    # Isolated tail-check disk: HELP.COM is a fixture, never the shipped help utility.
    help_disk = output / 'help-fixture-logical.img'
    shutil.copy2(equipment_disk, help_disk)
    run(['cpmrm', '-f', 'ccpm86-320', str(help_disk), '0:help.cmd', '0:help.hlp'], ROOT / 'media')
    run(['cpmcp', '-f', 'ccpm86-320', str(help_disk), str(output / 'pchecho.com'), '0:help.com'], ROOT / 'media')
    run(['cpmchattr', '-f', 'ccpm86-320', str(help_disk), 's', '0:help.com'], ROOT / 'media')
    run(['python3', str(ROOT / 'tools/cpm86twist.py'), 'twist', str(help_disk),
         str(output / 'pc-helpt.img')], ROOT)
    from pcpathbuild import fixtures
    fixtures(output, ROOT, PROBES, run)
    record_build(output, enhanced_keyboard, extras)
    print('Built', output / 'pc-text.img')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('output', type=Path, help='new directory for isolated sources, logs and image')
    parser.add_argument('--extras', type=Path, help='optional local directory with original DOS tools/demos; never required for the core build')
    parser.add_argument('--audit-flag', action='store_true')
    parser.add_argument('--legacy-keyboard', action='store_true', help='disable experimental set-1 prefix filtering for baseline comparisons')
    args = parser.parse_args()
    build(args.output.resolve(), args.audit_flag, not args.legacy_keyboard, args.extras.resolve() if args.extras else None)
