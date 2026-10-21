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
  CMD (default) or CMD,SUB. Each drive is searched in that order, followed by
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
- **Implementation:** private BDOS 165 supports drive-only v1 clients and v2
  path/order updates. Settings reuse SUP's patch reserve without changing
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
| Distribution | Main, tools, development and maintenance disks; retain full HELP on main, provide WINDOW/WMENU plus layout samples on tools, and place DSKMAINT/HDMAINT/INITDIR on maintenance. Select reference binaries explicitly. |

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
