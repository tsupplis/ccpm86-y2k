# PCTMP resident system process

PCTMP is the default PC-MODE RSP. Native TMP remains available in rsp1;
hardware services belong to the single XIOS module in kernel/con.

Build with `make -C kernel/rsp3`, or use the normal top-level `make` to
build commands, kernel and distribution disks. RSP3 follows the other RSPs:
ASM86 → H86 → GENCMD, then rename CMD to RSP. Outputs are local to the module.

The normal kernel build produces both ccpm.sys with PCTMP/PC-MODE XIOS and
ccpm-native.sys with native TMP/XIOS. These become the bootable ccpm86.img and
ccpmnative.img disks. The native SYSGEN staging directory keeps its RSP selection
separate. OS placement is 7008h, leaving console buffers below video memory.

All fixes and extensions are mandatory. There is no historical-build switch.

Experimental tools, tests and investigation documentation
are kept in private/pcmode. They are not dependencies of the normal build.
