# API inventory

| Scope | Contract |
| --- | --- |
| Build | Current PC-MODE and native-TMP builds; 22 October 2026 |
| Numbers | Hexadecimal unless explicitly labelled decimal |
| Implemented | Handler present in current code; not exhaustive DOS conformance certification |
| Partial / stub / missing | Limited contract / no operation / no implementation |
| DOS files | FCB operations mapped to CP/M files; no FAT or DOS directories |
| TMP | Native command processor (`kernel/rsp1`); does not provide DOS interrupts |
| PCTMP | PC-MODE command processor, COM/EXE loader and DOS/BIOS interrupt adapters (`kernel/rsp3`) |
| XIOS | Shared hardware, console, keyboard, display, disk and printer implementation (`kernel/con`) |
| Extension column | NEW marks proposed work (or explicitly implemented project additions); — means no proposal recorded. Sources/dependencies are below |
| Native API | `INT E0h` is provided by the shared kernel, with either TMP or PCTMP |

## INT 21h — DOS services

| AH | Service | Reference version | Status | Limit / behaviour | Extension |
| --- | --- | --- | --- | --- | --- |
| 00 | Terminate | 1.10 / 1.25 | Implemented | Legacy termination; CS must identify PSP | — |
| 01 | Console input with echo | 1.10 / 1.25 | Implemented | Ctrl-C path implemented | — |
| 02 | Console output | 1.10 / 1.25 | Implemented | — | — |
| 03 | AUX input | 1.10 / 1.25 | Stub | Unconditional RET; not enabled by configuration | NEW |
| 04 | AUX output | 1.10 / 1.25 | Stub | Unconditional RET; not enabled by configuration | NEW |
| 05 | Printer output | 1.10 / 1.25 | Implemented | Native list output; assigned printer | — |
| 06 | Direct console I/O | 1.10 / 1.25 | Implemented | — | — |
| 07 | Console input without echo/check | 1.10 / 1.25 | Implemented | — | — |
| 08 | Console input without echo, Ctrl-C check | 1.10 / 1.25 | Implemented | Ctrl-C path implemented | — |
| 09 | Print dollar-terminated string | 1.10 / 1.25 | Implemented | — | — |
| 0A | Buffered console input | 1.10 / 1.25 | Implemented | Ctrl-C path implemented | — |
| 0B | Console input status | 1.10 / 1.25 | Implemented | — | — |
| 0C | Flush input and read | 1.10 / 1.25 | Implemented | AL=01h/06h/07h/08h/0Ah read selectors | — |
| 0D | Disk reset | 1.10 / 1.25 | Implemented | — | — |
| 0E | Select default drive | 1.10 / 1.25 | Implemented | — | — |
| 0F | FCB open | 1.10 / 1.25 | Implemented | FCB → CP/M filesystem; no FAT | — |
| 10 | FCB close | 1.10 / 1.25 | Implemented | FCB → CP/M filesystem; no FAT | — |
| 11 | FCB find first | 1.10 / 1.25 | Implemented | FCB → CP/M filesystem; no FAT | — |
| 12 | FCB find next | 1.10 / 1.25 | Implemented | FCB → CP/M filesystem; no FAT | — |
| 13 | FCB delete | 1.10 / 1.25 | Implemented | FCB → CP/M filesystem; no FAT | — |
| 14 | FCB sequential read | 1.10 / 1.25 | Implemented | FCB → CP/M filesystem; no FAT | — |
| 15 | FCB sequential write | 1.10 / 1.25 | Implemented | FCB → CP/M filesystem; no FAT | — |
| 16 | FCB create | 1.10 / 1.25 | Implemented | FCB → CP/M filesystem; no FAT | — |
| 17 | FCB rename | 1.10 / 1.25 | Implemented | FCB → CP/M filesystem; no FAT | — |
| 18 | Reserved/legacy in-use | Unused / stub | Unused | AL=00h; no service performed | — |
| 19 | Get default drive | 1.10 / 1.25 | Implemented | — | — |
| 1A | Set DTA | 1.10 / 1.25 | Implemented | — | — |
| 1B | Get FAT pointer / geometry, current drive | Implemented in checked IBM/MS source | Partial | Native geometry/allocation pointer; not a DOS FAT | NEW |
| 1C | Get FAT pointer / geometry, selected drive | Implemented in checked IBM/MS source | Missing | AL=00h; no service performed | NEW |
| 1D | Reserved/legacy read-only | Unused / stub | Unused | AL=00h; no service performed | — |
| 1E | Reserved/legacy attributes | Unused / stub | Unused | AL=00h; no service performed | — |
| 1F | Get current-drive DPB (DS:BX) | Implemented in checked IBM/MS source | Missing | AL=00h; no service performed | NEW |
| 20 | USERCODE placeholder | Checked dos.asm: USERCODE → MOV AL,0 / RET | Stub (matches source) | AL=00h; no user-code service | — |
| 21 | FCB random read | 1.10 / 1.25 | Implemented | FCB → CP/M filesystem; no FAT | — |
| 22 | FCB random write | 1.10 / 1.25 | Implemented | FCB → CP/M filesystem; no FAT | — |
| 23 | FCB file size | 1.10 / 1.25 | Implemented | FCB → CP/M filesystem; no FAT | — |
| 24 | Set FCB random record | 1.10 / 1.25 | Implemented | FCB → CP/M filesystem; no FAT | — |
| 25 | Set interrupt vector | 1.10 / 1.25 | Implemented | Vector replacement; not a promise of device virtualization | — |
| 26 | Create PSP | 1.10 / 1.25 | Implemented | PSP creation; does not allocate memory or EXEC | — |
| 27 | FCB random block read | 1.10 / 1.25 | Implemented | FCB → CP/M filesystem; no FAT | — |
| 28 | FCB random block write | 1.10 / 1.25 | Implemented | FCB → CP/M filesystem; no FAT | — |
| 29 | Parse filename into FCB | 1.10 / 1.25 | Implemented | — | — |
| 2A | Get date | 1.10 / 1.25 | Implemented | — | — |
| 2B | Set date | 1.10 / 1.25 | Implemented | — | — |
| 2C | Get time | 1.10 / 1.25 | Implemented | — | — |
| 2D | Set time | 1.10 / 1.25 | Implemented | — | — |
| 2E | Set verify-on-write | 1.10 / 1.25 | Stub | Unconditional RET; not enabled by configuration | NEW |
| 2F | Get DTA | Checked dos.asm: GETDMA, present in shared IBM/MS dispatcher | Implemented | ES:BX=current DTA | — |
| 30 | Version query | Project extension; analogous to local dos.asm DOSEXT | Implemented | AX=1901h: AL=1, AH=25 decimal; other registers preserved; no OEM/serial contract | NEW (implemented) |
| 31–FF | All higher functions | DOS 2.x and later | Missing | Shared fallback sets AL=00h; no DOS 2.x CF/error contract | — |

