# CCP/M-86 3.1 local patches

Summary of changes in the source-built PC/XT distribution, relative to
`033212a`. This is not a catalogue of historical Digital Research fixes.
User-facing commands and key bindings are documented in README.md and HELP.

## Console and CCP/TMP

- **Line editing:** BDOS Function 10 supports cursor movement, insertion,
  deletion, tabs and wrapped lines. Left/Right, Home/End and Delete work during
  buffered input; **Up maps exactly to Ctrl-W**. Raw application input retains
  its existing behavior.
- **Recall:** one last nonempty input line per console, including application
  Function 10 prompts. Ctrl-W recalls on an empty line and otherwise moves to
  the end. Blank or abandoned lines preserve recall; there is no history list.
  Ctrl-R/U/E behavior is documented in README.md and differs from CP/M Plus.
- **CCP/TMP commands:** built-in CLS and SYSDISK, bare `?` for quick help and
  `? topic` for HELP. Command errors distinguish syntax, unavailable drives,
  memory/resources and loader failures. Unknown errors include their number.
  Idle consoles display `Terminal` in the status line. SYSDISK refuses changes
  while files are open; drive validation restores the original drive/error mode.
  Explicit drives/extensions can still select external commands. Help expansion
  rejects oversized requests without adding per-console storage.
- **SETDEF:** per-console paths of one to four drives, default `*`; search order
  CMD, CMD,SUB, CMD,DOS (default) or CMD,SUB,DOS. DOS expands to COM, EXE,
  BAT in PCTMP and is ignored by native TMP. Each drive is searched in that order, followed by
  the fixed SYS-restricted system-disk fallback. Bare SETDEF displays settings,
  including Sys Disk as the last path entry. Explicit drives/extensions restrict
  lookup; explicit SUB remains available in CMD-only mode. Resident commands
  take priority for untyped names; settings are validated before atomic updates.
- **SUB execution:** uses the matching rebuilt SUBMIT with the resolved script
  drive/user, password and arguments. Nested scripts, chaining and $INCLUDE
  remain supported. Invalid CMD files stop lookup. Oversized expanded tails
  and missing SUBMIT.CMD produce errors 43/44 (126-character expanded-tail
  limit). SUBMIT is located through CMD-only lookup; the caller's working
  drive/user and ordinary explicit SUBMIT behavior are preserved.
- **Implementation:** project-local BDOS 240/F0h supports v3 path/order updates while retaining
  compatibility with drive-only v1 clients and v2 path/order updates. Settings reuse SUP's patch reserve without changing
  SYSDAT, PD or UDA layouts. Recall reserves 4 KiB in CIO; cursor state uses
  ten bytes of the caller's stack frame. Recall truncates to the receiving
  buffer; full buffers reject overflow. Full recalled or middle-insertion lines
  wait for Return; ordinary append retains the original auto-accept convention.
  Programmable keys outside Function 10 and PIN printer/scroll controls remain
  unchanged; normal return and process termination clear editor mode.

## Kernel and display

- Standalone BDOS excludes unavailable DOS filesystem objects. Floppy startup retains
  the system drive selected by GENCCPM. Hard-disk startup selects the BIOS boot
  disk after the installed floppies (normally C:), avoiding an erroneous B:
  selection after successful hard-disk login. Temporary-drive configuration is
  unchanged by hard-disk login. `python3 tools/bootdrv.py` checks the built XIOS
  startup routines with synthetic BIOS geometry and disk-login results.
- BIOS RTC startup and time-setting support preserve the software-clock fallback
  and required interrupt vectors. Date display/input handles century rollover.
- Status text is bright yellow on black. Cursor shape uses enhanced-adapter
  character height, with legacy defaults retained.
- Ctrl+Alt+1–4 selects consoles; Ctrl+Alt+Tab cycles through them. Existing
  keypad shortcuts and console ownership restrictions remain in effect.
- EGA/VGA use protected BIOS calls for CGA-compatible graphics and reliable
  text restoration. CGA intensity/palette handling and enhanced-adapter waits
  are corrected. Native EGA 640x350, 16-colour mode requires a capability probe
  and at least 128 KiB of video RAM; palette changes check graphics ownership.
  This provides no general application INT 10h gateway, custom text modes or
  graphics context switching. BIOS calls use a private 1,024-byte stack and
  preserve registers, flags and interrupt masks; graphics blocks console
  switching until text restoration.
