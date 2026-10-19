# CCP/M-86 3.1 local patches

This records the local changes in the source-built PC/XT distribution.
It is not a claim to enumerate every historical Digital Research fix.
The functional kernel/RSP changes can be compared with commit `033212a`.

## Kernel

`SETDEF.CMD` and the shared CLI now support one to four command search entries
per console (default `*`). The system drive remains an automatic final fallback;
normal list entries retain normal file visibility, explicit drives bypass the
list, and resident commands keep priority. Missing optional drives are skipped;
other open errors stop the search. Successful loads preserve the caller's working
drive/user. SUBMIT and chaining use the same path. Extension ordering is unchanged.

The local BDOS 165 extension accepts a ten-byte packet at DS:DX:
`'S','D',1,operation,count,entry[4],system_drive`, where operation 0 queries and
1 sets, count is 1..4, and each entry is 0..15 (A..P) or FFh (current drive).
On success BX/AX and CX are zero; the packet returns the effective list and
current system drive. On failure BX/AX=FFFFh and CX is the error code. The kernel
validates the entire update before publishing it atomically. The API supports
console IDs 0..15; CLI callers outside that range retain default `*` searching.
Settings and CLI snapshots use 121 bytes of SUP's existing 128-byte patch
reserve (`patch.cod`), leaving historic
SYSDAT, PD and UDA layouts unchanged. `supif.sup` dispatches function 165 locally
without extending SYSENT; unmodified kernels reject the utility's API call.


| Change | Sources / entry points | Behavior |
| --- | --- | --- |
| Standalone BDOS | `kernel/con/bdos.a86`: `PCMODE=false` | Excludes unavailable DOS-filesystem objects; this build runs CP/M commands. |
| RTC startup | `kernel/con/xrtc.lib`: `rtc_start`; `xinit.lib`: `INIT` | Read BIOS INT 1Ah date/time before replacing interrupt vectors; convert BCD and Gregorian dates to the CP/M clock. Keep the software clock if RTC services are unavailable. |
| RTC persistence | `file5.bdo`: time-set path; `xioscb.def`: `io_setrtc`; `xbegin.lib`: function 25 | BDOS time changes invoke `io_rtc_set`, which writes the BIOS RTC when supported. Calls run in initialization/process context, never IRQ0. |
| RTC vectors | `xinit.lib`: `int_save_tbl`, `int_no_save` | Preserve INT 1Ah for later RTC calls. INT 10h is still not preserved for ordinary application use. |
| Status color | `xchar.lib`: `sl_attrib`; `xinit.lib`: Compaq defaults | Color status text uses attribute 0Eh, bright yellow on black; monochrome renders its corresponding attribute. |
| Cursor shape | `xinit.lib`: `detect_cursor`, `fix_crt`; `xconout.lib`: `set_mono`, cursor parameters | Use enhanced-adapter BIOS character height before vector takeover; use the bottom two scan lines without reading write-only legacy CRTC registers. Keep CGA, MDA and Compaq defaults as fallbacks. Reapply cursor shape/visibility after graphics-to-text register initialization. |
| CGA graphics palette | `xconout.lib`: `color_320`, `black_white_320` | Set the CGA intensity bit at graphics-mode entry for bright colours. |
| EGA/VGA CGA compatibility | `xinit.lib`, `xconout.lib` | Detect the enhanced color adapter at startup; preserve video BIOS/font vectors, including IBM VGA INT 6Dh. Set CGA modes through BIOS on a private stack with IRQs masked, save the console image, restore standard BIOS text/font before redrawing, and block console switching until text returns. Keep legacy CGA register setup. Function 30 CH=2 adds owner-checked packed-CGA palette handling via ESC !. |
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
| Boot track | `boot/bootsect.a86`, `boot/loader.a86`, `boot/Makefile`, `../cpm86-cmdtools/h862bin.c` | Rebuild all 4,096 bytes of the working boot track from reconstructed sources; match the local loader extent-counter value of 3. Remove the unrelated alternate loader and TCOPY. |
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
| Video BIOS | XIOS uses video BIOS internally for CGA modes 4/6 on EGA/VGA. Calls restore registers, flags and PIC mask; a private 1,024-byte stack avoids consuming the application's UDA stack. This is not a general application INT 10h gateway. Native EGA mode 10h is capability-gated; other native EGA modes, custom text fonts/row counts and graphics context switching are not supported. |

