import argparse
import serial
import time
import threading
import math

DUMP_REQUEST = b"\0"
FLASH_REQUEST = b"\1"
PROGRAMMER_READY = b"\2"
PROGRAMMER_FINISHED = b"\3"

ready = threading.Event()

def show_percentage(current, total, previous):
    new_percentage = math.floor(current / total * 10) * 10
    if new_percentage > previous:
        print("{}%".format(new_percentage))
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
    print("Downloading data from ROM")
    percentage = 0
    for i in range(0, data_length):
        percentage = show_percentage(i, data_length, percentage)
        data += ser.read()
    print("100%")

    file.write(data)


def serial_monitor(ser, keep_alive):
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


class SerialHandler:
    def __init__(self, worker, file_flags, prompt, serial_monitor_keep_alive):
        self.worker = worker
        self.file_flags = file_flags
        self.prompt = prompt
        self.serial_monitor_keep_alive = serial_monitor_keep_alive

    def handle(self, args):
        filename = input(self.prompt)
        try:
            with serial.Serial(args.port, args.baudrate, timeout=None) as ser, \
                    open(filename, self.file_flags) as file:

                writer = threading.Thread(target=self.worker, args=(ser, file))
                monitor = threading.Thread(target=serial_monitor, args=(ser, self.serial_monitor_keep_alive))

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
        choice = input("Enter (D) to dump the current ROM to a file or (F) to flash a file onto the ROM: ").lower()
        if choice in ["d", "f"]:
            return choice
        else:
            print("Invalid choice. Please enter 'D' or 'F'")


def main():
    parser = argparse.ArgumentParser(description='Serial Communication Program')
    parser.add_argument('port', help='Serial port name')
    parser.add_argument('baudrate', type=int, help='Baud rate for the serial connection')

    args = parser.parse_args()
    
    dump_policy = SerialHandler(read_data, "wb", "Enter the file to dump ROM into: ", False)
    flash_policy = SerialHandler(write_data, "rb", "Enter the file to flash onto ROM: ", True)
    
    while True:
        choice = prompt()
        if choice == "d":
            dump_policy.handle(args)
            break
        elif choice == "f":
            flash_policy.handle(args)
            break


if __name__ == "__main__":
    main()