## DOS allocation interfaces — checked IBM/MS reference source

| AH | Handler | Input | Successful return | Current PCTMP |
| --- | --- | --- | --- | --- |
| 1B | GETFATPT → GETFATPTDL | Current drive (forces DL=0) | DS:BX=FAT, AL=sectors/cluster, CX=bytes/sector, DX=cluster count | Partial native substitute; not a DOS FAT |
| 1C | GETFATPTDL | DL=0 current, 1=A, 2=B, … | Same as 1B; invalid drive returns AL=FFh | Missing; fallback AL=0 |
| 1F | GETDSKPT | Current drive | DS:BX=DPB | Missing; fallback AL=0 |

## INT 21h — principal later-version gaps

| AH | Service | Status | Extension |
| --- | --- | --- | --- |
| 31 | Terminate and stay resident | Missing | — |
| 33 | Ctrl-Break control | Missing | — |
| 35 | Get interrupt vector | Missing | — |
| 36 | Free disk space | Missing | — |
| 38 | Country information | Missing | — |
| 39 / 3A / 3B | Make / remove / change directory | Missing | — |
| 3C / 3D / 3E | Create / open / close handle | Missing | NEW |
| 3F / 40 / 42 | Read / write / seek handle | Missing | NEW |
| 41 / 43 | Delete pathname / file attributes | Missing | NEW |
| 44 | IOCTL | Missing | NEW |
| 45 / 46 | Duplicate / force duplicate handle | Missing | NEW |
| 47 | Get current directory | Missing | — |
| 48 / 49 / 4A | Allocate / free / resize memory | Missing | NEW |
| 4B | Execute program | Missing; shell COM/EXE loading is separate | NEW |
| 4C / 4D | Terminate with status / get child status | Missing | NEW |
| 4E / 4F | Find first / next by pathname | Missing | NEW |
| 54 | Get verify flag | Missing | — |
| 56 / 57 | Rename pathname / file date and time | Missing | NEW |

