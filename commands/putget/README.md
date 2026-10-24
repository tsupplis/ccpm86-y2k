# PUT / GET / RDR: Console I/O Redirection for Concurrent CP/M-86

Bracket-style console redirection for native programs under Concurrent CP/M-86 3.1.

| Command | Effect |
| --- | --- |
| `PUT [out{,A}{,E\|EO}] cmd` | Capture the program's console output in `out` |
| `GET [in{,E\|EI}] cmd` | Feed the program's console input from `in` |
| `RDR [in,out{,A}{,E\|EI\|EO}] cmd` | Both at once |

Flags: `A` appends to the output file, `E` echoes both streams to the screen, `EO` echoes output only, `EI` echoes input only.

```text
A>PUT [BUILD.LOG,A,EO] ASM86 MYPROG
A>GET [SCRIPT.IN] PIP
A>RDR [KEYS.IN,PIP.LOG] PIP
```

## Scope and limits

* **Works for native `.CMD` programs.** Verified in QEMU: overwrite, append, `E`/`EO`, multi-record capture, GET feeding PIP, RDR with an input file and an output log.
* **DOS programs (`.COM`, `.EXE`) cannot be redirected.** They are launched by PC-MODE (`PCTMP`), not by `p_cli`, and `PCTMP` serves their `INT 21h` console calls itself. It reaches the kernel through a private copy of the `INT E0h` vector saved at its initialisation (`shared_bdos_vector`), so the hook installed here never sees them. `p_cli` also rejects explicit `.COM`/`.EXE` (`e_badftype`). Tested: `PUT [x] prog.exe` and `prog.com` fail with "CLI command execution failed".
* **Shell built-ins are not redirected** (`DIR`, `TYPE`, `CLS` and the PCTMP built-ins `USER`, `PRINTER`, `DOSMEM`, `DOS8087`, `DOSUSPND`, `REM`, `SYS`, `PAUSE`). Use `SDIR.CMD`, `TYPE.CMD`, etc.
* **RSPs** (such as `DIR`) are not captured. Checked in QEMU: `PUT [D.TXT] DIR B:*.CMD` showed the listing on the screen and left `D.TXT` empty. The cause has not been investigated.
* **Only the caller's virtual console is affected.** The hook filters on the console number of the running process; it does not track descendants by `p_parent`. Any process on that console is captured while the tool runs.

## How it works

1. `init_environment` checks BDOS 12 (BH = 11h or 14h, as SUBMIT does), saves the SYSDAT segment, raises priority one level above the parent and records the console number (BDOS 153).
2. The bracket prefix is parsed; file names go through BDOS 152 into FCBs. Input is opened (BDOS 15). Output is deleted and created, or opened and positioned at the first `1Ah` for `A` (BDOS 35 and 33).
3. The `INT E0h` vector (`0000:0380h`) is replaced by `int_e0_hook`. The hook runs on its own stack and saves and restores the child's DMA address and segment (BDOS 52 and 51).
4. The child is started with `p_cli` (BDOS 150), and the tool waits with BDOS 146 until the console is released.
5. The hook is removed, the last partial record is padded with `1Ah` and written, and the files are closed.

Trapped calls, only when the running process's console is ours:

| BDOS call | Redirected side |
| --- | --- |
| 2 Console Output, 6 Direct I/O (output), 9 Print String | output |
| 1 Console Input, 6 Direct I/O (`DL`=FFh), 10 Read Buffer, 11 Console Status | input |

Details:

* Output is written with random writes (BDOS 34) and a record counter. Screen echo calls BDOS 2 with `in_interceptor` set, so the hook does not capture its own echo.
* Input is read in 128-byte records. At `1Ah` or end of file, `trap_readbuf` returns an empty line, which ends programs that quit on an empty line (PIP does). Any later input call goes to the keyboard. An input file should therefore contain everything the program needs, including its exit. Checked in QEMU: `GET [K.IN] PIP` with one copy line in `K.IN` ran the copy, then PIP exited at the end of the file.
* `p_cns` is at PD offset `20h` (see `kernel/con/pd.def`).

## Files

```
commands/putget/
├── Makefile       # builds put.cmd get.cmd rdr.cmd
├── put.a86        # PUT front end
├── get.a86        # GET front end
├── rdr.a86        # RDR front end
├── putget.lib     # shared engine: environment, parsing, files, hook, traps
├── putget.dat     # shared data buffers (one copy, included at org 100h)
├── putget.def     # constants; `debug equ 0` gates the BDOS call logger
└── README.md
```

The tools are shipped on `ccpmext.img` with DREDIX and PIP.

## Possible future work

Redirecting DOS programs would need a change in PC-MODE: a redirect record consulted by the console paths of `pctmp.a86` (`dos_raw_output`, `raw_console_input`, `dos_read_line`, `dos_poll_character`). Hooking `INT 21h` from the tools is a possible alternative, but PCTMP swaps the `INT 10h`..`INT 27h` vector region per console (`swap_child_vectors`), so it is untested whether such a hook would survive.