## Verification

| Check | Coverage |
| --- | --- |
| `make -C commands/sysdisk check` | Every byte of the reconstructed CMD, including its 128-byte header and padding. |
| `make check` | Reconstructed SYSDISK, image geometry, source-built boot track versus original reference, embedded kernel and expected payloads. |
| `make check-qemu` | All three bootable disks, RTC startup, yellow status, direct/round-robin/keypad switching, SDIR empty/wildcard/options, SYSDISK query/set/syntax, restored HELP topics beyond 64 KB, DREDIX editing/navigation/exit with console switching, WINDOW VIEW and WMENU installation. |

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

## Original HELP recovery and window utilities

Restore CONFIG, DSKMAINT, FUNCTION, HDMAINT, SETUP, SYSDISK, WINDOW and WMENU
from `base/help.hlp` into `commands/help/help.dat`. Restore indexed DDT86
parameters, HELP examples, SET examples/options and FILESPEC conventions,
keeping the existing PIP4, Y2K and expanded source-tool help. Correct the
function-key defaults and console-switch shortcuts, and document both SYSDISK
implementations and configured-drive restrictions.

Package the original WINDOW.CMD and WMENU.CMD on the tools disk, with
CARDDECK.SUB, CLEAR.SUB, GLASS.SUB and WORDPROC.SUB layout samples. Put HDMAINT
and INITDIR on a fourth, bootable maintenance disk, keeping both existing
system disks bootable and the complete rebuilt HELP on the main 320 KB disk.
Select base binaries explicitly instead of adding every new reference binary.


## EGA/VGA graphics interface

XIOS function 30 keeps CH=0 for mode setting and CH=1 for mode queries.
CH=2 sets the CGA color-select byte in CL (bits 0..5) for the graphics owner.
The console ESC ! back door now accepts function 30 and replaces the supplied
DL with the calling virtual console number. Original 3.1 kernels reject that
escape request without printing its eight parameter bytes. The companion
cpm86-hacking library retains its direct CGA port write as a fallback and
sends this palette extension only on BDOS 31h.

On enhanced adapters, modes 4/5 use BIOS palette function 0Bh; mode 6 sets
the two attribute palette entries through function 1000h so palette selection
cannot accidentally turn monochrome graphics into green/white graphics.
Text mode 2/3 resets the BIOS font and hardware before XIOS redraws the
saved console and restores cursor/status visibility. Arbitrary custom fonts
and row counts are not preserved.

Verification uses headless PCE with IBM EGA and VGA ROMs: both bright
four-color palettes, mode 6, repeated transitions via BALLC, saved console
restoration, blocked graphics-console switching and working console
switching/DIR after exit. The BIOS stack's lower 512 bytes remain untouched
in these runs. Physical cards and other ROM implementations remain untested.


## Native EGA 640x350, 16 colours

The enhanced graphics path accepts BIOS mode 10h through ESC a and XIOS
function 30 graphics nibble 5 (mode byte 50h). It rejects the request before
changing ownership on CGA/MDA, a 64 KB EGA, or an adapter without the enhanced
colour BIOS response. Startup records the EGA BIOS memory code (1..3 means
128..256 KB).

Reserved INT E6h v1 selector 30 now returns AX=4745h and BX bit 0 when native
mode 10h is available. The original 3.1 selector was a no-op. The companion
library supplies BX=0 and requires the signature/capability before issuing
the native mode request. It probes only Concurrent BDOS 31h.

Function 30 CH=3 sets native EGA palette entry CL (0..15) to raw six-bit color
DH (0..63). The ESC ! back door forces DL to the caller's console and the
handler checks mode/graphics ownership. CH=2 remains CGA-only. The kernel
uses its protected BIOS function 1000h path, not application BIOS calls.