## DOS and BIOS interrupts

| INT | Selector | Service | Status / limit | Implementation | Extension |
| --- | --- | --- | --- | --- | --- |
| 10 | AH=00 | Set video mode | Native mode bridge; text and CGA support; no blanket EGA/VGA DOS claim | PCTMP + XIOS | — |
| 10 | AH=01 | Set cursor shape | Implemented | PCTMP + XIOS | — |
| 10 | AH=02 | Set cursor position | Implemented; bounded to console | PCTMP + XIOS | — |
| 10 | AH=03 | Get cursor position/shape | Partial: position returned, CX=0 (shape missing) | PCTMP + XIOS | NEW |
| 10 | AH=04 | Light pen | No operation | PCTMP | — |
| 10 | AH=05 | Select display page | No operation; one page exposed | PCTMP | NEW |
| 10 | AH=06 / 07 | Scroll up / down | Implemented; bounded rectangle | PCTMP + XIOS | — |
| 10 | AH=08 | Read character/attribute | Implemented | PCTMP + XIOS | — |
| 10 | AH=09 | Write character/attribute | Implemented; cursor unchanged, page forced to zero | PCTMP + XIOS | — |
| 10 | AH=0A | Write character | Implemented; console default attribute, page forced to zero | PCTMP + XIOS | — |
| 10 | AH=0B | CGA palette | Implemented; foreground graphics console | PCTMP + XIOS | — |
| 10 | AH=0C / 0D | Write / read pixel | CGA modes 4/5/6, page 0, bounded coordinates, foreground only | PCTMP + XIOS | — |
| 10 | AH=0E | Teletype | Native console output path | PCTMP + XIOS | — |
| 10 | AH=0F | Get video mode | Implemented; BH=0 | PCTMP + XIOS | — |
| 10 | AH=10–FF | Later video functions | No operation | PCTMP | — |
| 11 | — | Equipment word | Cached equipment; display bits from console DOS-entry mode | PCTMP + XIOS | — |
| 12 | — | Memory size | Allocation end in KiB; not a general physical-memory entitlement | PCTMP | — |
| 13 | AH=02 / 03 / 04 | Raw read / write / verify | Unavailable: BDOS raw-disk body compiled out | PCTMP → BDOS | NEW |
| 13 | Other AH | Other disk services | AX=0, CF clear without service; not usable support | PCTMP | NEW |
| 14 | All | Serial BIOS | Not intercepted; no PC-MODE ownership/virtualization | Firmware | NEW |
| 15 | All | Cassette / later system services | Not intercepted; firmware-dependent | Firmware | — |
| 16 | AH=00 | Read key | Implemented; per-console queue | PCTMP + CIO/XIOS | — |
| 16 | AH=01 | Poll key | Implemented; per-console lookahead, ZF status | PCTMP + CIO/XIOS | — |
| 16 | AH=02 | Shift status | Implemented; per-console state | PCTMP + XIOS | — |
| 16 | AH=03–FF | Other / enhanced keyboard calls | Fall through to shift query; not their specified service | PCTMP | NEW |
| 17 | AH=00 | Print character | Native assigned list device; ownership and bounded ready poll; ignores BIOS printer selection | PCTMP + CIO/XIOS | — |
| 17 | AH=01 | Initialize printer | Native list detach; not BIOS hardware initialization | PCTMP + CIO | — |
| 17 | AH=02 | Printer status | Native list status mapping | PCTMP + XIOS | — |
| 17 | AH=03–FF | Other printer calls | No service after ownership check | PCTMP | — |
| 18 / 19 | — | ROM BASIC / bootstrap | Not intercepted; no application compatibility contract | Firmware | — |
| 1A | All | BIOS clock | Not intercepted; firmware-dependent, separate from DOS date/time adapters | Firmware | — |
| 1B / 1C | — | Break / timer callbacks | Not installed as PC-MODE virtual callbacks | Firmware / XIOS | — |
| 1D–1F | — | BIOS table vectors | No per-console virtualization | Firmware | — |
| 20 | — | Terminate | Implemented; CS=PSP legacy convention | PCTMP | — |
| 21 | AH | DOS service dispatch | See tables above | PCTMP | — |
| 22 | — | Termination continuation | Installed return/cleanup handler; callback, not ordinary service | PCTMP | — |
| 23 | — | Ctrl-C callback | Installed handler; checked-input unwind supported | PCTMP | — |
| 24 | — | Critical-error callback | Installed abort/retry path | PCTMP | — |
| 25 / 26 | — | Absolute disk read / write | Unavailable: BDOS raw-disk body compiled out; no valid disk result promised | PCTMP → BDOS | NEW |
| 27 | — | Stay resident | IRET only; does not retain a program | PCTMP | — |
| Other | — | Unlisted interrupts | No additional PCTMP-installed handler; existing firmware/kernel ownership applies | — | — |

