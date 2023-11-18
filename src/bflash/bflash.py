#!/usr/bin/env python3

import argparse
import serial
import time
import threading
import math
import readline
import os

DUMP_REQUEST = b"\0"
FLASH_REQUEST = b"\1"
PROGRAMMER_READY = b"\2"
PROGRAMMER_FINISHED = b"\3"

ready = threading.Event()


def show_percentage(current, total, previous, msg):
    new_percentage = math.floor(current / total * 10) * 10
    if new_percentage > previous:
        print(f"{msg} {new_percentage}%")
        return new_percentage
    return previous


def write_data(ser, file):
    data = bytearray(file.read())
    data_length = len(data).to_bytes(4, byteorder='little')

    while not ready.is_set():
        time.sleep(0.5)

    ser.write(FLASH_REQUEST)
    ser.write(data_length)
    ser.write(data)
    ser.flush()


def read_data(ser, file):
    while not ready.is_set():
        time.sleep(0.5)

    ser.write(DUMP_REQUEST)
    data_length = int.from_bytes(ser.read(4), byteorder='little')
    data = bytearray()
    percentage = 0
    for i in range(0, data_length):
        percentage = show_percentage(i, data_length, percentage, "Downloading... ")
        data += ser.read()
    print("Downloading... 100%")
        
    file.write(data)


def serial_monitor(ser, keep_alive):
    try:
        while True:
            c = ser.read()
            if c == PROGRAMMER_READY:
                ready.set()
                if not keep_alive:
                    return
            elif c == PROGRAMMER_FINISHED:
                break
            else:
                print(c.decode(), end="")
    except Exception as e:
        print(f"Error in serial_monitor: {e}")


def input_with_autocomplete(prompt, completer):
    readline.set_completer(completer)
    readline.parse_and_bind('tab: complete')
    try:
        return input(prompt)
    finally:
        readline.set_completer()


def filename_completer(text, state):
    files = [f for f in os.listdir('.') if os.path.isfile(f)]
    files = [f for f in files if f.startswith(text)]
    return files[state] if state < len(files) else None


class SerialHandler:
    def __init__(self, worker, file_flags, prompt, serial_monitor_keep_alive):
        self.worker = worker
        self.file_flags = file_flags
        self.prompt = prompt
        self.serial_monitor_keep_alive = serial_monitor_keep_alive

    def handle(self, args):
        filename = input_with_autocomplete(self.prompt, filename_completer).strip()
        try:
            with serial.Serial(args.port, args.baudrate, timeout=None) as ser, \
                    open(filename, self.file_flags) as file:

                writer = threading.Thread(target=self.worker, \
                                          args=(ser, file))
                monitor = threading.Thread(target=serial_monitor, \
                                           args=(ser, self.serial_monitor_keep_alive))

                writer.start()
                monitor.start()

                writer.join()
                monitor.join()

        except FileNotFoundError:
            print(f"Error: File '{filename}' not found.")
        except serial.SerialException as e:
            print(f"Error: {e}. Check if the specified serial port is valid.")


def prompt():
    while True:
        choice = input_with_autocomplete("Enter (D) to dump the current ROM to a file or (F) to flash a file onto the ROM: ", \
                                         filename_completer).lower()
        if choice in ["d", "f"]:
            return choice
        else:
            print("Invalid choice. Please enter 'D' or 'F'")


def main():
    parser = argparse.ArgumentParser(description='Serial Communication Program')
    parser.add_argument('port', help='Serial port name')
    parser.add_argument('baudrate', type=int, help='Baud rate for the serial connection')

    args = parser.parse_args()

    choice = prompt()
    if choice == "d":
        SerialHandler(read_data, "wb", "Enter the file to dump ROM into: ", False).handle(args)
    elif choice == "f":
        SerialHandler(write_data, "rb", "Enter the file to flash onto ROM: ", True).handle(args)


if __name__ == "__main__":
    main()
