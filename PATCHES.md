# CCP/M-86 3.1 local patches

This records the local changes in the source-built PC/XT distribution.
It is not a claim to enumerate every historical Digital Research fix.
The functional kernel/RSP changes can be compared with commit `033212a`.

## Kernel

| Change | Sources / entry points | Behavior |
| --- | --- | --- |
| Standalone BDOS | `kernel/con/bdos.a86`: `PCMODE=false` | Excludes unavailable DOS-filesystem objects; this build runs CP/M commands. |
| RTC startup | `kernel/con/xrtc.lib`: `rtc_start`; `xinit.lib`: `INIT` | Read BIOS INT 1Ah date/time before replacing interrupt vectors; convert BCD and Gregorian dates to the CP/M clock. Keep the software clock if RTC services are unavailable. |
| RTC persistence | `file5.bdo`: time-set path; `xioscb.def`: `io_setrtc`; `xbegin.lib`: function 25 | BDOS time changes invoke `io_rtc_set`, which writes the BIOS RTC when supported. Calls run in initialization/process context, never IRQ0. |
| RTC vectors | `xinit.lib`: `int_save_tbl`, `int_no_save` | Preserve INT 1Ah for later RTC calls. INT 10h is still not preserved for ordinary application use. |
| Status color | `xchar.lib`: `sl_attrib`; `xinit.lib`: Compaq defaults | Color status text uses attribute 0Eh, bright yellow on black; monochrome renders its corresponding attribute. |
| Cursor shape | `xinit.lib`: `detect_cursor`, `fix_crt`; `xconout.lib`: `set_mono`, cursor parameters | Detect enhanced-adapter character scan-line height before vector takeover; use the bottom two scan lines. Keep CGA, MDA and Compaq defaults as fallbacks. |
| Enhanced-adapter waits | `xchar.lib`: `sl_put1`; `xconout.lib`: screen clearing; `xinit.lib`: `var_sync` | Disable CGA snow/retrace waits on EGA/VGA while retaining them for legacy CGA. |
| Configured drives | `xinit.lib`: `fix_disk_tables` | Retain GENCCPM system/temporary drives instead of replacing them with the last floppy drive. |
| Console shortcuts | `xbegin.lib`: `key_make`, `key_next_console`, `key_console_event` | Ctrl+Alt+top-row 1–4 selects consoles 0–3. Ctrl+Alt+Tab advances from the foreground console and wraps at four consoles. Emit existing switch events, retaining PIN ownership/restrictions and keypad aliases. |

## Resident processes

| RSP | Sources | Local change |
| --- | --- | --- |
| PIN | `kernel/rsp1/pin.p86`, `comlit.lit` | Remove the duplicate `ctrlC` literal declaration; the shared include already defines it. No new switching policy is added to PIN. |
| VOUT, SERIN, SEROUT, TMP | `kernel/rsp1/` | Rebuilt from supplied sources; no additional functional patch in this set. |
| ABORT, CLOCK, DIR, ECHO | `kernel/rsp2/` | Rebuilt from supplied sources; no additional functional patch in this set. |

## Generator and build corrections

| Change | Sources | Reason |
| --- | --- | --- |
| Three-byte RTM flags | `commands/genccpm/genc3.c` | Serialize each flag as explicit bytes `FF FF FF`. Compiler-dependent structure layout previously produced `00 FF FF`, allowing the scheduler to interpret FF00h as a process pointer. |
| Kernel data size | `kernel/sysgen/genccpm.in` | Four directory buffers and two data buffers per shared pool keep data plus resident-process data within 64 KB. |
| Rebuild dependencies | `kernel/con/Makefile`, `kernel/sysgen/Makefile` | Track included assembly and actual RSP sources; depend on GENCCPM so changed components regenerate the system image. |
| Boot track | `boot/bootsect.a86`, `boot/loader.a86`, `boot/Makefile`, `tools/h862bin.py` | Rebuild all 4,096 bytes of the working boot track from reconstructed sources; match the local loader extent-counter value of 3. Remove the unrelated alternate loader and TCOPY. |
| Media construction | `media/Makefile`, `tools/cpm86twist.py` | Install the verified source-built boot track. Use CP/M logical track order for cpmtools, then restore physical CHS order, including reversed head-1 cylinders. |

## Command changes

