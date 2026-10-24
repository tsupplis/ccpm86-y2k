# PUT / GET / RDR: Console I/O Redirection for Concurrent CP/M-86

Comprehensive architecture, syntax specification, and implementation plan for Digital Research bracket-style console input, output, and dual redirection utilities (`PUT`, `GET`, and `RDR`) in Concurrent CP/M-86.

---

## 1. Syntax Specification

In Concurrent CP/M-86, square brackets `[...]` are Digital Research's standard syntax for options and modifiers (`CHSET`, `SDIR`, `SETDEF`, `VCMODE`).

The bracket prefix `[...]` cleanly separates the redirection parameters from the child command line.

### Flag Conventions:
* **`A`** : **Append** to output file instead of overwriting.
* **`E`** : **Echo** to screen (applies to both streams in `RDR`, or standard echo in `PUT`/`GET`).
* **`EO`**: **Echo Output** to screen only (in `PUT` and `RDR`).
* **`EI`**: **Echo Input** to screen only (in `GET` and `RDR`).

---

### A. `PUT.CMD` (Output Redirection / `>` and `>>`)

Captures all console output (`BDOS 2, 6, 9` and DOS `INT 21h` stdout) from a command and its subchildren into a disk file.

* **Syntax:**
  ```text
  PUT [outfile{,flags}] command_line
  ```
* **Examples:**
  ```text
  A>PUT [BUILD.LOG] ASM86 MYPROG $SZ
  A>PUT [BUILD.LOG,A] LINK86 MYPROG
  A>PUT [SDIR.TXT,E] SDIR [FULL]
  A>PUT [BUILD.LOG,A,EO] LIB86 MYLIB
  A>PUT [DOSLOG.TXT] MSMASM HELLO;
  ```

---

### B. `GET.CMD` (Input Redirection / `<`)

Feeds sequential input from a text file into console input (`BDOS 1, 6, 10` and DOS `INT 21h` stdin) for interactive utilities until EOF (`Ctrl-Z`).

* **Syntax:**
  ```text
  GET [infile{,flags}] command_line
  ```
* **Examples:**
  ```text
  A>GET [SCRIPT.IN] ED MYFILE.TXT
  A>GET [CMDS.DAT] PIP
  A>GET [BATCH.IN,E] DDT86 PROG.CMD
  A>GET [SCRIPT.IN,EI] PIP
  ```

---

### C. `RDR.CMD` (Unified Dual Redirection / `<` and `>`)

Simultaneously redirects console input and output for a single child process.

* **Syntax:**
  ```text
  RDR [infile, outfile{,flags}] command_line
  ```
* **Examples:**
  ```text
  A>RDR [SCRIPT.IN, BUILD.LOG] ED MYFILE.TXT
  A>RDR [COMMANDS.DAT, PIP.LOG, EO] PIP
  A>RDR [SCRIPT.IN, ED.LOG, EI] ED MYFILE.TXT
  A>RDR [NEXTCMD.IN, MASTER.LOG, A, EO] PIP
  A>RDR [TEST.IN, TEST.LOG, A, E] TESTSUITE
  ```

---

## 2. Technical Architecture & Concurrency Rules

### A. Concurrency Isolation (Virtual Console & Subchildren)
Overwriting `INT E0h` globally in real mode would affect background tasks and other virtual consoles. `PUT`, `GET`, and `RDR` ensure safety through two fields in `SYSDAT` and process descriptors:

1. **Virtual Console Check:**
   Every process descriptor (`PD`) has a console index `p_cns` (`PD + 24h` / `SYSDAT:[0068h]->p_cns`).
2. **Descendant Tracking:**
   Every child process records its creator in `p_parent` (`PD + 1Ch`).
3. **Filter Rule:**
   The `INT E0h` interceptor checks:
   $$\text{Active Console} == \text{Our Console} \quad \text{AND} \quad (\text{Active\_PD} == \text{Child\_PD} \ \lor \ \text{p\_parent} \in \text{Child Tree})$$
   * **Result:** All output from subchildren (like `SUBMIT` batch files or compiler drivers) is captured.
   * **Safety:** Other virtual consoles and background daemons (spooler, clock) are completely untouched.

---

### B. Process Lifecycle: `p_cli`, `p_attach`, `p_detach`, and Priority

The parent/child execution flow follows the standard `cpm86-make` / `execproc.c` lifecycle:

