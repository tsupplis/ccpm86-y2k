# PCTMP resident system process

PCTMP is a separate optional PC-MODE RSP. Native TMP stays in rsp1; hardware
services belong to the single XIOS module in kernel/con.

The build now follows the other assembly RSPs: **ASM86 → H86 → GENCMD**, then
rename pctmp.cmd to pctmp.rsp. GENCMD creates the group headers and record padding.
There is no NASM step or Python RSP packager in this module's Makefile.

From the repository root:

```sh
make -C kernel/rsp3
```

The Makefile uses the same local `SNIPPETS`, pattern rules and `clean` structure
as the other assembly RSP modules. Outputs are local to the build directory.
There is no OUTPUT setting, source-copying wrapper or dependency on private.

The checked-in pcdefs.lib selects supported extensions with PCMODE_BUILD=0ffffh.
Reconstruction checks set it to 0 only in a temporary source copy. Sandbox tools
copy sources into their chosen directory and invoke ordinary Make there. For an
isolated complete personality build, use `make pcmode`; its default output is
build/pcmode. The private sandbox is optional and disposable.

`pctmp.a86` is native ASM86 source, with helpers in pcguard.lib, pcshell.lib,
pcerror.lib, pchelp.lib and pcsearch.lib. Undecoded historical ranges remain clearly marked
DB data; they are not presented as reconstructed instructions. pcdata.lib
explicitly declares the original normalized 2,800-byte DATA template. Initialized
CODE sidecars ensure GENCMD retains their full storage extent. Pinned reference
inputs and provenance remain under inputs/.

`tools/pctmpcheck.py` verifies group layout, DATA preservation and allowed patch
locations; it does not build or package an RSP. `tools/pcmodeverify.py` builds
the original configuration through Make in a temporary copy and compares the
complete result to the original template.
Historical XIOS/label checks and DOS test probes still use NASM separately.

The top-level `make pcmode` includes this RSP with the normal RSP1/RSP2 services
in isolated SYS generation; native TMP is not also selected. See the
[PC-MODE build and checks](../pcmode/README.md) and [status](../pcmode/status.md).
