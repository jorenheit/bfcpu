#!/usr/bin/env python3
"""
bfdbg.py - a terminal Brainfuck interpreter with a built-in debugger.

Features
--------
- Four-pane curses UI: program, memory tape, output, and line-buffered input.
- Forward stepping and exact reverse stepping via compact per-instruction undo records.
- Animated debug-running plus cooperative fast-run commands that redraw periodically.
- Run-to-bracket and run-to-I/O execution commands plus non-executing program/source navigation.
- One-key reset back to the initial program state.
- Interactive memory view with follow-data-pointer mode, scrolling, goto, and cell editing.
- Output history with ASCII, hex, and decimal display modes.
- Line-buffered input terminal for Brainfuck ',' commands.

Run:
    python3 bfdbg.py program.bf

Keys are shown in the in-app help pane footer. Press '?' inside the app for a
larger help overlay.
"""

from __future__ import annotations

import argparse
import curses
import locale
import os
import sys
import time
from collections import deque
from dataclasses import dataclass, field
from typing import Deque, Dict, Iterable, List, Optional, Set

BF_CHARS = set("><+-.,[]")
OUTPUT_MODES = ("ascii", "hex", "dec")



@dataclass(frozen=True)
class InputByte:
    """One queued input byte plus the line/batch it came from.

    The source id lets reverse execution treat terminal input as an
    interactive side effect: when rewinding past a `,`, the rest of the line
    that supplied that byte is discarded instead of silently replayed.
    """

    value: int
    source_id: int


@dataclass
class UndoRecord:
    """The compact inverse of one executed debugger operation.

    A Brainfuck instruction changes only a tiny part of the machine state: the
    instruction pointer, sometimes the data pointer, at most one memory cell,
    maybe one output byte, and maybe one consumed input byte. Keeping just that
    delta is enough for exact reverse execution and is much cheaper than copying
    the whole sparse tape before every step.
    """

    ip: int
    dp: int
    tick: int
    halted: bool
    waiting_for_input: bool
    reason: str = "step"
    command: str = ""
    cell_addr: Optional[int] = None
    cell_was_present: bool = False
    cell_value: int = 0
    output_len: Optional[int] = None
    consumed_input: Optional[InputByte] = None


@dataclass
class BFState:
    ip: int = 0
    dp: int = 0
    memory: Dict[int, int] = field(default_factory=dict)
    output: bytearray = field(default_factory=bytearray)
    input_queue: Deque[InputByte] = field(default_factory=deque)
    next_input_source_id: int = 1
    tick: int = 0
    halted: bool = False
    waiting_for_input: bool = False


class BrainfuckMachine:
    def __init__(self, program: str, *, cell_modulus: Optional[int] = 256) -> None:
        self.program = "".join(ch for ch in program if ch in BF_CHARS)
        self.brackets = self._build_bracket_map(self.program)
        self.state = BFState()
        self.history: List[UndoRecord] = []
        self.cell_modulus = cell_modulus
        self.error: Optional[str] = None
        self._set_halted_if_needed()

    @staticmethod
    def _build_bracket_map(program: str) -> Dict[int, int]:
        stack: List[int] = []
        pairs: Dict[int, int] = {}
        for idx, cmd in enumerate(program):
            if cmd == "[":
                stack.append(idx)
            elif cmd == "]":
                if not stack:
                    raise ValueError(f"Unmatched closing bracket at instruction {idx}")
                other = stack.pop()
                pairs[idx] = other
                pairs[other] = idx
        if stack:
            raise ValueError(f"Unmatched opening bracket at instruction {stack[-1]}")
        return pairs

    def _cell(self, addr: int) -> int:
        return self.state.memory.get(addr, 0)

    def _set_cell(self, addr: int, value: int) -> None:
        if self.cell_modulus is not None:
            value %= self.cell_modulus
        if value == 0:
            self.state.memory.pop(addr, None)
        else:
            self.state.memory[addr] = value

    def _set_halted_if_needed(self) -> None:
        self.state.halted = self.state.ip < 0 or self.state.ip >= len(self.program)

    def enqueue_input_line(self, line: str) -> None:
        # Brainfuck input is byte-oriented. UTF-8 keeps normal ASCII programs simple
        # while still allowing non-ASCII terminal input. Each submitted line gets a
        # source id so reverse execution can discard that line when rewinding past
        # the comma that consumed from it.
        source_id = self.state.next_input_source_id
        self.state.next_input_source_id += 1
        for byte in (line + "\n").encode("utf-8"):
            self.state.input_queue.append(InputByte(byte, source_id))
        if self.state.waiting_for_input:
            self.state.waiting_for_input = False
            self._set_halted_if_needed()

    def _make_undo_record(self, command: str, reason: str = "step") -> UndoRecord:
        s = self.state
        record = UndoRecord(
            ip=s.ip,
            dp=s.dp,
            tick=s.tick,
            halted=s.halted,
            waiting_for_input=s.waiting_for_input,
            reason=reason,
            command=command,
        )
        if command in "+-,":
            record.cell_addr = s.dp
            record.cell_was_present = s.dp in s.memory
            record.cell_value = s.memory.get(s.dp, 0)
        if command == ".":
            record.output_len = len(s.output)
        return record

    def _restore_cell_from_undo(self, record: UndoRecord) -> None:
        if record.cell_addr is None:
            return
        if record.cell_was_present:
            self.state.memory[record.cell_addr] = record.cell_value
        else:
            self.state.memory.pop(record.cell_addr, None)

    def _discard_input_source(self, source_id: int) -> None:
        # Terminal input is interactive, not a deterministic replay tape. When the
        # user reverses over a comma, the line that supplied that byte should be
        # considered unaccepted by the program; otherwise running forward again
        # silently receives the same queued input. Remove all still-queued bytes
        # from that submitted line, but keep other queued lines intact.
        self.state.input_queue = deque(
            item for item in self.state.input_queue if item.source_id != source_id
        )

    def edit_cell(self, addr: int, value: int) -> None:
        record = UndoRecord(
            ip=self.state.ip,
            dp=self.state.dp,
            tick=self.state.tick,
            halted=self.state.halted,
            waiting_for_input=self.state.waiting_for_input,
            reason="edit cell",
            command="edit",
            cell_addr=addr,
            cell_was_present=addr in self.state.memory,
            cell_value=self.state.memory.get(addr, 0),
        )
        self.history.append(record)
        self._set_cell(addr, value)

    def reset(self) -> None:
        self.state = BFState()
        self.history.clear()
        self.error = None
        self._set_halted_if_needed()

    def step_forward(self, *, record_history: bool = True, reason: str = "step") -> str:
        s = self.state
        if not self.program:
            s.halted = True
            return "Program is empty."
        if s.halted:
            return "Program has halted. Press Left to reverse."
        if s.ip < 0 or s.ip >= len(self.program):
            s.halted = True
            return "Program has halted. Press Left to reverse."

        cmd = self.program[s.ip]
        if cmd == "," and not s.input_queue:
            s.waiting_for_input = True
            return "Input needed: press i, type a line, then Enter."

        undo = self._make_undo_record(cmd, reason) if record_history else None
        s.waiting_for_input = False

        if cmd == ">":
            s.dp += 1
            s.ip += 1
        elif cmd == "<":
            s.dp -= 1
            s.ip += 1
        elif cmd == "+":
            self._set_cell(s.dp, self._cell(s.dp) + 1)
            s.ip += 1
        elif cmd == "-":
            self._set_cell(s.dp, self._cell(s.dp) - 1)
            s.ip += 1
        elif cmd == ".":
            s.output.append(self._cell(s.dp) & 0xFF)
            s.ip += 1
        elif cmd == ",":
            input_byte = s.input_queue.popleft()
            if undo is not None:
                undo.consumed_input = input_byte
            self._set_cell(s.dp, input_byte.value)
            s.ip += 1
        elif cmd == "[":
            if self._cell(s.dp) == 0:
                s.ip = self.brackets[s.ip] + 1
            else:
                s.ip += 1
        elif cmd == "]":
            if self._cell(s.dp) != 0:
                s.ip = self.brackets[s.ip] + 1
            else:
                s.ip += 1

        s.tick += 1
        self._set_halted_if_needed()
        if undo is not None:
            self.history.append(undo)
        return f"Executed {cmd!r}."

    def step_reverse(self) -> str:
        if not self.history:
            return "No earlier state available."
        undo = self.history.pop()
        s = self.state
        self._restore_cell_from_undo(undo)
        if undo.output_len is not None:
            del s.output[undo.output_len:]
        if undo.consumed_input is not None:
            self._discard_input_source(undo.consumed_input.source_id)
        s.ip = undo.ip
        s.dp = undo.dp
        s.tick = undo.tick
        s.halted = undo.halted
        s.waiting_for_input = undo.waiting_for_input
        if undo.command == "edit":
            return "Reversed cell edit."
        return f"Reversed {undo.command!r} ({undo.reason})."

    def current_command(self) -> str:
        if 0 <= self.state.ip < len(self.program):
            return self.program[self.state.ip]
        return "∎"