- The graphics interface adds owner-checked XIOS function 30 palette operations
  through ESC !, plus an INT E6h capability signature for native EGA mode 10h.
  The companion library retains legacy CGA fallback and adds four-plane EGA
  drawing, XOR and 8x14 ROM-font text; older kernels reject enhanced requests
  without changing video mode.

## PC-MODE and loader hardening

- Normal kernel builds produce both PCTMP/PC-MODE and native TMP/XIOS systems.
  Separate top-level XIOS assembly wrappers share the same implementation includes.
  PCTMP uses the ordinary ASM86/H86/GENCMD build in kernel/rsp3; runtime builds
  have no dependency on private/pcmode. Media packaging includes BATCH.CMD.
- XIOS gives DOS sessions 25 text rows, hides the status line while row 25 is
  owned, and restores native 24-row operation on return. Equipment reporting
  uses cached hardware information and the console's saved entry mode. BIOS
  character-only output uses the console attribute, matching recovered XIOS.
- PCTMP validates EXE image placement against the actual allocation, excluding
  private loader storage and PSP, and reserves image plus MINALLOC. Both aligned
  and unaligned EXEs use the bounded scratch reader, avoiding final-record
  padding writes outside the allocation and rejecting premature EOF.
- Native CMD preflight rejects unsupported/duplicate group types, image lengths
  exceeding group minima, file-paragraph overflow, missing base groups and
  insufficient base-page capacity before allocation or CHAIN memory changes.
  Empty groups do not participate in relative-memory spreading.
- Native CMD relocation checks reject invalid groups, overflowing paragraph
  calculations and writes beyond the actual group extent, including reused
  shared-code extents. Premature group EOF is an error; read/abort error codes
  survive loop-counter restoration.
- Native CMD cleanup records allocations only after success. Shared-code
  attachments are tracked for rollback; failures unlink the actual newly
  created cache owner, preserving existing cached owners. Failed sharing is
  handled instead of assuming memory-descriptor allocation always succeeds.
  The rollback owner uses two spare bytes in SUP's existing patch reserve;
  SYSDAT, PD and UDA layouts are unchanged.
- Status cache invalidation also runs after window repaint, so a status refresh
  during repaint cannot leave a blank line cached as already displayed.

Validation on 22 October 2026: supervisor assembly and normal kernel/SYSGEN
build succeed. A read-only inventory of all 13 base CMD files satisfies the
new header checks. User PCE screenshots confirm row-25 ownership/status restoration
and the equipment probe. Seventeen focused native CMD cases pass under both PCTMP and native TMP, covering
header rejection, relocation bounds, truncated reads and shared failure/recovery.
A further 24 DOS/MZ/video checks pass, including stack/image bounds, valid EXE
recovery, character output, status restoration and second-console return. All six
packaged images pass payload, boot-track and size comparisons. These results do
not claim exhaustive allocation-failure or CHAIN coverage. Review notes and build logs remain in private/pcmode.

Real BATCH.CMD also passes eight grouped floppy cases: explicit and automatic BAT
selection, multiple commands, %1/%2 arguments, empty scripts, REM/blank lines,
missing-command continuation and subsequent script recovery. These use the
packaged interpreter, not the earlier routing stub; broader batch semantics
remain outside this check.

## Commands and build

| Area | Changes |
| --- | --- |
| SDIR | Link missing disk-parameter code and terminate command tails correctly. |
| DATE, SHOW, TOD, SDIR | Y2K date input/display corrections. |
| PIP | Ship PIP4 fixes with the 3.1N banner; BDOS requirement remains 3.1. |
| HELP | Rebuild indexed help, restore original CONFIG/maintenance/DDT86/SET/FILESPEC/window topics and examples, correct function-key defaults, and document PIP archive/confirmation options and HEX validation; support topics beyond 64 KiB. |
| DREDIX | Use BDOS keyboard input and console escapes with a local 80x24 screen buffer; preserve navigation, highlighting and console restoration. |
| SYSDISK | Reconstruct readable source with byte-identical output; retain the original OS-version guard. |
| INITDIR | Rebuild with corrected PL/I object conversion: preserve frame/target relocations and reserved DATA storage. Track declaration/converter dependencies; explicit `make -C commands/initdir check` exercises timestamp creation/removal on disposable disks. |
| Resident processes | Rebuild supplied sources; remove PIN's duplicate Ctrl-C declaration and obsolete standalone CLS/SYSDISK RSPs. |
| GENCCPM | Correct three-byte RTM flag serialization; size disk buffers to keep kernel/RSP data within 64 KiB. |
| Build/media | Track source dependencies, rebuild the byte-identical 4,096-byte boot track (including loader extent count), remove the alternate loader/TCOPY, and handle physical floppy track ordering, including reversed head-1 cylinders. |
| Distribution | PC-MODE and native system disks, tools, native/DOS development and maintenance disks; retain full HELP on main, provide WINDOW/WMENU plus layout samples on tools, and place DSKMAINT/HDMAINT/INITDIR plus configuration utilities and SYSTAT on maintenance. Tools and both developer disks are data-only. Select reference binaries explicitly. |

