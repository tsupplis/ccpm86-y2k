#!/usr/bin/env python3
"""Extract fixed-address CCP/M-86 3.1 / Concurrent DOS 3.2 SYS modules.

Uses SYSDAT entry points and the GENCCPM RSP list, not string scanning.
Raw regions partition the complete input, including header and file trailer.
Recovered .rsp files clear generated links/registers and restore base names;
they are reconstruction candidates, not proof of compatibility with a kernel.
See commands/genccpm/genc1.c, genc3.c and genccpm.h for the source format.
"""
import argparse
import hashlib
import json
from pathlib import Path
import re
import struct


def word(data, offset):
    return struct.unpack_from('<H', data, offset)[0]


def put(data, offset, value):
    struct.pack_into('<H', data, offset, value)


def digest(data):
    return hashlib.sha256(data).hexdigest()


def require(condition, message):
    if not condition:
        raise ValueError(message)


def cmd(code, data):
    header = bytearray(128)
    for offset, kind, block in ((0, 1, code), (9, 2, data)):
        require(len(block) % 16 == 0, 'unaligned RSP group')
        struct.pack_into('<B4H', header, offset, kind, len(block) // 16,
                         0, len(block) // 16, 0)
    image = bytes(header) + code + data
    return image + bytes((-len(image)) % 128)


def analyse(blob):
    require(len(blob) >= 128, 'short CMD header')
    require(blob[0] == 1 and blob[9] == 2 and blob[18] == 0,
            'expected CODE and DATA groups only')
    clen, cbase = word(blob, 1) * 16, word(blob, 3)
    dlen, dbase = word(blob, 10) * 16, word(blob, 12)
    require(cbase > 0 and dbase == cbase + clen // 16,
            'expected contiguous fixed-address system groups')
    doff, end = 128 + clen, 128 + clen + dlen
    require(end <= len(blob) and dlen >= 0x210, 'truncated system groups')
    data = blob[doff:end]
    version = word(data, 0x7a)
    require(version & 255 in (0x31, 0x32), 'unsupported SYSDAT version')

    def address(segment):
        offset = doff + (segment - dbase) * 16
        require(doff <= offset < end, 'RSP address outside DATA group')
        return offset

    entries = []
    for index, name in enumerate(('sup', 'rtm', 'mem', 'cio', 'bdos', 'xios', 'net')):
        values = struct.unpack_from('<4H', data, index * 8)
        entries.append(dict(name=name, entry_offset=values[0], entry_segment=values[1],
                            init_offset=values[2], init_segment=values[3]))
    rsps, seen = [], set()
    segment = word(data, 0x42)
    while segment:
        require(segment not in seen, 'cyclic RSP chain')
        seen.add(segment)
        offset = address(segment)
        require(offset + 0x140 <= end, 'short RSP header/PD/UDA')
        header = blob[offset:offset + 0x140]
        name = header[0x18:0x20].decode('ascii').strip()
        require(bool(re.fullmatch(r'[A-Za-z0-9_]+', name)), 'invalid RSP process name')
        cs, ds, es, ss = struct.unpack_from('<4H', header, 0x90)
        require(ds == es == ss == segment and cs > segment,
                'only separate-code RSPs are supported by this extractor')
        rsps.append(dict(name=name, segment=segment, file_offset=offset,
                         previous=word(header, 0), copies_sysdat_offset=word(header, 2),
                         instance=header[4], memory=word(header, 0x26),
                         code_segment=cs, data_segment=ds))
        segment = word(header, 0)
    require(bool(rsps), 'no RSPs')
    rsps.sort(key=lambda item: item['segment'])
    boundaries = sorted({r['segment'] for r in rsps} | {r['code_segment'] for r in rsps})
    require(boundaries[0] == rsps[0]['segment'], 'code before first RSP')
    region_end = dict(zip(boundaries, boundaries[1:] + [dbase + dlen // 16]))
    for rsp in rsps:
        rsp['data_size'] = (region_end[rsp['segment']] - rsp['segment']) * 16
        rsp['code_size'] = (region_end[rsp['code_segment']] - rsp['code_segment']) * 16
        require(rsp['data_size'] >= 0x140, 'short RSP data group')

    regions = []

    def region(filename, start, stop, **details):
        require(0 <= start < stop <= len(blob), 'invalid extraction interval')
        regions.append(dict(file=filename, file_offset=start, size=stop - start,
                            sha256=digest(blob[start:stop]), **details))

    region('raw/header.bin', 0, 128)
    core = entries[:5]
    require(core[0]['init_segment'] == cbase, 'unexpected supervisor base')
    for index, entry in enumerate(core):
        start = 128 + (entry['init_segment'] - cbase) * 16
        stop = (128 + (core[index + 1]['init_segment'] - cbase) * 16
                if index + 1 < len(core) else doff)
        region('raw/core/' + entry['name'] + '.bin', start, stop,
               load_segment=entry['init_segment'])
    xios_start = entries[5]['init_offset']
    # GENCCPM genmfl() starts its first table immediately after static XIOS.
    tables_start = word(data, 0x5a)
    require(0x210 <= xios_start < tables_start < (rsps[0]['segment'] - dbase) * 16,
            'invalid SYSDAT/XIOS/table boundaries')
    region('raw/core/sysdat.bin', doff, doff + xios_start, load_segment=dbase)
    region('raw/core/xios.bin', doff + xios_start, doff + tables_start,
           load_segment=dbase, origin=xios_start)
    region('raw/tables.bin', doff + tables_start, address(rsps[0]['segment']),
           load_segment=dbase, origin=tables_start)
    families = {}
    for rsp in rsps:
        families.setdefault(rsp['code_segment'], []).append(rsp)
        region('raw/rsps/' + rsp['name'].lower() + '.data.bin', rsp['file_offset'],
               rsp['file_offset'] + rsp['data_size'], load_segment=rsp['segment'])
    recovered = []
    for cs, copies in families.items():
        first = copies[0]
        family = first['name']
        # Only the appended numeric instance is stripped, not letters in a name.
        if first['copies_sysdat_offset'] or len(copies) > 1:
            family = first['name'][:-len(format(first['instance'], 'x'))]
        code_start = address(cs)
        code = blob[code_start:code_start + first['code_size']]
        region('raw/rsps/' + family.lower() + '.code.bin', code_start,
               code_start + len(code), load_segment=cs)
        raw_data = blob[first['file_offset']:first['file_offset'] + first['data_size']]
        template = bytearray(raw_data)
        put(template, 0, 0)
        template[4] = 0
        if first['copies_sysdat_offset'] or len(copies) > 1:
            template[0x18:0x20] = family.encode().ljust(8, b' ')
        for offset in range(0x90, 0x98, 2):
            put(template, offset, 0)
        # Check all installed copies by replaying GENCCPM's documented edits.
        for copy in copies:
            require(copy['data_size'] == len(template), 'unequal family data sizes')
            expected = bytearray(template)
            put(expected, 0, copy['previous'])
            expected[4] = copy['instance']
            if first['copies_sysdat_offset'] or len(copies) > 1:
                expected[0x18:0x20] = (family[:7] + format(copy['instance'], 'x')).encode().ljust(8, b' ')[:8]
            for offset, value in zip(range(0x90, 0x98, 2),
                                     (cs, copy['segment'], copy['segment'], copy['segment'])):
                put(expected, offset, value)
            require(bytes(expected) == blob[copy['file_offset']:copy['file_offset'] + len(template)],
                    'unexplained per-instance differences in ' + copy['name'])
        recovered.append((family.lower(), cmd(code, template)))
    if end < len(blob):
        region('raw/trailer.bin', end, len(blob))
    regions.sort(key=lambda item: item['file_offset'])
    cursor = 0
    for item in regions:
        require(item['file_offset'] == cursor, 'gap or overlap in raw extraction')
        cursor += item['size']
    require(cursor == len(blob), 'incomplete extraction')
    return dict(size=len(blob), sha256=digest(blob), version=f'{version:04x}',
                code_base=cbase, data_base=dbase, consoles=data[0x47],
                entries=entries, rsps=rsps, regions=regions), recovered


def verify(source, output):
    manifest = json.loads((output / 'manifest.json').read_text())
    chunks = []
    for item in manifest['regions']:
        data = (output / item['file']).read_bytes()
        require(len(data) == item['size'] and digest(data) == item['sha256'], 'changed raw region')
        chunks.append(data)
    require(b''.join(chunks) == source and digest(source) == manifest['sha256'],
            'extracted regions do not reproduce input byte for byte')
    _, templates = analyse(source)
    for name, image in templates:
        require((output / 'recovered' / (name + '.rsp')).read_bytes() == image,
                'changed reconstructed RSP')


def externalize(source, output):
    """Add inspection containers; preserve installed patches and raw extraction."""
    manifest, templates = analyse(source)
    verify(source, output)
    files = {}
    rows = []
    doff = 128 + word(source, 1) * 16
    label_offset = word(source, doff + 0x78)
    for item in manifest['regions']:
        if not item['file'].startswith('raw/core/'):
            continue
        name = Path(item['file']).stem
        block = source[item['file_offset']:item['file_offset'] + item['size']]
        note = 'Installed GENCCPM patches retained.'
        if name == 'sup':
            require(0 < label_offset < len(block) and label_offset % 16 == 0,
                    'invalid supervisor label boundary')
            files['containers/oslabel.bin'] = block[label_offset:]
            block = block[:label_offset]
            note += ' Appended OS label extracted separately.'
        if name == 'xios':
            # GENCCPM discards the input prefix occupied by SYSDAT. Its original
            # contents are unrecoverable; zero-fill it to preserve XIOS origins.
            block = bytes(item['origin']) + block
            note += ' Missing input prefix zero-filled to preserve origin; generated buffer pointers retained.'
        header = bytearray(128)
        kind = 2 if name == 'sysdat' else 1
        struct.pack_into('<B4H', header, 0, kind, len(block) // 16, 0, len(block) // 16, 0)
        image = bytes(header) + block
        image += bytes((-len(image)) % 128)
        files['containers/' + name + '.con'] = image
        rows.append(f'| {name}.con | {len(block)} | {note} |')
    files['rebuilt.sys'] = b''.join(
        (output / item['file']).read_bytes() for item in manifest['regions'])
    require(files['rebuilt.sys'] == source, 'rebuild differs from source')
    report = [
        '# Externalized system modules', '',
        f"Source SHA-256: `{manifest['sha256']}`. Version word: `{manifest['version']}`.", '',
        '## Kernel containers', '',
        'These .CON files have standard group headers for inspection/disassembly. '
        'They are installed snapshots, not pristine build inputs or independently runnable programs. '
        'Full regeneration with GENCCPM is not yet validated.', '',
        '| File | Group bytes | Recovery status |', '| --- | ---: | --- |', *rows, '',
        '`containers/oslabel.bin` holds the appended supervisor label and its padding. '
        'There is no separate NET region in the extracted layout.', '',
        '## Resident processes', '',
        'The recovered directory contains one .RSP template per shared-code family. '
        'Generated links, initial segment registers, instance number and process-name suffix '
        'are reset. Replaying those edits reproduces every installed DATA instance exactly. '
        'Other runtime/kernel assumptions are retained.', '',
        '| Template | Instances | CODE bytes | DATA bytes per instance |',
        '| --- | --- | ---: | ---: |']
    for name, image in templates:
        # Match recovered code with the structurally identified shared-code group.
        code = image[128:128 + word(image, 1) * 16]
        matches = [r for r in manifest['rsps']
                   if source[doff + (r['code_segment'] - manifest['data_base']) * 16:
                             doff + (r['code_segment'] - manifest['data_base']) * 16 + r['code_size']] == code]
        report.append(f"| {name}.rsp | {', '.join(r['name'] for r in matches)} | {len(code)} | {word(image, 10) * 16} |")
    report += ['', '## Remaining regions and exact rebuild', '',
        '`raw/header.bin` preserves the original load descriptors; `raw/tables.bin` '
        'preserves generated memory/process/buffer tables; `raw/trailer.bin`, when present, '
        'preserves generator text and trailing padding. All installed RSP DATA copies and '
        'shared CODE blocks remain separately available under `raw/rsps`.', '',
        '`manifest.json` records original offsets, sizes, load segments and hashes. '
        '`rebuilt.sys` is the concatenation of all raw regions and is byte-identical to the input. '
        'This validates lossless extraction, not a fresh link from the .CON/.RSP candidates.', '',
        'Files loaded externally (for example MUSW.SYS), original relocation records, '
        'discarded XIOS prefix bytes, and uninitialized memory cannot be recovered from these SYS bytes.', '']
    files['README.md'] = ('\n'.join(report)).encode()
    for name, data in files.items():
        path = output / name
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(data)
        require(path.read_bytes() == data, 'externalized output verification failed')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('input', type=Path)
    parser.add_argument('output', type=Path)
    parser.add_argument('--check', action='store_true', help='verify an existing extraction')
    parser.add_argument('--externalize', action='store_true',
                        help='add CON inspection containers, inventory and exact rebuilt SYS')
    args = parser.parse_args()
    source = args.input.read_bytes()
    if not args.check:
        manifest, recovered = analyse(source)
        args.output.mkdir(parents=True, exist_ok=False)
        manifest['source'] = str(args.input.resolve())
        for item in manifest['regions']:
            path = args.output / item['file']
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_bytes(source[item['file_offset']:item['file_offset'] + item['size']])
        for name, image in recovered:
            path = args.output / 'recovered' / (name + '.rsp')
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_bytes(image)
        (args.output / 'manifest.json').write_text(json.dumps(manifest, indent=2) + '\n')
    verify(source, args.output)
    if args.externalize:
        externalize(source, args.output)
    print(f'{args.input.name}: extracted regions reproduce all {len(source)} bytes; RSP copies verified')


if __name__ == '__main__':
    main()
