# `bfdbg` — terminal Brainfuck debugger

`bfdbg.py` is a self-contained Python terminal application for running and debugging Brainfuck programs. It uses the standard-library `curses` module, so on Linux/macOS it does not need extra packages.

```bash
python3 bfdbg.py program.bf
```

On Windows, install the `windows-curses` package first or run it inside WSL.

## Layout

The interface has four panes:

1. **Program** — a horizontally scrolling instruction tape. The centered instruction is highlighted, and the current instruction pointer is marked with `IP`.
2. **Memory tape** — a horizontally scrolling sparse tape with decimal, hex, and printable-character views.
3. **Output** — a virtual output screen with scrollback and ASCII/hex/decimal modes.
4. **Input terminal** — a line-buffered input pane. Lines submitted here are queued for Brainfuck `,` instructions.

## Main keys

### Execution

| Key | Action |
|---|---|
| `Right`, `Space`, `n` | Step forward one Brainfuck instruction |
| `Left`, `Backspace` | Reverse one executed instruction or cell edit using the undo log |
| `r` | Animated debug-run; redraws while running and logs compact per-instruction undo records |
| `R` | Cooperative fast-run until halt, input, or limit; redraws periodically and still logs compact per-instruction undo records |
| `x` | Reset program state: `ip=0`, `dp=0`, memory/output/input queue cleared, undo history cleared |
| `q` | Quit |
| `R` or `Esc` while running | Pause a cooperative run |
| `?` | Show help overlay |

Fast run is much quicker because it avoids redrawing after every instruction. It now yields to curses periodically, so output is flushed during long runs and keys such as `R`/`Esc`, `q`, `x`, `v`, and output scrolling remain responsive. It still records a compact inverse operation for every executed instruction, so pressing `Left` after a fast run rewinds one Brainfuck instruction at a time rather than jumping over the whole run.

### Run-to-bracket and run-to-I/O

| Key | Action |
|---|---|
| `o` | Debug-run until the next opening bracket `[` is about to execute |
| `O` | Cooperative fast-run until the next opening bracket `[` is about to execute |
| `c` | Debug-run until the next closing bracket `]` is about to execute |
| `C` | Cooperative fast-run until the next closing bracket `]` is about to execute |
| `b` | Debug-run to the current loop boundary when the instruction pointer is on `[` or `]` |
| `B` | Cooperative fast-run to the current loop boundary when the instruction pointer is on `[` or `]` |
| `.` | Debug-run until the next output instruction `.` is about to execute |
| `>` | Cooperative fast-run until the next output instruction `.` is about to execute |
| `,` | Debug-run until the next input instruction `,` is about to execute |
| `<` | Cooperative fast-run until the next input instruction `,` is about to execute |

Both debug and fast run-to commands keep compact per-instruction undo records. Debug commands redraw after each instruction and use the smaller `--run-limit`. Fast commands use the larger `--fast-run-limit` and redraw only at chunk boundaries, which keeps them responsive without destroying the speed-up.

### Program view

These are source-navigation controls; they move the program view, not the execution state.

| Key | Action |
|---|---|
| `[` | Jump view to the next opening bracket `[` |
| `]` | Jump view to the next closing bracket `]` |
| `m` | Jump view to the matching bracket, when centered on `[` or `]` |
| `{` / `}` | Scroll program view left/right by one instruction |
| `(` / `)` | Scroll program view left/right by ten instructions |
| `p` | Recenter and follow the current instruction pointer |

### Memory tape

| Key | Action |
|---|---|
| `h` / `l` | Scroll memory left/right by one cell |
| `H` / `L` | Scroll memory left/right by ten cells |
| `g` | Go to a memory address; accepts decimal or `0x...` |
| `d` | Jump back to the data pointer and follow it |
| `f` | Toggle automatic data-pointer follow |
| `e` | Edit the centered memory cell |

### Output and input

| Key | Action |
|---|---|
| `v` | Cycle output mode: ASCII, hex, decimal |
| `PgUp`, `u` | Scroll output history up |
| `PgDn`, `j` | Scroll output history down |
| `End` | Follow the bottom of the output |
| `i` | Enter input mode |
| `Enter` in input mode | Queue the current line plus a newline |
| `Esc` in input mode | Cancel input mode |

## Command-line options

```bash
python3 bfdbg.py program.bf --delay 0.01 --run-limit 100000 --fast-run-limit 10000000
```

| Option | Meaning |
|---|---|
| `--cell-bits N` | Cell width in bits. Default: `8`. Use `0` for unbounded integer cells. |
| `--delay SECONDS` | Delay between instructions during animated debug-run. Default: `0.02`. |
| `--run-limit N` | Safety cap for one debug run-to-bracket command. Default: `100000`. |
| `--fast-run-limit N` | Safety cap for one fast run command. Default: `10000000`. |

## Notes

- The Brainfuck source is filtered to the eight command characters `><+-.,[]`; comments and whitespace are ignored.
- Cells wrap at 8 bits by default. Use `--cell-bits 0` for unbounded integer cells, or another positive bit width for a different wrapping size.
- The tape is sparse and supports negative addresses, which is convenient while debugging programs that move left of zero.
- Reverse stepping uses compact per-instruction undo records rather than full snapshots. Each record stores the previous `ip`, `dp`, status flags, tick count, and only the tiny piece of state changed by the command: at most one cell value, one emitted output byte, or the input line source used by a `,`.
- Fast mode no longer creates undo gaps. It is fast because it avoids per-instruction screen redraws, not because it skips history. During long fast runs, the UI redraws periodically and polls keys, so output keeps appearing and the run can be paused or reset.
- Reset with `x` clears the machine state and undo history. It does not reload the file from disk; it restarts the already-loaded filtered program.
- Rewinding over an input command `,` treats terminal input as interactive: it restores the machine state before the command, then discards the queued line that supplied the consumed byte. This prevents a rewound `,` from silently receiving the same input again.
