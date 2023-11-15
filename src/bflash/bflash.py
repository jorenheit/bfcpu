import argparse
import serial
import time
import threading

ready = threading.Event()

def write_data(ser, data):
    while not ready.is_set():
        time.sleep(0.5)
        
    data_length = len(data).to_bytes(4, byteorder='little')
    ser.write(data_length)
    ser.write(data)
    ser.flush()

def serial_monitor(ser):
    while True:
        c = ser.read()
        if c == b"\1":
            ready.set()
        elif c == b"\0":
            break
        else:
            print(c.decode(), end="")

def main():
    parser = argparse.ArgumentParser(description='Serial Communication Program')
    parser.add_argument('port', help='Serial port name')
    parser.add_argument('baudrate', type=int, help='Baud rate for the serial connection')
    parser.add_argument('filename', help='File name to read data from')

    args = parser.parse_args()

    try:
        with serial.Serial(args.port, args.baudrate, timeout=None) as ser, open(args.filename, 'rb') as file:
            data = bytearray(file.read())

            writer = threading.Thread(target=write_data, args=(ser, data))
            monitor = threading.Thread(target=serial_monitor, args=(ser,))

            writer.start()
            monitor.start()
            
            writer.join()
            monitor.join()

    except FileNotFoundError:
        print(f"Error: File '{args.filename}' not found.")            
    except serial.SerialException as e:
        print(f"Error: {e}. Check if the specified serial port is valid.")

if __name__ == "__main__":
    main()
