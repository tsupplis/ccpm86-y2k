# Kernel architecture

CCP/M-86 3.1, 8086 real mode. Components are build/ABI boundaries, not protected
address spaces. Arrows show principal service dependencies, not an exhaustive call graph.

## Runtime components

```mermaid
graph TB
    app_native["Native applications"]
    app_dos["DOS applications"]
    shell_tmp["TMP: native command processor"]
    shell_pctmp["PCTMP: shell and DOS personality"]
    services["Resident services: PIN, VOUT, serial, CLOCK, ABORT, DIR, ECHO"]
    kernel_sup["SUP: routing, loader, CLI"]
    kernel_rtm["RTM: processes, scheduling, queues, flags"]
    kernel_mem["MEM: allocation and sharing"]
    kernel_cio["CIO: console and character I/O"]
    kernel_bdos["BDOS: filesystem and disk services"]
    kernel_xios["XIOS: drivers, virtual screens, IRQs"]
    kernel_sd["SYSDAT: shared tables and module vectors"]
    hardware["PC hardware and firmware"]

    app_native -->|INT E0h| kernel_sup
    shell_tmp -->|INT E0h| kernel_sup
    shell_pctmp -->|INT E0h| kernel_sup
    shell_tmp -.->|launch CMD via SUP| app_native
    shell_pctmp -.->|launch CMD via SUP| app_native
    shell_pctmp -.->|load COM or EXE| app_dos
    app_dos -->|INT 21h and selected BIOS interrupts| shell_pctmp
    services --> kernel_sup
    kernel_sup --> kernel_rtm
    kernel_sup --> kernel_mem
    kernel_sup --> kernel_cio
    kernel_sup --> kernel_bdos
    kernel_cio --> kernel_xios
    kernel_bdos --> kernel_xios
    kernel_sd -.->|shared ABI| kernel_sup
    kernel_sd -.->|shared ABI| kernel_xios
    services -->|XIOS interface| kernel_xios
    services -.->|input and spool queues| kernel_cio
    kernel_cio -.->|queued I/O| services
    kernel_xios -->|flags, waits, dispatch| kernel_sup
    kernel_xios --> hardware
```

TMP and PCTMP are alternatives, one configured command processor per console.
Dotted launch edges show process lifecycle; solid interrupt edges show runtime
service calls. Native applications call the kernel directly, without TMP/PCTMP
forwarding their calls. PCTMP additionally supplies resident DOS handlers, executed
in the calling DOS application's context; INT 21h is not IPC to the shell process.

## Component responsibilities

| Component | Responsibility | Interface / relationship |
| --- | --- | --- |
| Native application | A CP/M/CCP/M program, typically `.CMD`; e.g. ED or ASM86 | Targets the native API through `INT E0h`; launched via SUP's loader/CLI services |
| DOS application | A DOS `.COM` or MZ `.EXE`; e.g. MSMASM or LINK | Loaded by PCTMP; uses its `INT 21h` and supported BIOS emulation |
| TMP | Native prompt, command parsing and built-ins | Requests native command execution from SUP; waits for foreground completion |
| PCTMP | TMP alternative plus DOS loading and resident DOS personality | Launches native commands through SUP; owns the DOS loader, `INT 21h` dispatch and interrupt-vector state |
| SUP | Kernel service entry, routing, loading and CLI services | Receives `INT E0h`; routes calls to the implementing kernel module |
| RTM | Processes, scheduling, queues, flags and synchronization | Manages application lifecycle; invokes the existing user-dispatch callback for PCTMP vector switching |
| MEM | Memory regions, allocation and sharing | Supplies native allocations used by both native loading and PCTMP's DOS loader |
| CIO | Console and character-device semantics | Connects application I/O to XIOS and queue-driven resident services |
| BDOS | FCB filesystem and disk services | Owns native file semantics; PCTMP translates supported DOS requests to these services |
| XIOS | PC device drivers, virtual screens, interrupts and hardware adaptation | Serves kernel/RSP calls; experimental legacy functions 31–34 support PCTMP's BIOS personality |
| SYSDAT | Shared module vectors, descriptors, queues and generated configuration | Common kernel ABI/data; not a separately scheduled service |
| PIN / VOUT | Physical input and switch events / buffered background output | Resident processes using native queues and XIOS; VOUT also manages spool files |
| Other service RSPs | Serial I/O, clock, abort, resident directory/echo services | Scheduled native processes, using kernel services and applicable XIOS interfaces |
| GENCCPM | Offline system composition | Combines CON/RSP inputs, configuration and OS label into `CCPM.SYS` |
| Boot loader | Load and enter the generated system | Transfers control to kernel initialization, which starts resident processes |
| Hardware / firmware | Devices, IRQ sources and BIOS routines | Primarily owned by XIOS; real mode also permits applications to access hardware directly |

