# Concurrent CP/M-86 3.1 Kernel and Tools

This project preserves Digital Research's Concurrent CP/M-86 3.1 as a usable,
rebuildable system for running and exploring classic 16-bit software. Runnable
disks and development tools accompany the sources and patch notes, making the
system useful today and accessible to anyone interested in its history.

![Concurrent CP/M-86 3.1 running in PCE](images/ccpm86.png)

| Goal | Distribution |
| --- | --- |
| Full source distribution | Kernel, eleven RSPs, most commands; gaps below |
| Additional tools | ASM80, HEXCOM, DREDIX, DR development chain, Microsoft BASIC 5.50A |
| Y2K components | SDIR, DATE, SHOW, TOD for CCP/M-86 3.2 through Concurrent DOS 4.1; 3.1 fixes included |

## Distributions

| Image | Bootable | Contents |
| --- | --- | --- |
| `ccpm86.img` | Yes | Kernel, stock commands, ED, HELP, SYSTAT, configuration/maintenance utilities, reconstructed SYSDISK |
| `ccpmtools.img` | Yes | DREDIX, INITDIR, ASM86, DDT86, GENCMD, GENCCPM, TCOPY, PIP, TOD |
| `ccpmdev.img` | No | DR development/BASIC tools, Microsoft BASIC, ASM80, HEXCOM, GENTAB |

`make dist` places the images above and `disks.zip` containing all three at the repository root.

320 KB: **untwist → cpmtools `ccpm86-320` → twist**.
[Media recipes and emulator usage](media/README.md).

## Tools

Format: [cpm86-kernel](../cpm86-kernel/README.md). **3.1** = rebuildable sources; SYSDISK: byte-identical CMD plus resident version.

| Tool | Binary (`base/`) | Source (`commands/`) |
| --- | --- | --- |
| `asm86` | — | 3.1 |
| `cls` | — | Resident; VT52 ESC-E (`kernel/custrsps/`) |
| `chset` | — | 3.1 |
| `config` | Original | — |
| `date` | — | 3.1, Y2K patched |
| `ddt86` | — | 3.1 |
| `dir` | — | 3.1; command/RSP |
| `dskmaint` | Original | — |
| `ed` | — | 3.1 |
| `era` / `eraq` | — | 3.1 |
| `function` | Original | — |
| `gencmd` | — | 3.1 |
| `genccpm` | — | 3.1; flag-table fix |
| `gentab` | — | 3.1 |
| `hdmaint` | Original | — |
| `help` / `help.hlp` | — | 3.1; indexed from `help.dat` |
| `initdir` | — | 3.1 |
| `pcetime` | Original | — |
| `pip` | — | PIP3/PIP4; ships PIP4 as 3.1N |
| `ren` | — | 3.1 |
| `sdir` | — | 3.1, Y2K/DPB/tail fixes |
| `set` | — | 3.1 |
| `setup` | Original | — |
| `show` | — | 3.1, Y2K patched |
| `submit` | — | ASM-86 source |
| `sysdisk` | — | ASM-86 reconstruction; byte-identical to `sysdisk.ref` |
| `systat` | — | 3.1 |
| `tcopy` | — | ASM-86 source in `boot/` |
| `tod` | — | 3.1, Y2K patched |
| `type` | — | 3.1 |
| `vcmode` | — | 3.1 |
| `asm80` | — | DR sources; CP/M-86 conversion |
| `hexcom` | — | HEXCOM 3.00-compatible C from [tpzasm](https://github.com/johnsonjh/tpzasm) |
| `dredix` | — | DR EDIX C; BDOS backend |

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
GENCCPM flag records, PIN and build corrections. Graphics remain CGA-oriented;
no general application INT 10h layer is provided.

## Build and checks

Build: [cpm86-crossdev](https://github.com/tsupplis/cpm86-crossdev), make,
cpmtools, Python 3, Git, tar, zip. Emulators: PCE (interactive), QEMU (checks).

| Command | Result |
| --- | --- |
| `make` | Commands, kernel/RSPs, boot sources, three images |
| `make -C commands/sysdisk check` | Byte-identical SYSDISK reconstruction |
| `make check` | SYSDISK identity, every packaged file, boot tracks, sizes |
| `make check-qemu` | Boot, RTC, status, switching, SDIR, SYSDISK, DREDIX |
| `make test` | Launch PCE |
| `make dist` | `y2k-dist.zip`, `disks.zip` and the three disk images at the repository root |
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
