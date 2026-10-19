# Concurrent CP/M-86 3.1 Kernel and Tools

**Disk Images are now available as part of the project releases, ready to use !**

This project preserves Digital Research's Concurrent CP/M-86 3.1 as a usable,
rebuildable system for running and exploring classic 16-bit software. Runnable
disks and development tools accompany the sources and patch notes, making the
system useful today and accessible to anyone interested in its history.

<p align="center">
<img src="images/ccpm86.png" alt="Concurrent CP/M-86 3.1 running in PCE" width="48%">
<img src="images/windows.png" alt="Overlapping console windows" width="48%">
<br><sub>Concurrent CP/M-86 3.1 &nbsp;·&nbsp; Console windows</sub><br>
<img src="images/setup.png" alt="XIOS configuration menu" width="48%">
<img src="images/basic.png" alt="Microsoft BASIC 5.50A" width="48%">
<br><sub>System setup &nbsp;·&nbsp; Microsoft BASIC 5.50A</sub>
</p>

| Goal | Distribution |
| --- | --- |
| Full source distribution | Kernel, nine RSPs, most commands; gaps below |
| Additional tools | ASM80, HEXCOM, DREDIX, DR development chain, Microsoft BASIC 5.50A |
| Y2K components | SDIR, DATE, SHOW, TOD for CCP/M-86 3.2 through Concurrent DOS 4.1; 3.1 fixes included |

## Distributions

| Image | Bootable | Contents |
| --- | --- | --- |
| `ccpm86.img` | Yes | Kernel, stock commands, ED, HELP, SYSTAT, configuration/maintenance utilities, reconstructed SYSDISK |
| `ccpmtools.img` | Yes | DREDIX, ASM86, DDT86, GENCMD, GENCCPM, WINDOW/WMENU with sample SUB files, PIP, TOD |
| `ccpmmaint.img` | Yes | HDMAINT, INITDIR, PIP, TOD |
| `ccpmdev.img` | No | DR development/BASIC tools, Microsoft BASIC, ASM80, HEXCOM, GENTAB |

`make dist` places the images above and `disks.zip` containing all four at the repository root.

320 KB: **untwist → cpmtools `ccpm86-320` → twist**.
[Media recipes and emulator usage](media/README.md).
The full HELP is on the main disk; WINDOW and WMENU are on the tools disk.
HDMAINT and INITDIR are on the bootable maintenance disk.
Window layouts: `CARDDECK.SUB`, `CLEAR.SUB`, `GLASS.SUB` and `WORDPROC.SUB`.
The [source-built boot track](boot/README.md) reproduces all 4,096 bytes of the original.

## Tools

Format: [cpm86-kernel](../cpm86-kernel/README.md). **3.1** = rebuildable sources; SYSDISK: byte-identical CMD plus TMP built-in.

