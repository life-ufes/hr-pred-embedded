import threading
import argparse
import serial
import time
import sys

# Args parser
parser = argparse.ArgumentParser(prog="test.py")
parser.add_argument('-b', '--baud-rate', type=int, required=True, help='The baud rate of serial communication')
parser.add_argument('-p', '--port', type=str, required=True, help='The port of serial communication')
args = parser.parse_args()

running = True

# Write data routine
def serial_write(ser):
    global running
    while running:
        time.sleep(0.25)
        print("write\n")
        pass


# Read data routine
def serial_read(ser):
    global running
    while running:
        time.sleep(0.5)
        print("read\n")
        pass


# Starting threads
def main():
    global running

    try:
        with serial.Serial(port=args.port, baudrate=args.baud_rate, timeout=1) as ser:
            write_thread = threading.Thread(target=serial_write, name="write_thread", args=(ser,))
            read_thread = threading.Thread(target=serial_read, name="read_thread", args=(ser,))

            write_thread.start()
            read_thread.start()

            write_thread.join()
            read_thread.join()

            while True:
                time.sleep(0.2)

    except KeyboardInterrupt:
        print("\nExiting program...")
        running = False
        time.sleep(0.5)
        sys.exit(0)

    except serial.SerialException as err:
        print(f"Serial access error: {err}")
        sys.exit(1)

if __name__ == '__main__':
    main()
