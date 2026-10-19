# Reproducible 320 KB media

Run `make` here (or `make -C media` from the repository root). The build uses
`base.img`, the empty **physical/CHS** 320 KB floppy with its working boot track, as its boot
loader template. Keep that input file: it is never modified or removed by
`make clean`. The loader does not need rebuilding for this media target.

The build:

1. Builds the commands, console modules, RSPs, and `GENCCPM`, then generates
   `kernel/sysgen/ccpm.sys`. It also builds `HELP.HLP` from `HELP.DAT`.
2. Copies the empty `base.img` template to `ccpm86base.img`.
3. Untwists that empty base, populates it with **cpmtools format
   `ccpm86-320`**, then twists
   it back to physical order. Do not run cpmtools directly on a raw image.

Outputs (each exactly 327,680 bytes):

- `ccpm86.img`: rebuilt kernel, startup clock display, everyday commands,
  the indexed help file, and CONFIG, FUNCTION, PCETIME, and SYSDISK from `../base/`.
  `PIP.CMD` is the CP/M-oriented PIP3 build.
- `ccpmtools.img`: the same kernel plus assemblers, debugger, generators,
  HEXCOM, PIP, TOD, and TCOPY, plus DSKMAINT, HDMAINT, and SETUP from
  `../base/`. This disk can also boot by itself.
- `ccpmtest.img`: empty scratch disk, created by `make ccpmtest.img` or
  `make test`.

`STARTUP.0` runs `A:TOD`; clock initialization comes from the kernel's BIOS
RTC support. The system and temporary drive settings come from
`kernel/sysgen/genccpm.in`, including when two floppy drives are present.

All image assembly steps are ordinary Makefile recipes; the only Python
needed for image assembly is the existing track-order conversion tool.

## Verification and emulators

`make test` (also `make check`) builds the system and scratch disks and
starts the existing PCE launcher. PCE is the default emulator.

The optional `make check-qemu` builds the disks and boots both in QEMU with two drives. It
checks the `A>` prompt, BIOS date/time, yellow status line, and a `DIR`
command. QEMU runs with `-snapshot`, so verification does not modify media.

`make test` starts the existing PCE launcher, with the system disk in A:
and the scratch disk in B:. `./ccpm86 vga` can also launch it directly.

For a manual QEMU session:

```sh
qemu-system-i386 -m 16 -no-fd-bootchk -boot a \
  -drive file=ccpm86.img,format=raw,if=floppy,index=0 \
  -drive file=ccpmtools.img,format=raw,if=floppy,index=1
```

`-no-fd-bootchk` is necessary because the DRI media byte at offset 511 is
`01`, rather than the final byte of a DOS boot signature.

`make check-media` independently checks image sizes, preserved boot tracks,
and embedded kernel contents. The optional QEMU check also validates the
64 KB kernel data limit and RTM flag entries. Directory/data buffers are
configured as four/two per shared pool to keep the data and RSP area below
64 KB. A malformed flag table previously let the scheduler interpret
`FF00` as a process address and crash before the prompt.

To inspect a generated disk:

```sh
python3 ../tools/cpm86twist.py untwist ccpm86.img /tmp/ccpm-logical.img
cpmls -f ccpm86-320 /tmp/ccpm-logical.img
```

After making changes with cpmtools, run `twist` on the logical copy to
produce a physical image. Running `make` again discards manual edits to
its generated outputs.
