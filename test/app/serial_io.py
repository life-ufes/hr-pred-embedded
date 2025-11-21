import time
import serial
import struct
import threading
import app.globals as app_globals

from app.globals import (
    w0, w1, w2, w3, w4,
    b_low, b_high, Al, tau,
    hr_reg, predicted_values, ground_truth_values
)


# Write data routine ----------------------------------------
def serial_write(ser: serial.Serial, data_generator) -> None:
    for data in data_generator:
        if not app_globals.running or not ser.is_open:
            break

        time.sleep(0.5)

        ground_truth = data[-1]
        ground_truth_values.append(ground_truth)
        print("HR-GT:", ground_truth)

        try:
            bin_packet = struct.pack("<76f", *data) # 76 float32 (304B) | little endian
            ser.write(bin_packet)

        except serial.SerialException as e:
            print(f"Serial write loop ended!")
            break
    
    print("\nData transmission from file complete. Shutting down the threads!")
    app_globals.running = False


# Read data routine ------------------------
def serial_read(ser: serial.Serial) -> None:
    while app_globals.running:
        time.sleep(0.05)
        
        if not ser.is_open:
            break

        try:
            message = ser.readline().decode("utf-8", errors="ignore").strip()
            if message:
                params = [float(p) for p in message.split(",")]
                w0.append(params[0])
                w1.append(params[1])
                w2.append(params[2])
                w3.append(params[3])
                w4.append(params[4])
                b_high.append(params[5])
                b_low.append(params[6])
                Al.append(params[7])
                tau.append(params[8])
                hr_reg.append(params[9])
                predicted_values.append(params[10])

                print(f"Python serial read: {message.split(",")}")
        
        except (serial.SerialException, OSError, TypeError)as e:
            print(f"Serial read loop ended!")
            break

# ----------------------------------------------------
def start_serial_threads(ser: serial.Serial, data_gen):
	# Init threads
	write_thread = threading.Thread(target=serial_write, name="write_thread", args=(ser, data_gen), daemon=True)
	read_thread = threading.Thread(target=serial_read, name="read_thread", args=(ser,), daemon=True)
	write_thread.start()
	read_thread.start()

	return read_thread, write_thread


# old read
# try:
        #     data = ser.read(4)  # lê 4 bytes (timeout de 1s vem do Serial())
        #     if len(data) == 4:
        #         hr = struct.unpack("<f", data)[0]   # int32 little endian
        #         print("READ HR:", hr) 
        #         predicted_values.append(hr)
        #         print(f"Python serial read: {hr}")

        # except serial.SerialException as e:
        #     print(f"Serial read error: {e}")
        #     break