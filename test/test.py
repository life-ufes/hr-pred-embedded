from test_data.utils import get_acc_data_from_csv, acc_data_chunker
import threading
import argparse
import struct
import serial
import time
import sys

# Global
running = True


# Write data routine
def serial_write(ser: serial.Serial, data_generator) -> None:
    global running

    for data in data_generator:
        if not running:
            break

        time.sleep(1)
        # print(data[0])
        bin_packet = struct.pack("<75f", *data) # float32 | little endian
        
        try:
            ser.write(bin_packet)
        except serial.SerialException as e:
            print(f"Serial write error: {e}")
            break
    
    print("\nData transmission from file complete. Shutting down global flag.")
    running = False


# Read data routine
def serial_read(ser: serial.Serial) -> None:
    global running
    while running:
        time.sleep(0.05)
        try:
            message = ser.readline().decode("utf-8", errors="ignore").strip()
            if message:
                print(f"Python serial read: {message}")
        except serial.SerialException as e:
            print(f"Serial read error: {e}")
            break


# Starting threads
def main(args):
    global running
    CHUNK_SIZE = 25

    print("--- Data Preparation ---")
    
    df_acc = get_acc_data_from_csv(args.file)
    data_gen = acc_data_chunker(df_acc, CHUNK_SIZE) 
    
    print("Generator created. Starting serial routine...")
    print("------------------------\n\n")

    try:
        with serial.Serial(port=args.port, baudrate=args.baud_rate, timeout=1) as ser:
            write_thread = threading.Thread(target=serial_write, name="write_thread", args=(ser, data_gen))
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


# Entrypoint
if __name__ == '__main__':
    parser = argparse.ArgumentParser(prog="test.py")
    parser.add_argument('-f', '--file', type=str, required=True, help='Data file name')
    parser.add_argument('-p', '--port', type=str, required=True, help='The port of serial communication')
    parser.add_argument('-b', '--baud-rate', type=int, required=True, help='The baud rate of serial communication')
    
    args = parser.parse_args()    
    main(args)