## INT E0h — native CCP/M services

| Calling convention | Value |
| --- | --- |
| Entry | CL=function; DX=parameter, DS=parameter segment |
| Return | AX=BX=result; CX=error when AX=FFFFh; ES=segment result where applicable |
| Preserved | DS, SI, DI, BP |
| Availability | Same table with TMP and PCTMP; device services reach shared XIOS |
| Error 1 / 2 | Not implemented / illegal function number; AX=BX=FFFFh |

| CL hex | Decimal | Service | Module | Status / limit | Extension |
| --- | --- | --- | --- | --- | --- |
| 00 | 0 | system reset | RTM | Implemented | — |
| 01 | 1 | conin | CIO | Implemented | — |
| 02 | 2 | conout | CIO | Implemented | — |
| 03 | 3 | Raw console input (MP/M API) | SUP | Public call not implemented; internal CIO handler remains | NEW |
| 04 | 4 | Raw console output (MP/M API) | SUP | Public call not implemented; internal CIO handler remains | NEW |
| 05 | 5 | list out | CIO | Implemented | — |
| 06 | 6 | raw conio | CIO | Implemented | — |
| 07 | 7 | getiobyte | SUP | Not implemented; error 1 | NEW |
| 08 | 8 | setiobyte | SUP | Not implemented; error 1 | NEW |
| 09 | 9 | conwrite | CIO | Implemented | — |
| 0A | 10 | conread | CIO | Implemented | — |
| 0B | 11 | constat | CIO | Implemented | — |
| 0C | 12 | get version | SUP | Implemented | — |
| 0D | 13 | diskreset | BDOS | Implemented | — |
| 0E | 14 | diskselect | BDOS | Implemented | — |
| 0F | 15 | file open | BDOS | Implemented | — |
| 10 | 16 | file close | BDOS | Implemented | — |
| 11 | 17 | search first | BDOS | Implemented | — |
| 12 | 18 | search next | BDOS | Implemented | — |
| 13 | 19 | file delete | BDOS | Implemented | — |
| 14 | 20 | file read seq | BDOS | Implemented | — |
| 15 | 21 | file write seq | BDOS | Implemented | — |
| 16 | 22 | file make | BDOS | Implemented | — |
| 17 | 23 | file rename | BDOS | Implemented | — |
| 18 | 24 | login vector | BDOS | Implemented | — |
| 19 | 25 | get def disk | BDOS | Implemented | — |
| 1A | 26 | set dma | BDOS | Implemented | — |
| 1B | 27 | get alloc vector | BDOS | Implemented | — |
| 1C | 28 | write protect | BDOS | Implemented | — |
| 1D | 29 | get r/0 vector | BDOS | Implemented | — |
| 1E | 30 | set file attr. | BDOS | Implemented | — |
| 1F | 31 | get disk parm block | BDOS | Implemented | — |
| 20 | 32 | user code | BDOS | Implemented | — |
| 21 | 33 | file read random | BDOS | Implemented | — |
| 22 | 34 | file write random | BDOS | Implemented | — |
| 23 | 35 | file size | BDOS | Implemented | — |
| 24 | 36 | set random record | BDOS | Implemented | — |
| 25 | 37 | reset drive | BDOS | Implemented | — |
| 26 | 38 | access drive | BDOS | Implemented | — |
| 27 | 39 | free drive | BDOS | Implemented | — |
| 28 | 40 | file write random w/zero fill | BDOS | Implemented | — |
| 29 | 41 | Test and Write (NOT IMPLEMENTED) | SUP | Not implemented; error 1 | NEW |
| 2A | 42 | Lock Record | BDOS | Implemented | — |
| 2B | 43 | Unlock Record | BDOS | Implemented | — |
| 2C | 44 | Set Multi-sector | BDOS | Implemented | — |
| 2D | 45 | Set Bdos Error Mode | BDOS | Implemented | — |
| 2E | 46 | Get Disk Free Space | BDOS | Implemented | — |
| 2F | 47 | Chain to Program | SUP | Implemented | — |
| 30 | 48 | Flush Buffers | BDOS | Implemented | — |
| 31 | 49 | Reserved | SUP | Illegal; error 2 | — |
| 32 | 50 | call xios | SUP | Restricted BIOS gateway; see below | — |
| 33 | 51 | set dma base | BDOS | Implemented | — |
| 34 | 52 | get dma | BDOS | Implemented | — |
| 35 | 53 | get max mem | MEM | Implemented | — |
| 36 | 54 | get abs max mem | MEM | Implemented | — |
| 37 | 55 | alloc mem | MEM | Implemented | — |
| 38 | 56 | alloc abs mem | MEM | Implemented | — |
| 39 | 57 | free mem | MEM | Implemented | — |
| 3A | 58 | free all mem | MEM | Implemented | — |
| 3B | 59 | load | SUP | Implemented | — |
| 3C | 60 | Reserved | SUP | Illegal; error 2 | — |
| 3D | 61 | Reserved | SUP | Illegal; error 2 | — |
| 3E | 62 | Reserved | SUP | Illegal; error 2 | — |
| 3F | 63 | Reserved | SUP | Illegal; error 2 | — |
| 40 | 64 | network login | NET | Requires optional NET module; absent from standard images | — |
| 41 | 65 | network logoff | NET | Requires optional NET module; absent from standard images | — |
| 42 | 66 | network send msg | NET | Requires optional NET module; absent from standard images | — |
| 43 | 67 | network rcv msg | NET | Requires optional NET module; absent from standard images | — |
| 44 | 68 | network status | NET | Requires optional NET module; absent from standard images | — |
| 45 | 69 | get network config addr | NET | Requires optional NET module; absent from standard images | — |
| 46 | 70 | set Compatibility attributes | NET | Requires optional NET module; absent from standard images | — |
| 47 | 71 | get network server config table | NET | Requires optional NET module; absent from standard images | — |
| 48 | 72 | set network error mode | NET | Requires optional NET module; absent from standard images | — |
| 49 | 73 | attach network | NET | Requires optional NET module; absent from standard images | — |
| 4A | 74 | detach network | NET | Requires optional NET module; absent from standard images | — |
| 4B | 75 | set message buffer size | NET | Requires optional NET module; absent from standard images | — |
| 4C | 76 | get network time and date | NET | Requires optional NET module; absent from standard images | — |
| 4D | 77 | get network parameter table | NET | Requires optional NET module; absent from standard images | — |
| 4E | 78 | Reserved | SUP | Illegal; error 2 | — |
| 4F | 79 | Reserved | SUP | Illegal; error 2 | — |
| 50 | 80 | Reserved | SUP | Illegal; error 2 | — |
| 51–61 | 81–97 | Unassigned | SUP | Illegal; error 2 | — |
| 62 | 98 | Reset Alloc Vector | BDOS | Implemented | — |
| 63 | 99 | Truncate File | BDOS | Implemented | — |
| 64 | 100 | Set Dir Label | BDOS | Implemented | — |
| 65 | 101 | Return Dir Label | BDOS | Implemented | — |
| 66 | 102 | Read File XFCB | BDOS | Implemented | — |
| 67 | 103 | Write File XFCB | BDOS | Implemented | — |
| 68 | 104 | Set Date and Time | BDOS | Implemented | — |
| 69 | 105 | Get Date and Time | BDOS | Implemented | — |
| 6A | 106 | Set Default Password | BDOS | Implemented | — |
| 6B | 107 | Return Serial Number | SUP | Implemented | — |
| 6C | 108 | Get/set program return code (CP/M 3 API) | SUP | Not implemented in this CCP/M-86 kernel; AX=BX=FFFFh, CX=1 | NEW |
| 6D | 109 | Get/Set Console Mode | CIO | Implemented | — |
| 6E | 110 | Get/Set Output Delimiter | CIO | Implemented | — |
| 6F | 111 | Print Block | CIO | Implemented | — |
| 70 | 112 | List Block | CIO | Implemented | — |
| 71 | 113 | Reserved | SUP | Illegal; error 2 | — |
| 72 | 114 | Reserved | SUP | Illegal; error 2 | — |
| 73 | 115 | Reserved | SUP | Illegal; error 2 | — |
| 74 | 116 | Set File_Date_Time | BDOS | Implemented | — |
| 75–7F | 117–127 | Unassigned | SUP | Illegal; error 2 | — |
| 80 | 128 | mem req | MEM | Implemented | — |
| 81 | 129 | (same function as 128) | MEM | Alias of 128 | — |
| 82 | 130 | mem free | MEM | Implemented | — |
| 83 | 131 | poll device | RTM | Implemented | — |
| 84 | 132 | flag wait | RTM | Implemented | — |
| 85 | 133 | flag set | RTM | Implemented | — |
| 86 | 134 | queue make | RTM | Implemented | — |
| 87 | 135 | queue open | RTM | Implemented | — |
| 88 | 136 | queue delete | RTM | Implemented | — |
| 89 | 137 | queue read | RTM | Implemented | — |
| 8A | 138 | cond. queue read | RTM | Implemented | — |
| 8B | 139 | queue write | RTM | Implemented | — |
| 8C | 140 | cond. queue write | RTM | Implemented | — |
| 8D | 141 | delay | RTM | Implemented | — |
| 8E | 142 | dispatch | RTM | Implemented | — |
| 8F | 143 | terminate | RTM | Implemented | — |
| 90 | 144 | create process | RTM | Implemented | — |
| 91 | 145 | set priority | RTM | Implemented | — |
| 92 | 146 | console attach | CIO | Implemented | — |
| 93 | 147 | console detach | CIO | Implemented | — |
| 94 | 148 | set def console | CIO | Implemented | — |
| 95 | 149 | console assign | CIO | Implemented | — |
| 96 | 150 | CLI | SUP | Implemented | — |
| 97 | 151 | call RPL | SUP | Requires matching resident procedure library | — |
| 98 | 152 | parse filename | SUP | Implemented | — |
| 99 | 153 | get def console | CIO | Implemented | — |
| 9A | 154 | sysdat addr | SUP | Implemented | — |
| 9B | 155 | time of day | SUP | Implemented | — |
| 9C | 156 | get PD addr | RTM | Implemented | — |
| 9D | 157 | abort process | RTM | Implemented | — |
| 9E | 158 | attach list | CIO | Implemented | — |
| 9F | 159 | detach list | CIO | Implemented | — |
| A0 | 160 | set list dev | CIO | Implemented | — |
| A1 | 161 | Cond. Attach list | CIO | Implemented | — |
| A2 | 162 | Cond. Attach Console | CIO | Implemented | — |
| A3 | 163 | MP/M Version Number | SUP | Implemented | — |
| A4 | 164 | get list dev | CIO | Implemented | — |
| A5 | 165 | AUX attach (later CDOS API) | SUP | Unimplemented; illegal, error 2 | NEW |
| A6 | 166 | AUX detach (later CDOS API) | SUP | Unimplemented; illegal, error 2 | NEW |
| A7 | 167 | Conditional AUX attach (later CDOS API) | SUP | Unimplemented; illegal, error 2 | NEW |
| A8 / A9 | 168 / 169 | Set / get AUX device (later CDOS API) | SUP | Unimplemented; illegal, error 2 | NEW |
| AA / AB | 170 / 171 | AUX input / output status (later CDOS API) | SUP | Unimplemented; illegal, error 2 | NEW |
| AC / AD | 172 / 173 | AUX block input / output (later CDOS API) | SUP | Unimplemented; illegal, error 2 | NEW |
| AE–EF | 174–239 | Unassigned | SUP | Illegal; error 2 | — |
| F0 | 240 | Per-console command search path/order | SUP | Project-only SETDEF extension; matching kernel/callers required | NEW (implemented) |
| F1–FF | 241–255 | Unassigned | SUP | Illegal; error 2 | — |

