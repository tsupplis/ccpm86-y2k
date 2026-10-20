#!/usr/bin/env python3
"""Check real XIOS startup instructions under emu2, with disk login stubbed.

Run after make -C kernel/con. BIOS geometry and label login are synthetic;
startup, drive mapping and DPH updates execute from the built xios.con.
No host disks are accessed. This is an explicit regression, not a build step.
"""
from pathlib import Path
import re
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parent.parent
CON = ROOT / 'kernel/con'
SYMBOLS = dict((name, int(value, 16)) for value, name in re.findall(
    r'\b([0-9A-F]{4}) ([A-Z][A-Z0-9]*)\b', (CON / 'xios.sym').read_text()))
HARNESS = r'''bits 16
cpu 8086
org 100h
start:
    mov dx,bios
    mov ax,2513h
    int 21h
    mov ax,cs
    add ax,(payload-$$+100h)/16
    mov [entry+2],ax
    mov [mapping+2],ax
    mov ds,ax
    mov es,ax
    ; A near call and far return trampoline in unused offset zero.
    mov byte [0],0e8h
    mov word [1],HDINIT-3
    mov byte [3],0cbh
    mov byte [HDLOGIN],0b0h
    mov byte [HDLOGIN+1],LOGIN
    mov byte [HDLOGIN+2],0c3h
    mov byte [HDINITHDINF],0c3h
    mov word [NUMFLOP],FLOPPIES
    mov word [NUMHARD],HARD
    mov byte [BOOTDISK],BOOT
    mov byte [SYSDISK],SYSTEM
    mov byte [TEMPDISK],TEMP
    ; Check mapping independently of login outcome.
    call far [cs:mapping]
    call far [cs:entry]
    cmp byte [SYSDISK],EXPECTED
    jne fail
    cmp byte [TEMPDISK],TEMP
    jne fail
    cmp byte [HDINITFLAG],0ffh
    jne fail
%if HARD
    cmp word [DPHTBL+2*FLOPPIES],HDDPH0
    jne fail
%if HARD == 2
    cmp word [DPHTBL+2*FLOPPIES+2],HDDPH1
    jne fail
%endif
%if LOGIN
    cmp word [HDDPH0+8],HDDPB0
    jne fail
    cmp word [HDDPH1+8],HDDPB1
    jne fail
%endif
%endif
    mov ax,4c00h
    int 21h
fail:
    mov ax,4c01h
    int 21h
bios:
    iret
entry: dw 0,0
mapping: dw mapcall,0
align 16
payload:
    incbin IMAGE,128
; Mapping uses its own trampoline beyond the loaded XIOS bytes.
mapcall equ $-payload
    db 0e8h
    dw FIXDISKTABLES-(mapcall+3)
    retf
'''


def check():
    with tempfile.TemporaryDirectory(prefix='ccpm-bootdrv-') as td:
        source = Path(td) / 'bootdrv.asm'
        image = Path(td) / 'bootdrv.com'
        source.write_text(HARNESS)
        count = 0
        for floppies in (1, 2):
            for hard in (0, 1, 2):
                for login in (0, 255):
                    for system, temp in ((0, 0), (2, 3)):
                        for boot in (0, 0x80, 0x81, 0x82):
                            expected = (floppies + boot - 0x80
                                        if hard and login and 0x80 <= boot < 0x80 + hard
                                        else system)
                            options = dict(SYMBOLS, BOOT=boot, EXPECTED=expected, FLOPPIES=floppies, HARD=hard,
                                           LOGIN=login, SYSTEM=system, TEMP=temp)
                            subprocess.run(['nasm', '-f', 'bin', str(source), '-o', str(image),
                                            f'-DIMAGE="{CON / "xios.con"}"',
                                            *(f'-D{k}={v}' for k, v in options.items())], check=True)
                            result = subprocess.run(['emu2', str(image)], capture_output=True,
                                                    timeout=10)
                            assert result.returncode == 0, (
                                f'floppies={floppies} hard={hard} login={login} '
                                f'boot={boot:02x} system={system} temp={temp}: {result.stderr.decode()}')
                            count += 1
    print(f'PASS: {count} XIOS startup cases; configured drives, HD mapping and DPBs')


if __name__ == '__main__':
    check()
