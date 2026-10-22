# Local command search API (BDOS 240 / F0h)

SETDEF and both command processors share settings stored by CON's supervisor.
The API is local to this kernel; the v3 SETDEF utility requires matching CON.
Function 165 is reserved for the later DR AUX attach API; no old-number alias
is provided. Rebuild SETDEF, PCTMP and the kernel together. F0h is project-local,
not a universally assigned CP/M extension number.

Native TMP's source is unchanged: its BDOS 150 CLI uses CMD/SUB and ignores DOS.
PC-MODE's pre-search uses the same policy before its existing DOS loader.

Call INT E0h with CL=240 (F0h) and DS:DX pointing to the packet. BX=0 means success;
BX=FFFFh indicates failure, with the kernel error in CX. A query returns the
current console's effective path, system disk and applicable order bits.

| Offset | Size | Meaning |
| --- | --- | --- |
| 0 | 2 | ASCII SD (word 4453h) |
| 2 | 1 | Version: 1, 2 or 3 |
| 3 | 1 | Operation bits: 0 query, 1 path, 2 order, 3 both |
| 4 | 1 | Path length, 1 through 4 |
| 5 | 4 | Drives 0 through 15; FFh means current drive |
| 9 | 1 | Returned system drive, zero based |
| 10 | 1 | Order bits (v2/v3 only) |

V1 is exactly ten bytes and supports query/path operations only. Byte 10 is
never accessed. V2 adds bit 0 at byte 10: enable SUB. V3 keeps the eleven-byte
size and adds bit 1: disable automatic DOS search. Zero therefore means the
default CMD,DOS. V2 queries expose only bit 0; v1/v2 writes preserve the DOS
setting. V3 queries expose both bits. Unsupported versions, operation values,
counts, drive values and order bits fail before any setting is published.
Only fields selected by the operation are validated for an update.

| V3 byte 10 | SETDEF order | PC-MODE candidates per drive |
| --- | --- | --- |
| 0 | CMD,DOS (default) | CMD, COM, EXE, BAT |
| 1 | CMD,SUB,DOS | CMD, SUB, COM, EXE, BAT |
| 2 | CMD | CMD |
| 3 | CMD,SUB | CMD, SUB |

Policy is per console and lasts until reboot; SYSDISK is shared globally.
Path/order updates preserve the other setting. Search uses drives as the outer
loop and types as the inner loop. Explicit types bypass ORDER; explicit drives
bypass the path and system fallback. DOS execution exists only in PC-MODE.
SUB execution uses the source-built SUBMIT and existing tail/password rules.
Sandbox probes live under `private/pcmode`; they are not build dependencies.
