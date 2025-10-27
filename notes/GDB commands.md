## GDB Quick Reference Guide

### General Info & Help

| Command            | Description                                         |
|--------------------|-----------------------------------------------------|
| `info gdb`         | GDB manual overview                                 |
| `help`             | Show help topics                                    |
| `set print pretty on` | Enable pretty printing (add to `~/.gdbinit`)     |

### Program Control

| Command          | Description                                                                 |
|------------------|-----------------------------------------------------------------------------|
| `run`            | Start the program                                                           |
| `start`          | Equivalent to `break main` + `run` (sets temporary breakpoint at `main`)    |
| `continue` / `c` | Resume execution after breakpoint                                           |
| `next` / `n`     | Execute next line (step over function calls)                                |
| `step` / `s`     | Execute next line (step into function calls)                                |
| `finish` / `fin` | Run until current function returns                                          |
| `quit` / `q`     | Quit GDB                                                                    |

### Breakpoints

| Command                         | Description                                                 |
|---------------------------------|-------------------------------------------------------------|
| `break funcName` / `b funcName` | Set breakpoint at function                                  |
| `break file:line`               | Set breakpoint at specific line in file                     |
| `break _exit`                   | Breakpoint at program exit                                  |
| `info breakpoints` / `info b`   | List all breakpoints                                        |
| `delete`                        | Delete all breakpoints                                      |
| `delete <breakpoint_no>`        | Delete specific breakpoint                                  |
| `command <breakpoint_no>`       | Define commands to run when breakpoint is hit (end with `end`) |

### Variables & Data Inspection

| Command           | Description                                  |
|-------------------|----------------------------------------------|
| `info locals`     | Show local variables in current frame         |
| `info variables`  | Show global/static variables                  |
| `p var`           | Print value of variable                       |
| `p var = value`   | Modify variable value                         |
| `whatis expr`     | Show datatype of expression                   |
| `ptype expr`      | Detailed print of datatype/structure          |

### Stack & Function Info

| Command            | Description                           |
|--------------------|---------------------------------------|
| `info functions`   | List all defined functions            |
| `bt`               | Print full backtrace (call stack)     |
| `frame` / `f <n>`  | Switch to frame `<n>`                 |
| `up` / `down`      | Move up or down call stack            |

### Watchpoints (Data Breakpoints)

| Command                 | Description                              |
|-------------------------|------------------------------------------|
| `watch var`             | Stop when variable is modified           |
| `rwatch var`            | Stop when variable is read               |
| `awatch var`            | Stop when variable is read or written    |
| `watch -l foo`          | Watch memory location of `foo`           |
| `watch foo if foo > 10` | Conditional watchpoint                   |

### Layout & Interface

| Command        | Description                                |
|----------------|--------------------------------------------|
| `layout next`  | Cycle through GDB TUI layouts              |
| `refresh`      | Repaint interface (fix display corruption) |
| `Ctrl+L`       | Shortcut to refresh TUI                    |
| `Ctrl+X` then `A` | Toggle text/GUI mode in GDB             |

### Shell & Python Commands

| Command                        | Description                                  |
|--------------------------------|----------------------------------------------|
| `shell <cmd>`                  | Run a shell command from GDB                 |
| `python gdb.execute("cmd")`    | Execute GDB command from Python mode         |
| `file <executable>`            | Load or switch the executable being debugged |

### Core Dumps & Crash Analysis

| Command            | Description                               |
|--------------------|-------------------------------------------|
| `gdb -c core.num`  | Examine dumped core file                   |
| `bt`               | Backtrace (to find where crash occurred)   |
| `info registers`   | Inspect CPU registers at crash             |
| `info threads`     | Show active threads at crash               |

### Useful Tips

Add this to your `~/.gdbinit`:

```text
set print pretty on
set pagination off
```

Run GDB with debugging symbols:

```bash
gcc -g program.c -o program
gdb ./program
```

Combine with TUI mode:

```bash
gdb -tui ./program
```

### Example Debug Flow

```bash
gcc -g stream.c -o stream
gdb ./stream
```

```text
(gdb) break main
(gdb) run
(gdb) next
(gdb) print count
(gdb) watch buffer[in]
(gdb) continue
(gdb) bt
(gdb) quit
```