# Source-built 320 KB boot track

`make` assembles the boot sector and loader and creates `boottrack.bin`.
`make check` compares all 4,096 bytes with the original track in
`../media/base.img`. The media build installs this generated track before
untwisting and populating the disk images.

| File | Purpose |
| --- | --- |
| `bootsect.a86` | 147-byte first stage, entered by the PC BIOS at 0000:7C00 |
| `loader.a86` | 846-byte second stage; loads CPM.SYS or CCPM.SYS |
| `Makefile` | Assemble, form the complete track, verify and clean |
| `boottrack.bin` | Generated 4,096-byte reserved track; not a CMD file |

Both sources come from the reconstructed DSKMAINT boot images in the companion
`cpm86-kernel` project (`commands/dskmaint/`). The loader here resets its
missing-extent counter to **3** after a matching extent instead of **2**. This
one-byte difference reproduces the loader already used by our disks.

## Track layout

| Offset | Length | Contents |
| --- | ---: | --- |
| `000h` | 147 bytes | Boot-sector code and messages |
| `1FFh` | 1 byte | Double-sided media marker, `01h` |
| `900h` | 846 bytes | Loader code and data |
| Other bytes | — | `E5h` filler |

The first stage reads sectors 5–8 into a segment 8 KB below the top of memory,
then enters the loader at offset 100h. The second stage reads the CP/M
filesystem using the disk's single/double-sided geometry, including reversed
head-1 cylinder numbering. It looks for CPM.SYS first and then CCPM.SYS, reads
the CMD header and loads the system into its specified memory location.

Only the 320 KB, double-sided track is built here. The DRI marker occupies the
last byte of the boot sector, so QEMU needs `-no-fd-bootchk`.

The earlier BOOT/LBDOS/LOAD implementation, its alternative LBDOS3, old source
snapshots and obsolete emulator recipes have been removed. TCOPY was also
removed: it required CP/M-86 direct BIOS calls and was not usable under CCP/M-86.
