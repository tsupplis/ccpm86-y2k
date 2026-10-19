# Concurrent CP/M-86 3.1 Kernel and Tools

Source-built CCP/M-86 3.1 for the IBM PC/XT, with modern host builds,
320 KB floppy images, additional development tools, and Y2K utilities.
The full-source goal is in progress: binary-only commands are listed below.

| Focus | Distribution |
| --- | --- |
| Rebuild CCP/M-86 3.1 from source | Kernel, all nine RSPs, and most stock commands; SYSDISK reconstructed byte-for-byte |
| Extend the toolset | ASM80, HEXCOM, DREDIX, Digital Research development tools, and Microsoft BASIC 5.50A |
| Supply Y2K components | SDIR, DATE, SHOW, TOD for the 3.2 family through Concurrent DOS 4.1, plus 3.1 fixes |

## Distributions

| Image | Bootable | Contents |
| --- | --- | --- |
| `media/ccpm86.img` | Yes | Kernel, stock commands, ED, HELP, SYSTAT, original configuration/maintenance tools, rebuilt SYSDISK |
| `media/ccpmtools.img` | Yes | DREDIX, INITDIR, ASM86, DDT86, GENCMD, GENCCPM, TCOPY, PIP, TOD |
| `media/ccpmdev.img` | No | RASM86, LINK86, LIB86, XREF86, SID86, CBAS86, CRUN86, Personal BASIC, Microsoft BASIC, ASM80, HEXCOM, GENTAB |
| `media/ccpmtest.img` | No | Empty scratch disk; created by `make test` |

All images are 320 KB. Assembly uses the empty, original-boot-track
`media/base.img`: untwist, populate with cpmtools format `ccpm86-320`, then
twist back. See [media/README.md](media/README.md) for PCE and QEMU usage.

## Repository layout

| Directory | Contents |
| --- | --- |
| `kernel/con/` | BDOS, CIO, MEM, RTM, SUP, SYSDAT, XIOS sources |
| `kernel/rsp1/`, `kernel/rsp2/` | Resident system process sources |
| `kernel/sysgen/` | GENCCPM configuration and generated `ccpm.sys` |
| `commands/` | Rebuildable utilities, including reconstructed SYSDISK |
| `base/` | Original binaries; SYSDISK byte-comparison reference |
| `dev/` | Imported Digital Research and Microsoft development binaries |
| `boot/`, `media/`, `tools/` | Loader sources, reproducible disk recipes, track conversion and guest checks |
| `doc/` | Reference programming and PL/M manuals |

## Tools

Same status-table format as [cpm86-kernel](../cpm86-kernel/README.md).
A source entry means the tool builds here; it does not imply binary identity
with an original. The SYSDISK comparison is enforced by `make check`.

