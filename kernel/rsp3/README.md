# PCTMP resident system process

PCTMP is a separate optional PC-MODE RSP. Native TMP stays in rsp1; hardware
services belong to the single XIOS module in kernel/con.

The build now follows the other assembly RSPs: **ASM86 → H86 → GENCMD**, then
rename pctmp.cmd to pctmp.rsp. GENCMD creates the group headers and record padding.
There is no NASM step or Python RSP packager in this module's Makefile.

From the repository root, choose a fresh sandbox output:

```sh
make -C kernel/rsp3 OUTPUT="$PWD/private/pcmode/builds/rsp3-next"
```

PCMODE=1 (default) selects the supported extensions. PCMODE=0 reconstructs the
original complete RSP byte for byte, including its header and DATA. Mode changes
are tracked explicitly even with coarse CP/M output timestamps. Unchanged builds
reuse the existing output. All copied sources, configuration, H86, listings and
RSP files stay in OUTPUT.

`pctmp.a86` is native ASM86 source, with helpers in pcguard.lib, pcshell.lib,
pchelp.lib and pcsearch.lib. Undecoded historical ranges remain clearly marked
DB data; they are not presented as reconstructed instructions. pcdata.lib
explicitly declares the original normalized 2,800-byte DATA template. Initialized
CODE sidecars ensure GENCMD retains their full storage extent. Pinned reference
inputs and provenance remain under inputs/.

`tools/pctmpcheck.py` verifies group layout, DATA preservation and allowed patch
locations; it does not build or package an RSP. `tools/pcmodeverify.py` builds
PCMODE=0 through Make and compares the complete result to the original template.
Historical XIOS/label checks and DOS test probes still use NASM separately.

The top-level `make pcmode` includes this RSP with the normal RSP1/RSP2 services
in isolated SYS generation; native TMP is not also selected. See the
[PC-MODE build and checks](../pcmode/README.md) and [status](../pcmode/status.md).