## INT E0h / CL=32h (50 decimal) — direct BIOS gateway

Packet at DS:DX: byte selector, word CX argument, word DX argument.

| Packet selector (decimal) | Service | Status | Extension |
| --- | --- | --- | --- |
| 0 / 1 | Cold / warm boot | Terminates caller; does not reboot machine | — |
| 2 | Console status | Native console status | — |
| 3 / 4 | Console input / output | Raw CIO path | — |
| 5 | List output | Native assigned list device | — |
| 6 / 7 | AUX output / input | Unconditional XIOS RET stubs; not enabled by configuration | NEW |
| 15 | List status | XIOS list status | — |
| All others | Disk, video and other direct BIOS functions | Rejected; error 2 | — |

## INT E0h / CL=F0h — SETDEF packet (project extension)

| Origin | Availability |
| --- | --- |
| Defined by this project; not an original Digital Research API | Both current builds; packet versions 1–3 are project-defined |
| Number assignment | F0h/240 is project-local, unused in the reviewed MP/M/CDOS tables; no universal reservation claimed |
| Previous number | A5h/165 removed: reserved for later DR AUX attach. Old SETDEF/PCTMP binaries must be rebuilt; no alias |

| Offset | Size | Meaning |
| --- | --- | --- |
| 0 | Word | Signature 4453h (`SD`) |
| 2 | Byte | Version: 1, 2 or 3 |
| 3 | Byte | Operation: 0=query, 1=set path, 2=set order, 3=set both; v1 accepts 0/1 |
| 4 | Byte | Path length: 1–4 |
| 5–8 | 4 bytes | Drives 00h–0Fh; FFh=current drive |
| 9 | Byte | Returned system drive |
| 10 | Byte | v2: SUB bit 0; v3: SUB bit 0, disable DOS bit 1; absent in v1 |
| Return | AX/BX, CX | Success 0; invalid packet FFFFh with error; native TMP ignores DOS selection |