## Limits and verification

- Default process quota: **256 KiB**, summed across all owned allocations.
  GENCCPM `memmax` uses hexadecimal 16-byte paragraphs (`4000` = 256 KiB).
- Default application pool: **368 KiB**, physical 04000h–60000h, in combinable
  16 KiB partitions. Raising the quota does not enlarge this pool.
- Each 8086 segment remains limited to 64 KiB; CMD group requests and an
  application's memory model impose additional limits.

`make check` verifies reconstructed binaries, boot-track identity, disk geometry
and image contents. `make check-qemu` covers booting, console switching, editing
and recall, SETDEF/search/SUB execution, command diagnostics, HELP, SDIR, SYSDISK,
DREDIX and window utilities. Separate PCE checks with IBM EGA/VGA ROMs cover
palettes, native EGA drawing and repeated graphics/text transitions.

DREDIX's README records modified-function-key overlap and missing help/tutorial
data. Detailed interfaces and implementation notes remain in the corresponding
sources; obsolete intermediate build-size figures are omitted here.

Physical hardware, other video ROMs, cross-OS Y2K compatibility and all imported
development binaries are not exhaustively tested.

BATCH.CMD is marked SYS on both system disks, allowing the SYS-restricted
system-disk fallback to find it from B:. Media verification checks this attribute.
The real-interpreter suite now passes 11 cases, adding implicit/explicit BAT
execution and arguments with B: current; all three preserve B: on return.

DOS application validation now also includes a complete MASM 1.10 → LINK 1.10 →
EXE execution → EXE2BIN → COM execution round trip, plus six supplied DOS BASIC
5.50A workflow checks covering input, arithmetic, SAVE/NEW/LOAD, rerun and SYSTEM
exit. Both generated programs and the saved BASIC program were extracted from
the test disks. The COM-convertible assembly sample has an expected no-STACK
linker warning. No kernel or application binary fixes were needed for these runs.

The same DOS toolchain also completes inside a single BAT script via the real
BATCH.CMD, with B: current and the interpreter on SYS A:. Freshly generated
OBJ/EXE/COM match the individual-command outputs byte for byte; both programs
run and the shell/status line return. This adds validation only, with no
runtime code changes.

API coverage is documented in [api.md](api.md): DOS/BIOS services, DOS 1.10
and MS-DOS 1.25 differences, native INT E0h functions and INT E6h selectors.
This is a source inventory; implemented handlers are not a full-conformance claim.

SETDEF's project-local INT E0h API moved from 165/A5h to 240/F0h because
Concurrent DOS 4.1 assigns 165 to AUX attach. SUP, SETDEF.CMD and PCTMP
are updated together; no old-number alias remains. Packet layouts v1–v3
are unchanged. Old SETDEF/PCTMP binaries require rebuilding with the kernel.
F0h is unused in the reviewed historical tables, not a universal reservation.

## DOS version query

- **NEW — INT 21h/AH=30h:** PCTMP returns AX=1901h (AL=1, AH=25 decimal), reporting 1.25. Other registers are preserved; no OEM/serial outputs are defined. This is a project extension, not a claim of complete DOS 1.25 compatibility. Native TMP is unchanged.

SETDEF source follows the command-module layout: module header, routine sections,
aligned ASM86 instructions, one instruction per line and descriptive helper labels.
This source-only cleanup preserves SETDEF.CMD byte for byte.
