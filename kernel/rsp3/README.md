# PCTMP resident system process

PCTMP is the default PC-MODE RSP. Native TMP remains available in rsp1;
hardware services belong to the single XIOS module in kernel/con.

Build with `make -C kernel/rsp3`, or use the normal top-level `make` to
build commands, kernel and distribution disks. RSP3 follows the other RSPs:
ASM86 → H86 → GENCMD, then rename CMD to RSP. Outputs are local to the module.

The normal kernel build includes rsp3, enables PCMODE_XIOS, and selects PCTMP
instead of TMP during SYSGEN. OS placement is 7008h to leave room for the
larger personality and console buffers below video memory.

`pcdefs.lib` selects the supported extensions. Setting PCMODE_BUILD to zero
reconstructs original PCTMP; this is for reference work, not the default build.
Undecoded historical ranges remain explicitly represented as DB data.

Experimental tools, reference binaries, tests and investigation documentation
are kept in private/pcmode. They are not dependencies of the normal build.
