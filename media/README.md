# Reproducible 320 KB media

Run `make` here, or `make -C media` from the repository root.
Root `make dist` copies the six distribution images to the root alongside
`y2k-dist.zip` and `disks.zip`.

| Image | Bootable | Contents |
| --- | --- | --- |
| `ccpm86.img` | Yes | PC-MODE/PCTMP kernel, core commands, ED, full HELP, BATCH, SYSDISK, SETDEF, PIP |
| `ccpmnative.img` | Yes | Native TMP/XIOS alternative, with the same core commands and shared kernel fixes |
| `ccpmtools.img` | No | ASM86, DDT86, GENCMD, GENCCPM, WINDOW/WMENU, sample SUB files, PIP, TOD |
| `ccpmext.img` | No | PUT, GET, REDIR (console redirection), DREDIX, PIP |
| `ccpmmaint.img` | Yes | PC-MODE kernel, disk maintenance tools, CONFIG, FUNCTION, SETUP, SYSTAT, PIP, TOD |
| `ccpmdev.img` | No | Native DR development/BASIC tools, Microsoft BASIC CMD, ASM80, HEXCOM, GENTAB |
| `ccpmdos.img` | No | DOS MASM, LIB, LINK, LINKCMD, LINKEXE, EXE2BIN and Microsoft BASIC COM |

Each image is 327,680 bytes. `PIP.CMD` is PIP4 with the `3.1N` banner.
`base.img` is the empty physical/CHS filesystem template and original boot
reference; `make clean` leaves it intact. Every generated image receives the
source-built `../boot/boottrack.bin`, verified byte-for-byte against that reference.
Native BASIC is supplied as dev/mbasic.cmd; DOS BASIC as dev/mbasic.com.


Root `make` builds both kernels in one run: `kernel/sysgen/ccpm.sys` (PC-MODE)
and `kernel/sysgen/ccpm-native.sys` (native TMP). The native floppy installs its
kernel as `CCPM.SYS`, as required by the normal boot loader. Both personalities
use the same XIOS implementation and shared kernel fixes. Native XIOS disables
the PC-MODE gateway; this is a source-built alternative, not an archival binary.
Boot either system disk in A: and use the data disks in B:. DOS programs require
the PC-MODE system. Configuration utilities live on maintenance to keep both
system disks within 320 KB.

## Build pipeline

| Step | Operation |
| --- | --- |
| Sources | Build commands, kernel components and all nine RSPs; run GENCCPM |
| Boot | Assemble the boot sector and loader; build and verify the complete 4 KB track |
| Reference check | Require rebuilt SYSDISK and DSKMAINT to match their originals byte-for-byte; run dual-OS DFORM checks |
| Help | Compile `HELP.DAT` into indexed `HELP.HLP` |
| Template | Copy empty `base.img`, then install the source-built boot track |
| Populate | Untwist → cpmtools format `ccpm86-320` → twist back |
| Verify | Extract and compare every packaged file; compare boot tracks with the source-built track and verify sizes |

Recipes are in the Makefiles. `h862bin` from the companion `cpm86-cmdtools`
extracts the assembled boot binaries; `../tools/cpm86twist.py` converts floppy track order.
[Boot sources and track layout](../boot/README.md).
Never use cpmtools directly on a physical 320 KB image: head 1 has reversed cylinder order.

## Run and verify

| Command | Action |
| --- | --- |
| `make test` / `make check` | Launch PCE |
| `./ccpm86 vga` | Launch the PCE VGA configuration |
| `make check-media` | Verify all six distributions and their payloads |
| `make check-qemu` | Boot all three system disks; exercise consoles, HELP, SDIR, SYSDISK, SETDEF, DREDIX and windows |

For WINDOW and WMENU, mount `ccpmtools.img` in B: and run `B:WINDOW VIEW`
or `B:WMENU`. DSKMAINT, DFORM, DCOPY, DCOMP, HDMAINT and INITDIR are on the separate bootable maintenance
disk, leaving room for the full HELP on A: and the window tools on B:.

Window layout samples are packaged alongside WINDOW.CMD:

| Sample | Purpose |
| --- | --- |
| `CARDDECK.SUB` | Arrange four overlapping windows |
| `CLEAR.SUB` | Restore all six consoles to full-screen windows |
| `GLASS.SUB` | Arrange mixed-size windows and select console 1 |
| `WORDPROC.SUB` | Set up a large editing window and three small windows |

Select B: before running a sample so its WINDOW commands find WINDOW.CMD.
SUBMIT remains on the main disk:

```text
A>B:
B>A:SUBMIT CARDDECK
B>A:SUBMIT CLEAR
```

For DREDIX, PUT, GET and REDIR, mount `ccpmext.img` in B: and run e.g. `B:DREDIX`.
Mount `ccpmdev.img` as a data disk after booting any system disk.
`STARTUP.0` runs `A:TOD`; kernel RTC support supplies startup date/time.
Drive defaults, allocation settings and buffers are in
`../kernel/sysgen/genccpm.in`; patch details are in `../PATCHES.md`.

A manual QEMU session:

```sh
qemu-system-i386 -m 16 -no-fd-bootchk -boot a \
  -drive file=ccpm86.img,format=raw,if=floppy,index=0 \
  -drive file=ccpmtools.img,format=raw,if=floppy,index=1
```

The DRI media byte at offset 511 is `01`, so `-no-fd-bootchk` is required.
Automated QEMU checks use snapshots and leave images unchanged.

Inspect a generated disk:

```sh
python3 ../tools/cpm86twist.py untwist ccpm86.img /tmp/ccpm-logical.img
cpmls -f ccpm86-320 /tmp/ccpm-logical.img
```

After manual cpmtools edits, twist the logical copy back to physical order.
The next `make` replaces generated images.

## Console shortcuts

| Keys | Action |
| --- | --- |
| Ctrl+Alt+top-row 1–4 | Select console 0–3 |
| Ctrl+Alt+Tab | Next console, wrapping from 3 to 0 |
| Ctrl+numeric keypad | Original selection aliases |

All use the normal switch events and honor console ownership/restrictions.