class DebuggerUI:
    def __init__(self, stdscr: "curses._CursesWindow", machine: BrainfuckMachine, *, delay: float, run_limit: int, fast_run_limit: int) -> None:
        self.stdscr = stdscr
        self.machine = machine
        self.delay = delay
        self.run_limit = run_limit
        self.fast_run_limit = fast_run_limit

        self.program_follow_ip = True
        self.program_center = 0

        self.memory_follow_dp = True
        self.memory_center = 0

        self.output_mode_idx = 0
        self.output_scroll = 0  # 0 means bottom, positive means lines above bottom.

        self.input_mode = False
        self.input_line = ""
        self.input_history: List[str] = []

        self.prompt_mode: Optional[str] = None
        self.prompt_text = ""
        self.prompt_value = ""

        self.running = False
        self.cooperative_run_label: Optional[str] = None
        self.request_quit = False
        # Fast runs execute in chunks and yield to curses periodically. This keeps
        # output visible and lets keys like R/Esc, q, x, v, and scrolling work
        # while a long-running program is busy.
        self.fast_chunk_size = 4096
        self.fast_yield_interval = 0.05
        self.status = "Ready. Press ? for help."
        self.show_help = False

    @property
    def output_mode(self) -> str:
        return OUTPUT_MODES[self.output_mode_idx]

    def run(self) -> None:
        curses.curs_set(0)
        self.stdscr.keypad(True)
        self.stdscr.timeout(-1)
        self._init_colors()

        while True:
            self._draw()
            if self.running:
                self.stdscr.timeout(0)
                ch = self.stdscr.getch()
                if ch == -1:
                    msg = self.machine.step_forward()
                    self.status = msg
                    self._after_execution_step()
                    if self.machine.state.halted or self.machine.state.waiting_for_input:
                        self.running = False
                    time.sleep(self.delay)
                    continue
            else:
                self.stdscr.timeout(-1)
                ch = self.stdscr.getch()

            if ch == -1:
                continue
            should_quit = self._handle_key(ch)
            if should_quit or self.request_quit:
                return

    def _init_colors(self) -> None:
        if not curses.has_colors():
            return
        curses.start_color()
        curses.use_default_colors()
        curses.init_pair(1, curses.COLOR_BLACK, curses.COLOR_CYAN)   # active/current
        curses.init_pair(2, curses.COLOR_YELLOW, -1)                 # pointer/IP marker
        curses.init_pair(3, curses.COLOR_GREEN, -1)                  # ok/status
        curses.init_pair(4, curses.COLOR_RED, -1)                    # warning/error
        curses.init_pair(5, curses.COLOR_MAGENTA, -1)                # secondary marker

    def _color(self, pair: int) -> int:
        if curses.has_colors():
            return curses.color_pair(pair)
        return 0

    def _handle_key(self, ch: int) -> bool:
        if self.prompt_mode is not None:
            return self._handle_prompt_key(ch)
        if self.input_mode:
            return self._handle_input_key(ch)
        if self.show_help:
            self.show_help = False
            return False

        if ch in (ord("q"), ord("Q")):
            return True
        if ch == ord("?"):
            self.show_help = True
            return False
        if ch in (curses.KEY_RIGHT, ord(" "), ord("n"), ord("N")):
            self.status = self.machine.step_forward()
            self._after_execution_step()
            return False
        if ch in (curses.KEY_LEFT, curses.KEY_BACKSPACE, 127, 8):
            self.status = self.machine.step_reverse()
            self._after_execution_step()
            return False
        if ch == ord("r"):
            self.running = not self.running
            self.status = "Animated debug run." if self.running else "Paused."
            return False
        if ch == ord("R"):
            self._fast_run_to_stop()
            return False
        if ch == ord("x"):
            self._reset_program_state()
            return False
        if ch == ord("i"):
            self.input_mode = True
            self.status = "Input mode: type a line, Enter submits, Esc cancels."
            return False
        if ch == ord("v"):
            self.output_mode_idx = (self.output_mode_idx + 1) % len(OUTPUT_MODES)
            self.output_scroll = 0
            self.status = f"Output mode: {self.output_mode}."
            return False
        if ch == ord("o"):
            self._run_to_next_bracket("[", fast=False)
            return False
        if ch == ord("O"):
            self._run_to_next_bracket("[", fast=True)
            return False
        if ch == ord("c"):
            self._run_to_next_bracket("]", fast=False)
            return False
        if ch == ord("C"):
            self._run_to_next_bracket("]", fast=True)
            return False
        if ch == ord("b"):
            self._run_to_matching_bracket(fast=False)
            return False
        if ch == ord("B"):
            self._run_to_matching_bracket(fast=True)
            return False
        if ch == ord("."):
            self._run_to_next_command({"."}, "output instruction '.'", fast=False)
            return False
        if ch == ord(">"):
            self._run_to_next_command({"."}, "output instruction '.'", fast=True)
            return False
        if ch == ord(","):
            self._run_to_next_command({","}, "input instruction ','", fast=False)
            return False
        if ch == ord("<"):
            self._run_to_next_command({","}, "input instruction ','", fast=True)
            return False
        if ch == ord("["):
            self._jump_program_to_next("[")
            return False
        if ch == ord("]"):
            self._jump_program_to_next("]")
            return False
        if ch == ord("m"):
            self._jump_program_to_matching_bracket()
            return False
        if ch == ord("{"):
            self._scroll_program(-1)
            return False
        if ch == ord("}"):
            self._scroll_program(1)
            return False
        if ch == ord("("):
            self._scroll_program(-10)
            return False
        if ch == ord(")"):
            self._scroll_program(10)
            return False
        if ch == ord("p"):
            self.program_follow_ip = True
            self.program_center = self.machine.state.ip
            self.status = "Program view follows the instruction pointer."
            return False
        if ch == ord("h"):
            self._scroll_memory(-1)
            return False
        if ch == ord("l"):
            self._scroll_memory(1)
            return False
        if ch == ord("H"):
            self._scroll_memory(-10)
            return False
        if ch == ord("L"):
            self._scroll_memory(10)
            return False
        if ch == ord("d"):
            self.memory_follow_dp = True
            self.memory_center = self.machine.state.dp
            self.status = "Memory view follows the data pointer."
            return False
        if ch == ord("f"):
            self.memory_follow_dp = not self.memory_follow_dp
            if self.memory_follow_dp:
                self.memory_center = self.machine.state.dp
            self.status = "Memory follow is on." if self.memory_follow_dp else "Memory follow is off."
            return False
        if ch == ord("g"):
            self._begin_prompt("goto", "Go to memory address")
            return False
        if ch == ord("e"):
            addr = self._current_memory_center()
            self._begin_prompt("edit", f"Set cell {addr} to")
            return False
        if ch in (curses.KEY_PPAGE, ord("u")):
            self.output_scroll += 1
            self.status = "Output scrolled up."
            return False
        if ch in (curses.KEY_NPAGE, ord("j")):
            self.output_scroll = max(0, self.output_scroll - 1)
            self.status = "Output scrolled down."
            return False
        if ch == curses.KEY_HOME:
            self.output_scroll = 10**9
            self.status = "Output scrolled to the top."
            return False
        if ch == curses.KEY_END:
            self.output_scroll = 0
            self.status = "Output follows the bottom."
            return False

        self.status = "Unknown key. Press ? for help."
        return False

    def _handle_input_key(self, ch: int) -> bool:
        if ch in (27,):  # Esc
            self.input_mode = False
            self.status = "Input mode cancelled."
            return False
        if ch in (10, 13, curses.KEY_ENTER):
            self.machine.enqueue_input_line(self.input_line)
            self.input_history.append(self.input_line)
            self.input_line = ""
            self.input_mode = False
            self.status = "Input line queued. Press Right to consume it."
            return False
        if ch in (curses.KEY_BACKSPACE, 127, 8):
            self.input_line = self.input_line[:-1]
            return False
        if ch == curses.KEY_DC:
            self.input_line = ""
            return False
        if 0 <= ch < 256:
            char = chr(ch)
            if char.isprintable() or char == "\t":
                self.input_line += char
        return False

    def _begin_prompt(self, mode: str, text: str) -> None:
        self.prompt_mode = mode
        self.prompt_text = text
        self.prompt_value = ""
        self.status = f"{text}: "

    def _handle_prompt_key(self, ch: int) -> bool:
        if ch == 27:
            self.prompt_mode = None
            self.status = "Prompt cancelled."
            return False
        if ch in (10, 13, curses.KEY_ENTER):
            value = self.prompt_value.strip()
            try:
                number = int(value, 0)
            except ValueError:
                self.status = f"Invalid number: {value!r}."
                self.prompt_mode = None
                return False
            if self.prompt_mode == "goto":
                self.memory_follow_dp = False
                self.memory_center = number
                self.status = f"Memory view centered at address {number}."
            elif self.prompt_mode == "edit":
                addr = self._current_memory_center()
                self.machine.edit_cell(addr, number)
                self.status = f"Cell {addr} set to {number}."
            self.prompt_mode = None
            return False
        if ch in (curses.KEY_BACKSPACE, 127, 8):
            self.prompt_value = self.prompt_value[:-1]
            return False
        if 0 <= ch < 256:
            char = chr(ch)
            if char.isprintable():
                self.prompt_value += char
        return False

    def _after_execution_step(self) -> None:
        s = self.machine.state
        self.program_follow_ip = True
        self.program_center = s.ip
        if self.memory_follow_dp:
            self.memory_center = s.dp
        self.output_scroll = min(self.output_scroll, 10**9)


    def _reset_program_state(self) -> None:
        self.running = False
        self.machine.reset()
        self.program_follow_ip = True
        self.program_center = self.machine.state.ip
        self.memory_follow_dp = True
        self.memory_center = self.machine.state.dp
        self.output_scroll = 0
        self.input_mode = False
        self.prompt_mode = None
        self.input_line = ""
        self.status = "Program reset: ip=0, dp=0, memory/output/input queue cleared."

    def _fast_run_to_stop(self) -> None:
        self.running = False
        self._run_until_stop(fast=True, description="halt/input/limit")

    def _run_to_next_bracket(self, needle: str, *, fast: bool) -> None:
        program = self.machine.program
        s = self.machine.state
        if not program or s.halted:
            self.status = "Program is not running."
            return
        target = None
        for idx in range(s.ip + 1, len(program)):
            if program[idx] == needle:
                target = idx
                break
        if target is None:
            self.status = f"No later {needle!r} exists in the program."
            return
        self._run_until_ip(target, f"next {needle!r} at instruction {target}", fast=fast)

    def _run_to_matching_bracket(self, *, fast: bool) -> None:
        program = self.machine.program
        s = self.machine.state
        if not (0 <= s.ip < len(program)) or s.halted:
            self.status = "The instruction pointer is not on a program instruction."
            return
        cmd = program[s.ip]
        if cmd not in "[]":
            self.status = "The current instruction is not a bracket."
            return

        match = self.machine.brackets[s.ip]
        if cmd == "[":
            if self.machine.state.memory.get(self.machine.state.dp, 0) == 0:
                before = s.ip
                msg = self._step_once_for_run(fast=fast, reason="fast loop skip" if fast else "loop skip")
                self._after_execution_step()
                self.status = (
                    f"Executed '[' at {before}; cell was zero, so the loop was "
                    f"skipped to instruction {self.machine.state.ip}."
                )
                if msg.startswith("Input needed"):
                    self.status = msg
                return
            self._run_until_ip(match, f"matching ']' at instruction {match}", fast=fast)
            return

        # A Brainfuck closing bracket jumps to the instruction after its matching
        # opening bracket, so the matching '[' is not itself fetched again during
        # normal forward execution. Treat this as a dynamic loop-boundary step.
        before = s.ip
        value = self.machine.state.memory.get(self.machine.state.dp, 0)
        msg = self._step_once_for_run(fast=fast, reason="fast loop boundary" if fast else "loop boundary")
        self._after_execution_step()
        if msg.startswith("Input needed") or msg.startswith("Program"):
            self.status = msg
        elif value != 0:
            mode = "Fast-executed" if fast else "Executed"
            self.status = (
                f"{mode} ']' at {before}; looped back after matching '[' "
                f"at instruction {match}."
            )
        else:
            mode = "Fast-executed" if fast else "Executed"
            self.status = f"{mode} ']' at {before}; loop exited."

    def _run_to_next_command(self, needles: Set[str], description: str, *, fast: bool) -> None:
        program = self.machine.program
        s = self.machine.state
        if not program or s.halted:
            self.status = "Program is not running."
            return
        if 0 <= s.ip < len(program) and program[s.ip] in needles:
            self.status = f"Already at {description} at instruction {s.ip}."
            self._after_execution_step()
            return
        self._run_loop(
            target=None,
            stop_commands=needles,
            description=f"next {description}",
            fast=fast,
            reason="run-to-io",
        )

    def _step_once_for_run(self, *, fast: bool, reason: str) -> str:
        return self.machine.step_forward(reason=reason)

    def _run_until_stop(self, *, fast: bool, description: str) -> None:
        self._run_loop(target=None, stop_commands=None, description=description, fast=fast, reason="run")

    def _run_until_ip(self, target: int, description: str, *, fast: bool) -> None:
        if self.machine.state.ip == target:
            self.status = f"Already at {description}."
            self._after_execution_step()
            return
        self._run_loop(target=target, stop_commands=None, description=description, fast=fast, reason="run-to-bracket")

    def _run_loop(self, *, target: Optional[int], stop_commands: Optional[Set[str]], description: str, fast: bool, reason: str) -> None:
        """Run until a dynamic stop condition, yielding to the UI along the way.

        Earlier fast runs were one blocking Python loop: very quick, but the
        output pane did not update and curses could not see new key presses. This
        loop keeps the same execution model, but redraws and polls the keyboard
        at chunk boundaries. It is still much faster than animated stepping
        because it avoids one redraw per instruction.
        """
        started = time.perf_counter()
        start_tick = self.machine.state.tick
        limit = self.fast_run_limit if fast else self.run_limit
        mode_label = "FAST RUN" if fast else "DEBUG RUN"
        self.cooperative_run_label = mode_label
        next_yield = time.perf_counter() + self.fast_yield_interval
        steps_since_yield = 0
        reason_text = "fast run" if fast else reason

        try:
            while True:
                s = self.machine.state
                steps = s.tick - start_tick

                if target is not None and s.ip == target:
                    self.status = self._run_done_message(steps, started, f"reached {description}", fast)
                    self._after_execution_step()
                    return
                if (
                    stop_commands is not None
                    and 0 <= s.ip < len(self.machine.program)
                    and self.machine.program[s.ip] in stop_commands
                ):
                    self.status = self._run_done_message(steps, started, f"reached {description} at instruction {s.ip}", fast)
                    self._after_execution_step()
                    return
                if s.halted:
                    if target is None:
                        outcome = "program halted"
                    else:
                        outcome = f"program halted before reaching {description}"
                    self.status = self._run_done_message(steps, started, outcome, fast)
                    self._after_execution_step()
                    return
                if s.waiting_for_input:
                    if target is None:
                        outcome = "input is needed"
                    else:
                        outcome = f"input is needed before reaching {description}"
                    self.status = self._run_done_message(steps, started, outcome, fast)
                    self._after_execution_step()
                    return
                if steps >= limit:
                    if target is None:
                        outcome = f"run limit ({limit}) reached"
                    else:
                        outcome = f"run limit ({limit}) reached before {description}"
                    self.status = self._run_done_message(steps, started, outcome, fast)
                    self._after_execution_step()
                    return

                before_tick = s.tick
                msg = self.machine.step_forward(reason=reason_text)
                if self.machine.state.tick == before_tick:
                    # No instruction was executed, usually because a ',' needs input.
                    self.status = msg
                    self._after_execution_step()
                    return

                steps_since_yield += 1
                now = time.perf_counter()
                if fast:
                    should_yield = steps_since_yield >= self.fast_chunk_size or now >= next_yield
                else:
                    should_yield = True

                if should_yield:
                    if self._yield_during_run(
                        started=started,
                        start_tick=start_tick,
                        description=description,
                        fast=fast,
                    ):
                        return
                    steps_since_yield = 0
                    next_yield = time.perf_counter() + self.fast_yield_interval
                    if not fast and self.delay > 0:
                        time.sleep(self.delay)
        finally:
            self.cooperative_run_label = None

    def _yield_during_run(self, *, started: float, start_tick: int, description: str, fast: bool) -> bool:
        """Redraw and process pending keys during a cooperative run.

        Returns True when the active run should stop because the user paused,
        reset, requested input mode, or quit.
        """
        self._after_execution_step()
        steps = self.machine.state.tick - start_tick
        elapsed = max(0.0, time.perf_counter() - started)
        rate = steps / elapsed if elapsed > 0 else 0.0
        mode = "fast run" if fast else "debug run"
        if rate > 0:
            self.status = f"{mode}: running toward {description}; {steps} steps ({rate:,.0f} instr/s). R/Esc pauses."
        else:
            self.status = f"{mode}: running toward {description}; {steps} steps. R/Esc pauses."
        self._draw()

        self.stdscr.timeout(0)
        while True:
            ch = self.stdscr.getch()
            if ch == -1:
                break
            action = self._handle_key_during_run(ch)
            if action == "quit":
                self.request_quit = True
                return True
            if action == "pause":
                return True
        return False

    def _handle_key_during_run(self, ch: int) -> str:
        """Handle a safe subset of hotkeys while a run command is active.

        It deliberately does not start nested run commands. Keys that would open
        an editing prompt pause the run first, so the normal event loop can take
        over cleanly.
        """
        if ch in (ord("q"), ord("Q")):
            self.status = "Quit requested."
            return "quit"
        if ch in (27, ord("R"), ord("r")):
            self.running = False
            self.status = "Run paused."
            return "pause"
        if ch in (curses.KEY_LEFT, curses.KEY_BACKSPACE, 127, 8):
            self.status = self.machine.step_reverse()
            self._after_execution_step()
            return "pause"
        if ch == ord("x"):
            self._reset_program_state()
            return "pause"
        if ch == ord("?"):
            self.show_help = True
            self.status = "Run paused for help."
            return "pause"
        if ch == ord("i"):
            self.input_mode = True
            self.status = "Run paused. Input mode: type a line, Enter submits, Esc cancels."
            return "pause"
        if ch in (ord("g"), ord("e")):
            if ch == ord("g"):
                self._begin_prompt("goto", "Go to memory address")
            else:
                addr = self._current_memory_center()
                self._begin_prompt("edit", f"Set cell {addr} to")
            return "pause"
        if ch == ord("v"):
            self.output_mode_idx = (self.output_mode_idx + 1) % len(OUTPUT_MODES)
            self.output_scroll = 0
            self.status = f"Output mode: {self.output_mode}."
            return "continue"
        if ch in (curses.KEY_PPAGE, ord("u")):
            self.output_scroll += 1
            self.status = "Output scrolled up."
            return "continue"
        if ch in (curses.KEY_NPAGE, ord("j")):
            self.output_scroll = max(0, self.output_scroll - 1)
            self.status = "Output scrolled down."
            return "continue"
        if ch == curses.KEY_HOME:
            self.output_scroll = 10**9
            self.status = "Output scrolled to the top."
            return "continue"
        if ch == curses.KEY_END:
            self.output_scroll = 0
            self.status = "Output follows the bottom."
            return "continue"
        if ch == ord("["):
            self._jump_program_to_next("[")
            return "continue"
        if ch == ord("]"):
            self._jump_program_to_next("]")
            return "continue"
        if ch == ord("m"):
            self._jump_program_to_matching_bracket()
            return "continue"
        if ch in (ord("."), ord(">"), ord(","), ord("<")):
            self.status = "Already running. Press R or Esc to pause before starting another run command."
            return "continue"
        if ch == ord("{"):
            self._scroll_program(-1)
            return "continue"
        if ch == ord("}"):
            self._scroll_program(1)
            return "continue"
        if ch == ord("("):
            self._scroll_program(-10)
            return "continue"
        if ch == ord(")"):
            self._scroll_program(10)
            return "continue"
        if ch == ord("p"):
            self.program_follow_ip = True
            self.program_center = self.machine.state.ip
            self.status = "Program view follows the instruction pointer."
            return "continue"
        if ch == ord("h"):
            self._scroll_memory(-1)
            return "continue"
        if ch == ord("l"):
            self._scroll_memory(1)
            return "continue"
        if ch == ord("H"):
            self._scroll_memory(-10)
            return "continue"
        if ch == ord("L"):
            self._scroll_memory(10)
            return "continue"
        if ch == ord("d"):
            self.memory_follow_dp = True
            self.memory_center = self.machine.state.dp
            self.status = "Memory view follows the data pointer."
            return "continue"
        if ch == ord("f"):
            self.memory_follow_dp = not self.memory_follow_dp
            if self.memory_follow_dp:
                self.memory_center = self.machine.state.dp
            self.status = "Memory follow is on." if self.memory_follow_dp else "Memory follow is off."
            return "continue"
        if ch in (ord("o"), ord("O"), ord("c"), ord("C"), ord("b"), ord("B"), ord("."), ord(">"), ord(","), ord("<")):
            self.status = "Already running. Press R or Esc to pause before starting another run command."
            return "continue"
        return "continue"

    def _run_done_message(self, steps: int, started: float, outcome: str, fast: bool) -> str:
        elapsed = max(0.0, time.perf_counter() - started)
        mode = "fast run" if fast else "debug run"
        if elapsed > 0:
            rate = steps / elapsed
            return f"{mode}: {outcome} after {steps} steps in {elapsed:.3f}s ({rate:,.0f} instr/s)."
        return f"{mode}: {outcome} after {steps} steps."

    def _scroll_program(self, delta: int) -> None:
        program = self.machine.program
        if not program:
            self.status = "Program is empty."
            return
        center = self._current_program_center() + delta
        center = max(0, min(len(program), center))
        self.program_follow_ip = False
        self.program_center = center
        self.status = f"Program view centered at instruction {center}. Press p to follow IP."

    def _jump_program_to_next(self, needle: str) -> None:
        program = self.machine.program
        if not program:
            self.status = "Program is empty."
            return
        center = self._current_program_center()
        for idx in range(center + 1, len(program)):
            if program[idx] == needle:
                self.program_follow_ip = False
                self.program_center = idx
                self.status = f"Program view jumped to instruction {idx} ({needle}). Press p to follow IP."
                return
        self.status = f"No later {needle!r} found."

    def _jump_program_to_matching_bracket(self) -> None:
        center = self._current_program_center()
        match = self.machine.brackets.get(center)
        if match is None:
            self.status = "The centered instruction is not a bracket."
            return
        self.program_follow_ip = False
        self.program_center = match
        self.status = f"Program view jumped to matching bracket at instruction {match}. Press p to follow IP."

    def _scroll_memory(self, delta: int) -> None:
        self.memory_follow_dp = False
        self.memory_center = self._current_memory_center() + delta
        self.status = f"Memory view centered at address {self.memory_center}. Press d to follow DP."

    def _current_program_center(self) -> int:
        if self.program_follow_ip:
            return self.machine.state.ip
        return self.program_center

    def _current_memory_center(self) -> int:
        if self.memory_follow_dp:
            return self.machine.state.dp
        return self.memory_center

    def _draw(self) -> None:
        self.stdscr.erase()
        height, width = self.stdscr.getmaxyx()
        if height < 18 or width < 70:
            self._draw_too_small(height, width)
            self.stdscr.refresh()
            return

        program_h = 7
        input_h = 6
        mid_h = height - program_h - input_h - 1
        memory_w = max(36, width // 2)
        output_w = width - memory_w

        program_win = self.stdscr.derwin(program_h, width, 0, 0)
        memory_win = self.stdscr.derwin(mid_h, memory_w, program_h, 0)
        output_win = self.stdscr.derwin(mid_h, output_w, program_h, memory_w)
        input_win = self.stdscr.derwin(input_h, width, program_h + mid_h, 0)
        status_win = self.stdscr.derwin(1, width, height - 1, 0)

        self._draw_program(program_win)
        self._draw_memory(memory_win)
        self._draw_output(output_win)
        self._draw_input(input_win)
        self._draw_status(status_win)

        if self.show_help:
            self._draw_help_overlay(height, width)

        self.stdscr.refresh()

    def _draw_too_small(self, height: int, width: int) -> None:
        msg = f"Terminal too small: {width}x{height}. Need at least 70x18."
        self.stdscr.addnstr(0, 0, msg, max(0, width - 1), self._color(4) | curses.A_BOLD)

    def _draw_box_title(self, win: "curses._CursesWindow", title: str) -> None:
        win.box()
        max_y, max_x = win.getmaxyx()
        label = f" {title} "
        win.addnstr(0, 2, label, max(0, max_x - 4), curses.A_BOLD)

    def _draw_program(self, win: "curses._CursesWindow") -> None:
        s = self.machine.state
        center = self._current_program_center()
        program = self.machine.program
        title = (
            f"Program  tick={s.tick}  ip={s.ip}  cmd={self.machine.current_command()!r}  "
            f"dp={s.dp}  undo={len(self.machine.history)}"
        )
        if s.halted:
            title += "  HALTED"
        if s.waiting_for_input:
            title += "  WAITING FOR INPUT"
        if not self.program_follow_ip:
            title += f"  view={center}"
        self._draw_box_title(win, title)

        max_y, max_x = win.getmaxyx()
        if not program:
            win.addnstr(2, 2, "No Brainfuck commands in program.", max_x - 4)
            return

        cell_w = 7
        usable_w = max_x - 4
        slots = max(1, usable_w // cell_w)
        if slots % 2 == 0:
            slots -= 1
        left = center - slots // 2
        x0 = 2 + max(0, (usable_w - slots * cell_w) // 2)

        rows = [2, 3, 4]
        labels = ("idx", "cmd", "mark")
        for row, label in zip(rows, labels):
            if row < max_y - 1:
                win.addnstr(row, 2, label, min(4, max_x - 4), curses.A_DIM)

        for slot in range(slots):
            idx = left + slot
            x = x0 + slot * cell_w
            if idx < 0 or idx > len(program):
                txt_idx, txt_cmd, txt_mark = "", "", ""
                attr = curses.A_DIM
            else:
                cmd = "∎" if idx == len(program) else program[idx]
                txt_idx = str(idx)[-cell_w:].center(cell_w)
                txt_cmd = cmd.center(cell_w)
                marks = []
                if idx == s.ip:
                    marks.append("IP")
                if idx == center:
                    marks.append("^")
                txt_mark = "/".join(marks).center(cell_w)
                attr = 0
                if idx == center:
                    attr |= curses.A_BOLD
                if idx == s.ip:
                    attr |= self._color(2) | curses.A_BOLD
                if idx == center:
                    attr |= self._color(1)
            for y, text in zip(rows, (txt_idx, txt_cmd, txt_mark)):
                if y < max_y - 1 and x < max_x - 1:
                    win.addnstr(y, x, text, min(cell_w, max_x - x - 1), attr)

        footer = "Right step  Left undo  r/R run  o/O [  c/C ]  ./, I/O  x reset  {}/() scroll"
        win.addnstr(max_y - 2, 2, footer, max_x - 4, curses.A_DIM)

    def _draw_memory(self, win: "curses._CursesWindow") -> None:
        s = self.machine.state
        center = self._current_memory_center()
        follow = "follow" if self.memory_follow_dp else "free"
        self._draw_box_title(win, f"Memory tape  dp={s.dp}  center={center}  {follow}")
        max_y, max_x = win.getmaxyx()
        cell_w = 8
        usable_w = max_x - 8
        slots = max(1, usable_w // cell_w)
        if slots % 2 == 0:
            slots -= 1
        left = center - slots // 2
        x0 = 7

        labels = [(2, "addr"), (3, "dec"), (4, "hex"), (5, "chr")]
        for y, label in labels:
            if y < max_y - 1:
                win.addnstr(y, 2, label, min(4, max_x - 4), curses.A_DIM)

        for slot in range(slots):
            addr = left + slot
            val = s.memory.get(addr, 0)
            x = x0 + slot * cell_w
            char = self._printable_byte(val)
            texts = (
                str(addr)[-cell_w:].center(cell_w),
                str(val).center(cell_w),
                f"{val & 0xFF:02X}".center(cell_w),
                char.center(cell_w),
            )
            attr = 0
            if addr == s.dp:
                attr |= self._color(2) | curses.A_BOLD
            if addr == center:
                attr |= self._color(1)
            for y, text in zip((2, 3, 4, 5), texts):
                if y < max_y - 1 and x < max_x - 1:
                    win.addnstr(y, x, text, min(cell_w, max_x - x - 1), attr)

        used_cells = len(s.memory)
        footer1 = f"used={used_cells} queued-input={len(s.input_queue)}"
        footer2 = "h/l scroll  H/L page  g goto  d jump-to-DP  f follow  e edit"
        if max_y >= 9:
            win.addnstr(max_y - 3, 2, footer1, max_x - 4, curses.A_DIM)
            win.addnstr(max_y - 2, 2, footer2, max_x - 4, curses.A_DIM)
        elif max_y >= 7:
            win.addnstr(max_y - 2, 2, footer2, max_x - 4, curses.A_DIM)

    @staticmethod
    def _printable_byte(value: int) -> str:
        b = value & 0xFF
        if b == 10:
            return "\\n"
        if b == 13:
            return "\\r"
        if b == 9:
            return "\\t"
        if 32 <= b <= 126:
            return chr(b)
        return "·"

    def _draw_output(self, win: "curses._CursesWindow") -> None:
        output = bytes(self.machine.state.output)
        self._draw_box_title(win, f"Output  mode={self.output_mode}  bytes={len(output)}")
        max_y, max_x = win.getmaxyx()
        body_h = max_y - 3
        body_w = max_x - 4
        lines = self._format_output_lines(output, body_w)
        if not lines:
            lines = [""]

        max_scroll = max(0, len(lines) - body_h)
        self.output_scroll = min(self.output_scroll, max_scroll)
        start = max(0, len(lines) - body_h - self.output_scroll)
        visible = lines[start:start + body_h]

        for row, line in enumerate(visible, start=1):
            if row >= max_y - 1:
                break
            win.addnstr(row, 2, line, body_w)

        footer = "v mode  PgUp/u scroll up  PgDn/j down  End bottom"
        win.addnstr(max_y - 2, 2, footer, max_x - 4, curses.A_DIM)

    def _format_output_lines(self, output: bytes, width: int) -> List[str]:
        width = max(1, width)
        if self.output_mode == "ascii":
            text_chars: List[str] = []
            for b in output:
                if b == 10:
                    text_chars.append("\n")
                elif b == 13:
                    continue
                elif b == 9:
                    text_chars.append("\t")
                elif 32 <= b <= 126:
                    text_chars.append(chr(b))
                else:
                    text_chars.append("·")
            return self._wrap_text("".join(text_chars), width)
        if self.output_mode == "hex":
            return self._wrap_tokens((f"{b:02X}" for b in output), width)
        return self._wrap_tokens((str(b) for b in output), width)

    @staticmethod
    def _wrap_text(text: str, width: int) -> List[str]:
        lines: List[str] = []
        for raw in text.split("\n"):
            if raw == "":
                lines.append("")
                continue
            while len(raw) > width:
                lines.append(raw[:width])
                raw = raw[width:]
            lines.append(raw)
        return lines

    @staticmethod
    def _wrap_tokens(tokens: Iterable[str], width: int) -> List[str]:
        lines: List[str] = []
        current = ""
        for token in tokens:
            piece = token if not current else " " + token
            if len(current) + len(piece) > width:
                if current:
                    lines.append(current)
                current = token
            else:
                current += piece
        if current or not lines:
            lines.append(current)
        return lines

    def _draw_input(self, win: "curses._CursesWindow") -> None:
        mode = "INPUT" if self.input_mode else "normal"
        queued = len(self.machine.state.input_queue)
        self._draw_box_title(win, f"Input terminal  {mode}  queued-bytes={queued}")
        max_y, max_x = win.getmaxyx()
        body_w = max_x - 4
        history_rows = max(0, max_y - 4)
        recent = self.input_history[-history_rows:]
        for i, line in enumerate(recent, start=1):
            prefix = "< "
            win.addnstr(i, 2, prefix + line, body_w, curses.A_DIM)
        prompt_attr = self._color(3) | curses.A_BOLD if self.input_mode else curses.A_DIM
        cursor = "█" if self.input_mode else ""
        prompt = "> " + self.input_line + cursor
        win.addnstr(max_y - 2, 2, prompt, body_w, prompt_attr)
        footer = "i input mode: type line, Enter queues newline, Esc cancels"
        win.addnstr(max_y - 1, 2, footer, body_w, curses.A_DIM)

    def _draw_status(self, win: "curses._CursesWindow") -> None:
        win.erase()
        max_y, max_x = win.getmaxyx()
        if self.prompt_mode is not None:
            text = f"{self.prompt_text}: {self.prompt_value}█"
            attr = self._color(3) | curses.A_BOLD
        else:
            run = "DEBUG RUN " if self.running else (f"{self.cooperative_run_label} " if self.cooperative_run_label else "")
            text = run + self.status
            attr = self._color(3)
            if "error" in text.lower() or "invalid" in text.lower() or "unmatched" in text.lower():
                attr = self._color(4) | curses.A_BOLD
            if self.machine.state.waiting_for_input:
                attr = self._color(4) | curses.A_BOLD
        win.addnstr(0, 0, text, max_x - 1, attr)

    def _draw_help_overlay(self, height: int, width: int) -> None:
        overlay_h = min(height - 4, 25)
        overlay_w = min(width - 8, 88)
        y = max(0, (height - overlay_h) // 2)
        x = max(0, (width - overlay_w) // 2)
        win = self.stdscr.derwin(overlay_h, overlay_w, y, x)
        win.erase()
        win.box()
        title = " bfdbg help "
        win.addnstr(0, 2, title, overlay_w - 4, curses.A_BOLD)
        lines = [
            "Execution",
            "  Right / Space / n   step forward one Brainfuck instruction",
            "  Left / Backspace    reverse one executed instruction/edit",
            "  r                   animated debug-run; redraws and logs compact undo records",
            "  R                   cooperative fast-run to halt/input/limit; periodic redraws",
            "  x                   reset program state",
            "",
            "Run-to-bracket / I/O",
            "  o / c               debug-run to next '[' / ']'",
            "  O / C               fast-run to next '[' / ']'",
            "  b / B               debug/fast run to current loop boundary",
            "  . / >               debug/fast run until next '.' is about to execute",
            "  , / <               debug/fast run until next ',' is about to execute",
            "",
            "Program view",
            "  [ / ]               jump view to next '[' / ']' without running",
            "  m                   jump view to matching bracket if centered on a bracket",
            "  { / }               scroll program view left/right",
            "  ( / )               scroll program view by ten instructions",
            "  p                   recenter view on the current instruction pointer",
            "",
            "Memory tape",
            "  h/l                 scroll memory left/right",
            "  H/L                 scroll memory by ten cells",
            "  g                   goto memory address, decimal or 0xhex",
            "  d                   jump back to data pointer and follow it",
            "  f                   toggle automatic data-pointer follow",
            "  e                   edit the centered memory cell",
            "",
            "Output and input",
            "  v                   cycle output mode: ASCII / hex / decimal",
            "  PgUp/PgDn or u/j    scroll output history",
            "  i                   input mode; Enter submits a line plus newline; Esc cancels",
            "  R/Esc while running pause; q quits; any key closes this help",
        ]
        for row, line in enumerate(lines, start=1):
            if row >= overlay_h - 1:
                break
            attr = curses.A_BOLD if line and not line.startswith(" ") else 0
            win.addnstr(row, 2, line, overlay_w - 4, attr)
        win.refresh()


def load_program(path: str) -> str:
    with open(path, "r", encoding="utf-8", errors="replace") as f:
        return f.read()


def parse_args(argv: Optional[List[str]] = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Brainfuck interpreter with a curses debugger UI.")
    parser.add_argument("program", help="Path to a Brainfuck source file.")
    parser.add_argument(
        "--cell-bits",
        type=int,
        default=8,
        help="Cell width in bits. Default: 8. Use 0 for unbounded integer cells.",
    )
    parser.add_argument(
        "--delay",
        type=float,
        default=0.02,
        help="Delay in seconds between instructions while running continuously. Default: 0.02.",
    )
    parser.add_argument(
        "--run-limit",
        type=int,
        default=100000,
        help="Maximum number of instructions for one debug run-to-bracket command. Default: 100000.",
    )
    parser.add_argument(
        "--fast-run-limit",
        type=int,
        default=10000000,
        help="Maximum number of instructions for one fast run command. Default: 10000000.",
    )
    return parser.parse_args(argv)


def main(argv: Optional[List[str]] = None) -> int:
    locale.setlocale(locale.LC_ALL, "")
    args = parse_args(argv)
    try:
        source = load_program(args.program)
        modulus = None if args.cell_bits == 0 else 1 << args.cell_bits
        machine = BrainfuckMachine(source, cell_modulus=modulus)
    except Exception as exc:
        print(f"bfdbg: {exc}", file=sys.stderr)
        return 2

    def wrapped(stdscr: "curses._CursesWindow") -> None:
        DebuggerUI(
            stdscr,
            machine,
            delay=max(0.0, args.delay),
            run_limit=max(1, args.run_limit),
            fast_run_limit=max(1, args.fast_run_limit),
        ).run()

    try:
        curses.wrapper(wrapped)
    except KeyboardInterrupt:
        pass
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