## INT E6h — native XIOS compatibility entry

| AL hex | Decimal | Service | Status / return | Extension |
| --- | --- | --- | --- | --- |
| 12 | 18 | Restore normal status line | Implemented | — |
| 15 | 21 | Display supplied status line | CX:SI points to status data | — |
| 1D | 29 | Hard-disk maintenance information | Open-file flag and ROM entry | — |
| 1E | 30 | Enhanced graphics capability | Local extension: AX='EG', BX capability bits; input BX=0 for old kernels | NEW (implemented) |
| 1F | 31 | Function-key table | CX=console; returns table pointers | — |
| 20 | 32 | Maximum floppy drive | AL=result | — |
| Other | — | Unassigned selectors | No operation | — |

## Extension candidates — source and dependencies

| Candidate calls | Source / scope | Dependencies | Validation / decision |
| --- | --- | --- | --- |
| E0h: 03/04/07/08/A5–AB | CDOS 4.1 `D1/aux.cio`, `D1/console.cio`, `D5/xaux.lib`; native 3/4/7/8 and 165–171 | CIO ownership/device state and cleanup; adapt existing SERIN/SEROUT queues; trace ASSIGN ABI; TMP unchanged | Deferred: no agreed serial test setup; no backport started |
| 21h: 03/04 | New PCTMP adapters at DOS 03h/04h | AUX native services, register/return handling | Deferred with AUX; DOS calls need their own validation |
| E0h: AC/AD | CDOS 4.1 `D1/aux.cio`; native 172/173 | AUX plus block buffers/counts and partial transfers | Later stage; serial test setup required |
| INT 14h | New PCTMP INT 14h adapter; existing serial driver reference | AUX ownership, UART settings/status mapping; coordinate firmware interception | Separate from DOS AUX; serial test setup required |
| E0h: 29 (41 decimal) | MP/M-86 `D1/bdosif.bdo`: BF_TANDW, TST_WRT, COMPARE_RECS | Adapt current BDOS shell, record buffers, multi-sector handling and disk mutex | Candidate; not DOS write verification |
| E0h: 6C (108 decimal) | CP/M 3 function 108 specification; neither checked MP/M nor CDOS source implements it | Per-process pending result, per-console last-job result, launch/abort/CHAIN semantics, TMP/PCTMP integration | Keep unsupported until semantics agreed |
| 10h: 03 | Existing PCTMP INT 10h/03h and XIOS cursor state | Return shape through saved CX; XIOS query interface | Localized candidate; emulator check feasible |
| INT 16h enhanced calls | Existing PCTMP INT 16h and XIOS keyboard state | Explicit enhanced selectors, scan-code/shift semantics | Candidate; selected calls only |
| 10h: 05 | Existing PCTMP/XIOS video paths | Per-console page storage, addressing and restoration | Later candidate |
| 21h: 2E | DOS 1.10/1.25 AH=2Eh specification | PCTMP flag; BDOS/XIOS actual media verification, not cached reread | Deferred; verification/error-injection method required |
| 21h: 1B/1C/1F | Existing `~/Projects/DOS/pcdos11-hacking/dos.asm`: GETFATPT (1Bh), GETFATPTDL (1Ch), GETDSKPT (1Fh) | GETTHISDRV (1Bh/1Ch), FATREAD, CURDRV/THISDRV, DOS DPB/FAT structures and saved-register frame; adapt to CP/M storage and PCTMP ABI | Source available; adaptation scope open. 1Fh returns a DPB pointer but its existing handler also calls FATREAD |
| DOS 2 file/handle subset | DOS 2 API specification; implementation not selected | PCTMP handles, byte offsets, errors, cleanup and CP/M file mapping | Future project; table IDs do not imply a complete DOS 2 implementation |
| 21h: 48–4B, 4D | DOS 2 API specification; implementation not selected | DOS2 handles, memory ownership, parent/child PSPs and exit status | After bounded DOS2 subset |
| INT 13h/25h/26h | Existing disabled `file4.bdo` raw-disk body | Filesystem/media contract, cache coherence, drive ownership and error semantics | Separate project; not enabled by this inventory |

