#!/usr/bin/env python3
"""Archive private experiment trees, verify every file, and optionally prune copies.

Archives preserve their source directory name. Restore with tar -xzf into a
separate directory; verify supports rechecking archives without extracting them.
"""
import argparse
import hashlib
import json
from pathlib import Path
import shutil
import tarfile


def digest(path):
    hasher = hashlib.sha256()
    with path.open('rb') as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b''):
            hasher.update(block)
    return hasher.hexdigest()


def inventory(source):
    result = {}
    for path in sorted(source.rglob('*')):
        if path.is_symlink():
            raise ValueError('Refusing to prune a tree with symlinks: ' + str(path))
        if path.is_file():
            result[str(path.relative_to(source.parent))] = {
                'bytes': path.stat().st_size, 'sha256': digest(path)}
        elif not path.is_dir():
            raise ValueError('Unsupported file type: ' + str(path))
    return result


def verify(archive, manifest=None):
    manifest = manifest or json.loads(archive.with_name(archive.name + '.json').read_text())
    if digest(archive) != manifest['archive_sha256']:
        raise ValueError('Archive SHA-256 differs: ' + str(archive))
    seen = set()
    with tarfile.open(archive, 'r:gz') as stored:
        for member in stored:
            path = Path(member.name)
            if path.is_absolute() or '..' in path.parts:
                raise ValueError('Unsafe archive member: ' + member.name)
            if member.isdir():
                continue
            if not member.isfile():
                raise ValueError('Unsupported archive member: ' + member.name)
            if member.name in seen:
                raise ValueError('Duplicate archive member: ' + member.name)
            seen.add(member.name)
            record = manifest['files'][member.name]
            hasher = hashlib.sha256()
            with stored.extractfile(member) as stream:
                for block in iter(lambda: stream.read(1024 * 1024), b''):
                    hasher.update(block)
            if member.size != record['bytes'] or hasher.hexdigest() != record['sha256']:
                raise ValueError('Archived file differs: ' + member.name)
    if seen != set(manifest['files']):
        raise ValueError('Archive inventory differs: ' + str(archive))
    return manifest


def archive_tree(source, target, prune=False):
    source, target = source.resolve(), target.resolve()
    if not source.is_dir() or target.is_relative_to(source):
        raise ValueError('Expected a source directory and an archive outside it')
    manifest_path = target.with_name(target.name + '.json')
    if target.exists() or manifest_path.exists():
        raise FileExistsError('Existing archives are never overwritten: ' + str(target))
    files = inventory(source)
    target.parent.mkdir(parents=True, exist_ok=True)
    # Disable hard-link coalescing: verification accounts for every regular file.
    with tarfile.open(target, 'w:gz', compresslevel=6, dereference=True) as stored:
        stored.add(source, arcname=source.name)
    manifest = {'source': str(source), 'root': source.name,
                'archive_sha256': digest(target), 'files': files}
    verify(target, manifest)
    manifest_path.write_text(json.dumps(manifest, indent=2) + '\n')
    if prune:
        if inventory(source) != files:
            raise RuntimeError('Source changed during archiving; expanded tree retained')
        shutil.rmtree(source)
    return manifest


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    commands = parser.add_subparsers(dest='command', required=True)
    create = commands.add_parser('archive')
    create.add_argument('source', type=Path)
    create.add_argument('target', type=Path)
    create.add_argument('--prune', action='store_true', help='remove expanded tree only after verification')
    check = commands.add_parser('verify')
    check.add_argument('archive', type=Path)
    args = parser.parse_args()
    if args.command == 'archive':
        result = archive_tree(args.source, args.target, args.prune)
    else:
        result = verify(args.archive)
    print(f"Verified {len(result['files'])} files: " + str(args.target if args.command == 'archive' else args.archive))
