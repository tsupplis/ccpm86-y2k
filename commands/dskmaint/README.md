# CCP/M DSKMAINT and dual-OS DFORM

DSKMAINT is reconstructed from this repository's `base/dskmaint.cmd`:
Digital Research version 1.5, dated 26 January 1984. The copied CP/M-86
sources have been replaced here; the other repository retains that version.

## Historical reconstruction

`make check-dsk` rebuilds both the annotated `dskmaint.a86` and frozen
`dskmorg.a86`, then compares their complete CMD files with the original.
All 6400 bytes must match, including these descriptors and final padding:

| Group | Stored bytes | Minimum bytes | Maximum bytes |
| --- | ---: | ---: | ---: |
| CODE | 2208 | 2208 | unspecified |
| DATA | 3776 | 3776 | unspecified |
| EXTRA | 0 | 8192 | 1048560 |
| STACK | 256 | 256 | unspecified |

ASM86 emits the four groups. GENCMD's `extra[m200 xffff]` supplies the
EXTRA allocation limits in paragraphs, without a post-build header patch.
DATA uses runtime origin 100h; its preceding base page is included by GENCMD.

The 147-byte boot code is identical to CP/M-86's. The 846-byte loader differs
only at offset 51h, where the missing-extent allowance is reset to 3 instead
of 2. Both standalone sources are assembled, converted by `h862bin`, and
included as data using `bin2asm`. Those tools are taken from PATH.

Unlike the CP/M edition, DSKMAINT uses native XIOS. It obtains SYSDAT, UDA,
process and console pointers, requires a local foreground unbuffered console,
sets P_KEEP/NOSWITCH, excludes filesystem access with the MXdisk queue and
releases those restrictions on exit. Its stack IOPB carries drive/count,
logical track, sector, DMA segment and offset. XIOS 15 formats, 11 writes,
and 10 reads/checks. Menu geometry comes from the target disk parameter block.

## DFORM

One DFORM binary supports single-user CP/M-86 1.1 (BDOS AX=0022h) and the
Concurrent/MPM family with OS version 1431h (CCP/M-86 3.1). Other versions
are rejected. The parser, confirmation, track-zero construction and format
workflow are shared. Existing command options and usage display are retained.

The CP/M backend uses ROM BIOS and temporarily installs DSKMAINT's diskette
parameter table. The CCP/M backend, in `ccpmio.a86`, uses the kernel's native
XIOS format/read/write services. It preserves the process UDA convention,
locks MXdisk only after confirmation, checks the open-file vector again under
the lock, and tracks partial acquisition for cleanup. It invalidates the
affected DPH login sequence, returns the token, resets BDOS state and restores
console/P_KEEP/priority state on success, cancellation and I/O failure.

Both paths write the same E5-filled boot track, with the boot code at zero,
side flag at 1FFh and loader at 900h. The loader's single differing byte is
selected for the detected OS. Verification uses BIOS function 4 on CP/M or a
native XIOS read on CCP/M. Both paths currently target the original IBM PC
40-cylinder, eight-sector, 512-byte formats (160K/320K), on A: or B:.

## Checks

`make check` combines full DSKMAINT byte parity and the DFORM simulation.
`tools/dfmtest.py` drives the NASM harness `tests/dfmtest.asm`; Python tools
remain in the top-level tools directory, and new filenames follow 8.3.

The harness runs the actual linked DFORM CMD on both OS paths. It checks the
full boot-track contents, sector IDs/track order or native stack IOPBs,
optional verification, retries, cancellation, ES/register preservation and
cleanup. Native cases reject any ROM BIOS access, and exercise foreground,
open-file, queue-failure and version checks. The harness accesses no host disk.
Real CP/M/CCP/M and floppy-hardware validation remains outstanding.
