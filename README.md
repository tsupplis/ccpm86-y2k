# Concurrent CP/M-86 & Concurrent DOS Y2K Patches

This projects regroups patch versions of Concurrent Y2K OS utilties.

Currently the supported OSes are
- Concurrent CP/M-86 3.1
- Concurrent DOS 3.2
- Concurrent DOS 4.1
- DOS Plus 1.2

Only the PC/XT/AT targets have been tested. I suspect they should work with compupro or others too. I plan to simulators for compupro and rc759 but any help would be welcome.

| Utility      | CCP/M 3.1 | CDOS 3.2  | CDOS 4.1  | DOS Plus 1.2|
|--------------|-----------|-----------|-----------|-------------|
| SDIR         | Fixed     | Fixed     | Fixed     | Fixed       |
| DATE         | Fixed     | Compliant | Compliant | Compliant   |
| SHOW         | Fixed     | Fixed     | Compliant | Compliant   |
* Fixed indicates that one of the tools need to be updated on the OS distribution.

- Concurrent DOS XM 6.21 seems fully compliant

You can go to https://github.com/tsupplis/cpm86-kernel and https://github.com/tsupplis/cpm86-hacking for CP/M-86 1.1 Y2K compliance.

## Where to find CP/M-86 and Concurrent CP/M-86?

The source for CP/M-86 and Concurrent CP/M-86, sources and binaries is http://www.cpm.z80.de.

A cleaned-up distribution and kernel is available at https://github.com/tsupplis/cpm86-kernel. This distribution is working well in virtual environments, patched with all known patches, 'y2k' friendly (it contains the version of tod which sources are in this project) and AT friendly.

## Binaries

The `binaries/` directory contains all the compiled CP/M-86 `.cmd` executables. The Y2K-patched tools are listed in the table above. The full set of rebuilt binaries is:

| Binary        | Description                          |
|---------------|--------------------------------------|
| `date.cmd`    | Date display/set utility (Y2K fixed) |
| `dir.cmd`     | Directory listing                    |
| `ed.cmd`      | Line editor                          |
| `era.cmd`     | File erase utility                   |
| `eraq.cmd`    | File erase with query                |
| `gencmd.cmd`  | Generate .cmd from .h86              |
| `help.cmd`    | Help utility                         |
| `pip3.cmd`    | Peripheral interchange program (v3)  |
| `pip4.cmd`    | Peripheral interchange program (v4)  |
| `ren.cmd`     | File rename utility                  |
| `sdir.cmd`    | Extended directory (Y2K fixed)       |
| `set.cmd`     | File attribute set utility           |
| `show.cmd`    | Disk space display (Y2K fixed)       |
| `submit.cmd`  | Submit batch file utility            |
| `systat.cmd`  | System status utility                |
| `tcopy.cmd`   | Track copy utility                   |
| `tod.cmd`     | Time-of-day utility                  |
| `type.cmd`    | File type/display utility            |
| `vcmode.cmd`  | Video console mode utility           |

## Sources and Build

The sources are written in Intel PL/M-86 and ASM-86, located in the `src/` directory. A `Makefile` is provided to rebuild all tools.

### Dependencies

The build relies on the cross-development toolchain from:

- **[cpm86-crossdev](https://github.com/tsupplis/cpm86-crossdev)** — provides the assemblers, linker, and CP/M-86 build utilities used by the Makefile (`cpm_asm86`, `cpm_gencmd`, `pcdev_rasm86`, `pcdev_linkcmd`)

> **Caveat:** Intel PL/M is not yet integrated into the build toolchain. The `PLM86` variable in the Makefile references `intel_plm86` which must be provided separately. (in plan for incorporation in cpm86-crossdev)

### Building

```sh
cd src
make
```

The Makefile targets:

- **`all`** — builds all `.cmd` binaries and low-level snippets (`boot.h86`, `load.h86`, `lbdos.h86`, `lbdos3.sys`)
- **`clean`** — removes all generated object and binary files
- **`ccpmtest.img`** — creates a test CP/M disk image with all tools copied onto it

### Code Pattern Fixed for Y2K

The fix applied across all affected tools is the same — a year overflow correction on date display:

```plm
emit$date$time: procedure;
    if tod.opcode = 0 then
      do;
      call emitn(.day$list(shl(week$day,2)));
      call emitchar(' ');
      end;
    call emit$slant(month);
    call emit$slant(day);
    if year>99 then
    do;
        year=year-100;
    end;
    call emit$bin$pair(year);
    call emitchar(' ');
    call emit$colon(hrs);
    call emit$colon(min);
    if tod.opcode = 0 then
      call emit$bcd$pair(sec);
    end emit$date$time;
```

The fix is on date display > 99. The date command code is just a bit more involved. The fixes mimic the Y2K fixes of CP/M 3.0 for 8080.