| Tool | Binary (`base/`) | Source (`commands/`) |
| --- | --- | --- |
| `asm86` | — | CCP/M-86 3.1 |
| `chset` | — | CCP/M-86 3.1 |
| `config` | Original | — |
| `date` | — | CCP/M-86 3.1, Y2K patched |
| `ddt86` | — | CCP/M-86 3.1 |
| `dir` | — | CCP/M-86 3.1; command and resident process |
| `dskmaint` | Original | — |
| `ed` | — | CCP/M-86 3.1 |
| `era` / `eraq` | — | CCP/M-86 3.1 |
| `function` | Original | — |
| `gencmd` | — | CCP/M-86 3.1 |
| `genccpm` | — | CCP/M-86 3.1; flag-table serialization fixed |
| `gentab` | — | CCP/M-86 3.1 |
| `hdmaint` | Original | — |
| `help` / `help.hlp` | — | CCP/M-86 3.1; indexed help rebuilt from `help.dat` |
| `initdir` | — | CCP/M-86 3.1 |
| `pcetime` | Original | — |
| `pip` | — | PIP3 and PIP4; images use PIP4 with the `3.1N` banner |
| `ren` | — | CCP/M-86 3.1 |
| `sdir` | — | CCP/M-86 3.1, Y2K patched; DPB module and command-tail fixes |
| `set` | — | CCP/M-86 3.1 |
| `setup` | Original | — |
| `show` | — | CCP/M-86 3.1, Y2K patched |
| `submit` | — | ASM-86 source |
| `sysdisk` | Original | Reconstructed ASM-86; byte-identical |
| `systat` | — | CCP/M-86 3.1 |
| `tcopy` | — | ASM-86 source in `boot/` |
| `tod` | — | CCP/M-86 3.1, Y2K patched |
| `type` | — | CCP/M-86 3.1 |
| `vcmode` | — | CCP/M-86 3.1 |
| `asm80` | — | DR 1976–1978 sources converted to CP/M-86 |
| `hexcom` | — | DR HEXCOM 3.00-compatible C implementation from [tpzasm](https://github.com/johnsonjh/tpzasm) |
| `dredix` | — | DR EDIX C sources; BDOS keyboard/display backend, no private XIOS access |

### Development binaries

Imported binaries are in `dev/`; their sources are not part of this repository.
Versions below come from the shipped binaries. Import does not imply a full
guest compatibility audit.

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
| `mbasic86.cmd` | BASIC-86 recreation from [cpm86-msbasic](https://github.com/tsupplis/cpm86-msbasic) | 5.50A | Microsoft lineage |
| `mbas522.cmd` | Earlier BASIC-86 reference; omitted from the dev disk | 5.22 | Microsoft |

## Y2K utilities

The existing compatibility audit covers the following releases. Current
image regression tests boot CCP/M-86 3.1; they do not retest every later OS.

| Utility | CCP/M-86 3.1 | 3.2 family | Concurrent DOS 4.1 | DOS Plus 1.2 |
| --- | --- | --- | --- | --- |
| SDIR | Fixed | Fixed | Fixed | Fixed |
| DATE | Fixed | Compliant | Compliant | Compliant |
| SHOW | Fixed | Fixed | Compliant | Compliant |
| TOD | Fixed | Fixed | Compliant | Compliant |

Fixed = replace the distribution's utility. Compliant = its existing utility
already handles Y2K. The 3.2 family covers the CCP/M-86/Concurrent DOS work.
The changes correct year display and date input handling; native disk formats
are unchanged. Concurrent DOS XM 6.21 was previously reported compliant.

## Kernel and RSP patches

| Area | Current changes |
| --- | --- |
| Clock | BIOS INT 1Ah RTC read at startup; BDOS time changes written back when supported |
| Display | Bright-yellow status line; cursor height follows CGA/MDA/EGA/VGA character cells |
| Enhanced adapters | Skip CGA retrace/snow waits on EGA/VGA |
| Console selection | Ctrl+Alt+1–4; Ctrl+Alt+Tab cycles; keypad aliases retained |
| System configuration | Honor GENCCPM system/temporary drives; standalone CP/M BDOS build |
| System generation | Correct three-byte flag records; disk buffers fit the 64 KB data/RSP area |
| RSPs | Remove duplicate PIN constant; rebuild all nine resident processes |

Detailed changes, source locations, limitations, and verification are in
[PATCHES.md](PATCHES.md). Console graphics remain CGA-oriented; this is not
a general BIOS INT 10h compatibility layer.

## Build and checks

Requires [cpm86-crossdev](https://github.com/tsupplis/cpm86-crossdev),
`make`, cpmtools, Python 3, Git, tar and zip. PCE is the interactive emulator;
QEMU is needed only for automated guest checks.

| Command | Result |
| --- | --- |
| `make` | Build commands, kernel/RSPs, boot sources, and three distribution images |
| `make -C commands/sysdisk check` | Rebuild SYSDISK and compare every byte with `base/sysdisk.cmd` |
| `make check` | SYSDISK identity and media-content checks |
| `make check-qemu` | Boot checks, RTC, status color, console switching, SDIR, SYSDISK, DREDIX |
| `make test` | Launch PCE with the system disk and scratch disk |
| `make dist` | `dist.zip`, `y2k-dist.zip`, and `dist/ccpm86-source.tar.gz` |
| `make source-dist` | Source archive, reference inputs and dev binaries; no ignored build products |
| `make clean` | Remove generated outputs; preserve original inputs |

The source archive includes current tracked files and new, non-ignored
sources. Y2K ZIP contains SDIR, DATE, SHOW and TOD. Binary ZIP separates
source-built commands, original inputs, development binaries and media.

Source pedigree: [Unofficial CP/M Web Site](http://www.cpm.z80.de).
License correspondence is retained in [LICENSE.md](LICENSE.md).

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