The companion graphics library draws four planes at A000h with EGA write
mode 2, latches, bit masks and optional XOR. It restores known drawing
defaults rather than reading write-only original EGA registers. Text uses
the 8x14 ROM font selected through vector 43h.

PCE EGA and VGA checks cover all 16 default colours, palette remapping and
invalid arguments, XOR and neighboring pixels, the bottom-right edge,
8x14 text, SINE, repeated text restoration and console switching after exit.
Older kernels reject the capability probe without changing video mode.
Physical EGA/VGA adapters and 64 KB EGA memory detection remain untested.

## Console line editing and single-line recall

BDOS Function 10 in `kernel/con/chario.cio` supports an independent cursor,
character insertion and deletion, and one saved nonempty input line per
console. There is no history list. Blank submissions and abandoned edits
retain the saved line; each accepted nonempty line replaces it.

| Control | Action |
| --- | --- |
| Ctrl-A / Ctrl-F | Move left / right by one buffer character |
| Ctrl-B | Go to beginning; at beginning, go to end |
| Ctrl-W | Recall on an empty line; otherwise go to end |
| Ctrl-G | Delete at cursor |
| Backspace / Rubout | Delete left of cursor |
| Ctrl-K | Delete from cursor to end |
| Ctrl-X | Delete everything to the left of cursor |
| Ctrl-U | Abandon line and advance to a new line |
| Ctrl-R | Retype the whole line on a new line, preserving cursor position |
| Ctrl-E | Physical newline and redraw without accepting input |
| Return / Ctrl-J | Accept the entire line, including the suffix after cursor |

XIOS maps Left/Right to Ctrl-A/Ctrl-F, **Up to Ctrl-W exactly**, Home/End to
absolute beginning/end, and Delete to Ctrl-G, only while Function 10 is
active. The spare CCB flag bit 40h marks this interval and is cleared on
normal return or process termination. PIN forwards tagged editor events
through the existing 16-bit queues. Raw applications and programmable
function-key expansion outside Function 10 retain their existing behavior.

Tabs are measured at eight-column stops and displayed control characters
occupy two cells. Cursor movement follows buffer characters; repainting
erases the old suffix and restores the cursor across wrapped rows using
the existing PC XIOS backspace/wrap behavior. Ordinary appending uses the
fast echo path rather than repainting the line. Editor state uses ten
bytes in the calling process's UDA stack frame, so console switches keep
independent cursor state without enlarging shared structures.

Recall remains console-wide, including application Function 10 prompts,
rather than TMP-only command history. This is CP/M 3-style editing, not
complete CP/M 3 control semantics: Ctrl-R/Ctrl-U do not save unsubmitted
prefixes, Ctrl-R redraws the whole buffer, and Ctrl-E redraws the whole
buffer on the next line. Printer/scroll controls remain handled by PIN.

The sixteen private 256-byte recall slots (length plus up to 255 input
bytes) remain in CIO's code group. Out-of-range console numbers have no
recall. Recall truncates to destination capacity. Full buffers reject extra
characters before any write; a full recalled line waits for Return. A line
filled by appending typed characters retains the original auto-accept
convention; filling the buffer by insertion in the middle waits for Return.

`tools/boot_check.py` covers recall, buffer limits, insertion/deletion,
control and native navigation keys, Up's two meanings, tabs, wrapped rows,
Return from the middle, console isolation and cursor state across switches.
The existing DREDIX checks exercise raw application navigation after the
line editor exits.

## Shell help shortcut and command diagnostics

TMP implements bare `?` as a compact command list and expands `? topic` to
`HELP topic` before parsing. Only a standalone question-mark token is an
alias. Expansion is bounded to the existing 128-character command buffer;
requests longer than 125 characters are rejected. No extra per-console data
is added, and normal HELP command lookup remains unchanged.

TMP now reports distinct command-name, extension, password-syntax, memory,
process, queue, CMD format/read and loader errors. Unknown CLI errors show
their decimal code. Explicit command drives and drive changes are validated
with BDOS error-return mode, restoring the original drive and error mode
before continuing or reporting an unavailable device. Kernel lookup policy
and application error handling are unchanged.