`INT E0h` is the native service entry; BDOS is one implementing module behind it.
`INT 21h` is the DOS API entry implemented inside PCTMP, not a second kernel BDOS.
The command prompt may wait while an application runs, but its resident PCTMP
handler code remains available. Built-ins execute in the command processor itself.

## Experimental PC-MODE decomposition

```mermaid
flowchart LR
    DOS["DOS COM / EXE"] -->|"INT 21h / BIOS interrupts"| TRAP

    subgraph P["PCTMP RSP"]
        TRAP["Interrupt handlers"] --> DOSAPI["DOS-to-CP/M services"]
        CLI["Command processor"] --> LOAD["COM / MZ loader"]
        LOAD --> GUARD["Validation + image-reader helpers"]
        TRAP --> BREAK["Checked input + Ctrl-C continuation"]
        VECT["Interrupt-vector save / restore"]
    end

    DOSAPI -->|"INT E0h"| OS["Existing kernel services"]
    LOAD -->|"INT E0h"| OS
    OS -->|"existing RTM user-dispatch callback"| VECT
    TRAP -->|"restored legacy XIOS 31–34"| BRIDGE

    subgraph X["XIOS"]
        BRIDGE["BIOS compatibility bridge"] --> NATIVE["Native screen / cursor / ownership helpers"]
        BRIDGE --> VIDEO["Guarded video firmware call"]
        IRQ["Existing IRQ / init paths"] --> ADAPT["Keyboard latches · BIOS clock · saved video vector"]
    end

    VIDEO --> BIOS["Pre-PCTMP INT 10h handler"]
    ADAPT --> BDA["BIOS data area / XIOS-private state"]

    classDef added fill:#fff3cd,stroke:#8a6800,color:#222;
    class GUARD,BREAK,BRIDGE,ADAPT added;
```

Amber components are additions/adaptations within their owning module. The
RTM callback and PCTMP→XIOS relationship predate this port. Firmware video calls
use the saved vector to avoid `PCTMP → XIOS → INT 10h → PCTMP` recursion.

## System composition

```mermaid
flowchart LR
    CORE["SUP · RTM · MEM · CIO · BDOS<br/>SYSDAT · XIOS<br/>.CON inputs"] --> GEN["GENCCPM"]
    SERVICE["Service .RSP inputs"] --> GEN
    SHELL["TMP or PCTMP .RSP"] --> GEN
    CONFIG["Configuration · OS label"] --> GEN
    GEN --> SYS["CCPM.SYS<br/>relocated modules · shared RSP CODE<br/>per-instance DATA · generated tables"]
    SYS --> BOOT["Boot loader → kernel INIT → resident processes"]
```

## Integrity assessment — `pc-tail` checkpoint

- Six native `.CON` inputs are byte-identical; XIOS is the changed core module.
  Eight service `.RSP` inputs are unchanged. PCTMP replaces TMP.
- PCTMP's DATA template/stride is unchanged. Its CODE has 49 changed original
  bytes plus 1,408 appended bytes. New mutable CODE state is console-indexed.
- Module responsibilities are retained; internal call graphs intentionally differ.
  Historical implementation equivalence is not claimed.
- Before permanent integration: review shared-CODE state, fixed four-console
  storage, reentrancy/lock contracts, and firmware/BIOS-state lifecycle.

Detailed evidence and review decisions: [PC-MODE module audit](private/pcmode/MODULES.md).
Current validation and remaining work: [PC-MODE status](private/pcmode/STATUS.md).