| Historical source label | Location relative to repository root |
| --- | --- |
| CDOS 4.1 | `../ossrc/cdos-41-oem/` |
| MP/M-86 | `../ossrc/mpm862sr/` |

## Sources

| Table | Source |
| --- | --- |
| Current DOS/BIOS dispatch | [pctmp.a86](kernel/rsp3/pctmp.a86), [pcguard.lib](kernel/rsp3/pcguard.lib) |
| Current device behaviour | [xpcmode.lib](kernel/con/xpcmode.lib), [xbegin.lib](kernel/con/xbegin.lib), [xchar.lib](kernel/con/xchar.lib) |
| Native function assignments | [sysdat.dat](kernel/con/sysdat.dat), [supif.sup](kernel/con/supif.sup), [sysfunc.sup](kernel/con/sysfunc.sup), [command.sup](kernel/con/command.sup) |
| Disabled raw disk / FAT | [bdos.a86](kernel/con/bdos.a86), [file4.bdo](kernel/con/file4.bdo) |
| IBM DOS 1.10 assignments | [IBM DOS 1.10 Reference, Appendix D](https://www.minuszerodegrees.net/manuals/IBM/IBM%20DOS%20Version%201.10%20-%20Reference%20-%20Second%20Edition%20MAY82.pdf) |
| MS-DOS 1.25 assignments | [Microsoft MSDOS.ASM, DISPATCH table](https://github.com/microsoft/MS-DOS/blob/main/v1.25/source/MSDOS.ASM) |
| CP/M 3 function 108 definition | [Digital Research CP/M Plus Programmer’s Guide, §3, function 108](https://www.bitsavers.org/pdf/digitalResearch/cpm_plus/CPM_Plus_Programmers_Guide_Jan83.pdf) |

| Local IBM/MS DOS comparison | `~/Projects/DOS/pcdos11-hacking/dos.asm`: DISPATCH → GETFATPT (1Bh), GETFATPTDL (1Ch), GETDSKPT (1Fh), active in both variants; IBM 1.10 manual wording does not establish binary absence |