| Tool | Purpose | Binary (`base/`) | Source (`commands/`) |
| --- | --- | --- | --- |
| `asm86` | 8086 assembler | — | 3.1 |
| `cls` | Clear the terminal screen | — | TMP built-in; VT52 ESC-E |
| `chset` | Inspect or change CMD header flags | — | 3.1 |
| `config` | Configure serial ports | Original | — |
| `date` | Display or set the date | — | 3.1, Y2K patched |
| `ddt86` | Interactive machine-code debugger | — | 3.1 |
| `dir` | List directory contents | — | 3.1; command/RSP |
| `dskmaint` | Format, verify and copy diskettes | Original | — |
| `ed` | Line-oriented text editor | — | 3.1 |
| `era` / `eraq` | Erase files, optionally with confirmation | — | 3.1 |
| `function` | Program function keys | Original | — |
| `gencmd` | Convert H86 output into CMD executables | — | 3.1 |
| `genccpm` | Generate and configure the system image | — | 3.1; flag-table fix |
| `gentab` | Generate disassembler opcode tables | — | 3.1 |
| `hdmaint` | Hard-disk maintenance | Original | — |
| `help` / `help.hlp` | Browse indexed command help | — | 3.1; indexed from `help.dat` |
| `initdir` | Enable directory date/time stamps | — | 3.1 |
| `pcetime` | Synchronize time with the PCE host | Original | — |
| `pip` | Copy and transfer files | — | 4.1; shipping PIP4 as 3.1N |
| `ren` | Rename files | — | 3.1 |
| `sdir` | Detailed, sorted directory listing | — | 3.1, Y2K/DPB/tail fixes |
| `set` | Set file and drive attributes | — | 3.1 |
| `setup` | Customize IBM PC/XT XIOS settings | Original | — |
| `show` | Display disk space and drive information | — | 3.1, Y2K patched |
| `submit` | Run batch command files | — | 3.1 |
| `sysdisk` | Display or change the system drive; TMP built-in and standalone tool | — | TMP built-in + standalone `SYSDISK.CMD`; byte-identical CMD reconstruction |
| `systat` | Inspect processes and system resources | — | 3.1 |
| `tod` | Display or set date and time | — | 3.1, Y2K patched |
| `type` | Display a text file | — | 3.1 |
| `window` | Control console windows from commands | Original | — |
| `wmenu` | Interactive console-window manager | Original | — |
| `vcmode` | Set virtual-console background modes | — | 3.1 |
| `asm80` | 8080 assembler | — | CP/M 2.2 8080 ASM; XLT86-translated and fixed |
| `hexcom` | Convert Intel HEX to COM binaries | — | HEXCOM 3.00-compatible C from [tpzasm](https://github.com/johnsonjh/tpzasm) |
| `dredix` | Full-screen text editor | — | 4.1; DREDIX C Backported from Concurrent DOS 4.1 |

### Development binaries

`dev/` contains imported binaries; guest compatibility is not fully audited.

| Tool | Product | Version | Vendor |
| --- | --- | --- | --- |
| `rasm86.cmd` | Relocating assembler | 1.4 | Digital Research |
| `link86.cmd` | Linkage editor | 2.02 | Digital Research |
| `lib86.cmd` | Library manager | 1.3 | Digital Research |
| `xref86.cmd` | Cross-reference tool | 1.1 | Digital Research |
| `sid86.cmd` | Debugger with 286 disassembler | 2.4 | Digital Research |
| `cbas86.cmd` | CBASIC compiler | 1.4 | Digital Research |
| `crun86.cmd` | CBASIC runtime | 1.4 | Digital Research |
| `pbasic.cmd` | Personal BASIC | 1.2 | Digital Research |
| `mbasic86.cmd` | [BASIC-86 recreation](https://github.com/tsupplis/cpm86-msbasic) | 5.50A | Microsoft lineage |
| `mbas522.cmd` | BASIC-86 reference; archive only | 5.22 | Microsoft |

## Y2K utilities

| Utility | CCP/M-86 3.1 | 3.2 family | Concurrent DOS 4.1 | DOS Plus 1.2 |
| --- | --- | --- | --- | --- |
| SDIR | Fixed | Fixed | Fixed | Fixed |
| DATE | Fixed | Compliant | Compliant | Compliant |
| SHOW | Fixed | Fixed | Compliant | Compliant |
| TOD | Fixed | Fixed | Compliant | Compliant |

Fixed = replacement needed; Compliant = existing utility works.
Audit: CCP/M-86/Concurrent DOS 3.2; boot regressions: 3.1.

## Kernel and RSP patches

[PATCHES.md](PATCHES.md): RTC startup/persistence, yellow status, adapter-aware
cursor, EGA/VGA retrace handling, console shortcuts, configured drives,
GENCCPM flag records, PIN and build corrections. CGA graphics modes 4/6 now
use kernel-controlled BIOS mode/palette calls on EGA/VGA, preserving the CGA
path and restoring text. Native EGA modes and a general application INT 10h
layer are not provided.

## Build and checks

Build: [cpm86-crossdev](https://github.com/tsupplis/cpm86-crossdev), make,
cpmtools, Python 3, Git, tar, zip. Emulators: PCE (interactive), QEMU (checks).
The boot build requires native `h862bin` from `cpm86-cmdtools` on PATH.

| Command | Result |
| --- | --- |
| `make` | Commands, kernel/RSPs, boot sources, four images |
| `make -C commands/sysdisk check` | Byte-identical SYSDISK reconstruction |
| `make check` | SYSDISK identity, source-built boot identity, every packaged file, sizes |
| `make check-qemu` | Boot, RTC, status, switching, HELP, SDIR, SYSDISK, DREDIX, windows |
| `make test` | Launch PCE |
| `make dist` | `y2k-dist.zip`, `disks.zip` and the four disk images at the repository root |
| `make source-dist` | Sources and reference/vendor inputs; no ignored outputs |

[Source pedigree](http://www.cpm.z80.de) · [License correspondence](LICENSE.md).

---

## Companion projects

| Project | Description |
|---------|-------------|
| [cpm86-kernel](https://github.com/tsupplis/cpm86-kernel)     | CP/M-86 1.1 distribution rebuilt from patched and reconstituted sources |
| [ccpm86-y2k](https://github.com/tsupplis/ccpm86-y2k)         | CCP/M-86 3.1 distribution rebuilt from patched and reconstituted sources |
| [cpm86-crossdev](https://github.com/tsupplis/cpm86-crossdev) | Unix CP/M-86 cross development project (compilers, emulation and tools) |
| [cpm86-hacking](https://github.com/tsupplis/cpm86-hacking)   | CP/M-86 miscellaneous tools and PCE emulator helpers |
| [cpm86-cmdtools](https://github.com/tsupplis/cpm86-cmdtools) | CP/M-86 `.cmd` file manipulation tools |
| [cpm86-ports](https://github.com/tsupplis/cpm86-ports)       | CP/M-86 application ports in C and assembler |
| [cpm86-vi](https://github.com/tsupplis/cpm86-vi)             | STevie vi port for CP/M-86 and PC-DOS 1.1 |
| [cpm86-msbasic](https://github.com/tsupplis/cpm86-msbasic)   | A recreaction of msbasic-86 for CP/M-86 and PC-DOS 1.1 from gwbasic sources |
| [cpm86-menu](https://github.com/tsupplis/cpm86-menu)   | A submit replacement and a menu system for submit orchestration for CP/M-86 1.1 |
| [cpm86-make](https://github.com/tsupplis/cpm86-make)   | A simple submit based make tool for CP/M-86 1.1 |
| [pcdos11-hacking](https://github.com/tsupplis/pcdos11-hacking) | PC-DOS 1.1 distribution, tools and notes |