| Tool | Sources | Change |
| --- | --- | --- |
| SDIR | `commands/sdir/Makefile`, `sdirmain.plm`, `dpb.plm` | Link the missing disk-parameter module, reject unresolved links, and explicitly terminate the length-prefixed command tail. This fixes re-entry into startup and the misleading illegal-tail error. |
| DATE, SHOW, TOD, SDIR | Corresponding PL/M sources; SDIR `timest.plm` | Y2K year display correction; DATE/TOD date input handles the century rollover. Existing compatibility coverage is in the README. |
| PIP | `commands/pip/pip4.plm` | Package PIP4 fixes with the `CONCURRENT CP/M-86 PIP VERSION 3.1N` banner. The required BDOS version remains 3.1; this is not the original PIP3 binary. |
| HELP | `commands/help/help.dat`, `Makefile` | Rebuild indexed HELP.HLP; document PIP 3.1N, explicit-name archive/confirmation options, and HEX validation. |
| DREDIX | `commands/dredix/vd.c`, `vk.c`, `ed.c` | Replace private XIOS calls/direct screen writes with BDOS keyboard input and a local 80x24 character/attribute buffer rendered through console escapes. Preserve scroll/highlight behavior and restore console mode on exit. See its README for overlapping modified-function-key codes and missing help/tutorial data. |
| SYSDISK | `commands/sysdisk/sysdisk.a86`, `Makefile` | Reconstruct the original utility as readable ASM-86, retaining its instruction encodings, data offsets, stack and CMD layout. `make check` requires byte identity with `commands/sysdisk/sysdisk.ref`. The original accepts OS versions 1410h and 1431h; that guard is deliberately unchanged. |

## Memory and BIOS limits

| Setting | Current meaning |
| --- | --- |
| `memmax=4000` paragraphs | 256 KiB total allocation per process, including its code/data and other owned allocations; inherited from `sysdat.dat`. |
| GENCCPM default pool `400,6000,400` | Physical 04000h–60000h in 16 KiB partitions, 368 KiB before overhead and concurrent use. Adjacent free partitions can be combined. |
| CMD group minimum/maximum | Per-program loader requests; they do not override the process allocation limit. |
| 8086 segment | 64 KiB addressable with one segment value; DREDIX's small-model data/heap remains within one data segment. |
| Video BIOS | Preserving INT 10h alone would not synchronize BIOS mode/cursor/screen changes with virtual consoles. Existing graphics ownership and mode support are CGA-oriented. No general INT 10h gateway has been added. |

## Verification

| Check | Coverage |
| --- | --- |
| `make -C commands/sysdisk check` | Every byte of the reconstructed CMD, including its 128-byte header and padding. |
| `make check` | Reconstructed SYSDISK, image geometry, source-built boot track versus original reference, embedded kernel and expected payloads. |
| `make check-qemu` | Both bootable disks, RTC startup, yellow status, direct/round-robin/keypad switching, SDIR empty/wildcard/options, SYSDISK query/set/syntax, and DREDIX editing/navigation/exit with console switching. |

Cross-OS Y2K compatibility, physical machines, and all imported development
binaries are not exhaustively covered by the automated 3.1 guest checks.

## TMP built-in commands

`kernel/rsp1/tmp.a86` implements `CLS` and `SYSDISK` as TMP built-ins.
CLS outputs VT52 ESC-E. SYSDISK queries or changes the system drive
(`SYSDISK A:`), rejecting invalid syntax, unavailable drives, and changes while
files are open. Device validation preserves TMP's default drive and error mode.
An explicit drive or extension still selects an external command; the original
SYSDISK.CMD remains available as a byte-identical reconstruction.

| Resource | Added cost |
| --- | ---: |
| Shared TMP code and constants, both commands | 384 bytes (paragraph aligned) |
| Data per console | 0 bytes |
| Extra processes, UDAs, stacks or queues | None |

TMP's code group grows from 1,280 to 1,664 bytes; its per-console data remains
624 bytes. This saves 1,136 bytes compared with the separate 1,520-byte RSPs.
The obsolete `kernel/custrsps/` implementations are removed. The system-image
recipe also removes stale CLS/SYSDISK RSP copies before GENCCPM scans the folder.

The status line displays `Terminal` when owned by TMP; application names still
appear while applications own the console.