```mermaid
sequenceDiagram
    autonumber
    actor User
    participant RDR as RDR.CMD (Parent)
    participant Kernel as BDOS (SUP/RTM)
    participant Child as Child / Subchild Process
    participant DiskIn as Input File
    participant DiskOut as Output File

    User->>RDR: RDR [IN.DAT, OUT.LOG, A, EO] SUBMIT BUILD
    RDR->>RDR: Parse bracket prefix [IN.DAT, OUT.LOG, A, EO]
    RDR->>DiskIn: Open input file (BDOS 15)
    RDR->>DiskOut: Open/Seek (Append) or Create output file (BDOS 15/22)
    RDR->>RDR: Install INT E0h interceptor (with console/PD filter)
    RDR->>Kernel: Call BDOS 150 (p_cli) with child command line
    Note over Kernel,RDR: p_cli transfers console ownership to child; parent loses console
    RDR->>Kernel: Call BDOS 140 (p_attach) to wait for child
    Note over RDR,Kernel: Parent blocks (sleeps) until child & subchildren release console

    loop While Child / Subchildren Run
        Child->>Kernel: INT E0h (BDOS 1/6/10: Console In or 2/6/9: Console Out)
        Kernel->>RDR: Filter matches active console & child PD
        alt Console Input Request
            RDR->>DiskIn: Read sector & supply character (BDOS 20)
            opt If EI or E set
                RDR->>Kernel: Echo input char to screen (XIOS)
            end
        else Console Output Request
            RDR->>DiskOut: Buffer & write sector (BDOS 21)
            opt If EO or E set
                RDR->>Kernel: Echo output char to screen (XIOS)
            end
        end
    end

    Child->>Kernel: Child exits & releases console (BDOS 143)
    Kernel-->>RDR: p_attach unblocks; control returns to RDR.CMD
    RDR->>DiskOut: Flush final sector (Ctrl-Z padded) & Close file (BDOS 16)
    RDR->>DiskIn: Close input file (BDOS 16)
    RDR->>RDR: Restore original INT E0h vector
    RDR->>User: Return to prompt
```

1. **`p_cli` (BDOS 150):** Submits the child command line. The kernel supervisor parses the `.CMD`, allocates child memory, sets child priority (`p_pri = 200`), transfers console ownership to the child, and starts the child.
2. **`p_attach` (BDOS 140) Blocking Wait:** The parent calls `BDOS 140` (`p_attach`). Because the child owns the console, the parent is put to sleep until the child (and all subchildren) terminate and release the console.
3. **Priority Management:** The parent retains its normal priority and yields, letting the child execute at standard foreground priority.

---

### C. Append Mode Mechanics (`A`)

1. **File Probe:** Open file via `BDOS 15` (Open File). If absent, fall back to `BDOS 22` (Make File).
2. **File Size Computation:** Issue `BDOS 35` (Compute File Size) to determine total existing records ($R$).
3. **Partial Sector Scan:**
   * Read the final sector via `BDOS 33` (Random Read) at record $R - 1$.
   * Scan for the CP/M EOF byte `1Ah` (<kbd>Ctrl-Z</kbd>).
   * Position the write cursor at the first `1Ah` (overwriting the EOF terminator).
4. **Append Stream:** Continue sequential sector writes from that offset onward.

---

## 3. Project Structure under `commands/putget/`

```
commands/putget/
├── Makefile       # Standard build rules (cpm86_asm86 + cpm86_gencmd)
├── put.a86        # Output redirection command (PUT [file,flags] cmd)
├── get.a86        # Input redirection command (GET [file,flags] cmd)
├── rdr.a86      # Unified input/output redirection (RDR [in,out,flags] cmd)
└── README.md      # Architecture, specification, and design documentation
```

---

## 4. Implementation Plan & Milestones

### Phase 1: Directory Scaffolding & Build Rules
- [ ] Create `commands/putget/Makefile`.
- [ ] Add `putget` to root `commands/Makefile`.

### Phase 2: `PUT.CMD` (Output Redirection)
- [ ] **Parser:** Parse `[filename,flags]`. Detect flags `A`, `E`, `EO`.
- [ ] **File Engine:** Overwrite (`BDOS 19/22`) vs Append (`BDOS 15/35/33` seek to `Ctrl-Z`).
- [ ] **Sector Accumulator:** 128-byte buffer + `BDOS 21` (Sequential Write).
- [ ] **Interceptor & Process Tree Filter:**
  - Hook `INT E0h` (`0000:0380h`).
  - Trap `CL = 2` (Con Out), `CL = 6` (Direct I/O out), `CL = 9` (Print String).
  - Handle screen echo if `E` or `EO` set.
- [ ] **Execution & Synchronization:**
  - Launch child via `BDOS 150` (`p_cli`).
  - Block-wait via `BDOS 140` (`p_attach`).
  - Flush partial sector padded with `1Ah`, close FCB (`BDOS 16`), restore `INT E0h`.

### Phase 3: `GET.CMD` (Input Redirection)
- [ ] **Parser:** Parse `[filename,flags]`. Detect flags `E`, `EI`.
- [ ] **Sector Reader:** Read 128-byte sectors via `BDOS 20` (Sequential Read).
- [ ] **Interceptor:**
  - Trap `CL = 1` (Con In), `CL = 6` (Direct I/O in), `CL = 10` (Read Buffered Line).
  - Feed characters from buffer until `1Ah` (EOF), then revert to keyboard.
  - Handle screen echo if `E` or `EI` set.
- [ ] **Execution & Cleanup:** Launch child via `BDOS 150`, wait via `BDOS 140`, close file, restore vectors.

### Phase 4: `RDR.CMD` (Unified Dual Redirection)
- [ ] Dual-parser: parse `[infile, outfile, flags]`.
- [ ] Combine input reader and output accumulator.
- [ ] Support `A`, `EI`, `EO`, and `E` flag matrix.

### Phase 5: Help & Media Integration
- [ ] Update [`commands/help/help.dat`](commands/help/help.dat) with `///1PUT`, `///1GET`, and `///1RDR` entries.
- [ ] Package tools into [`media/Makefile`](media/Makefile).
- [ ] Full regression testing under QEMU / PCE.
