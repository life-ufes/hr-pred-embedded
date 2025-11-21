from test_data.utils import get_acc_data_from_csv, acc_data_chunker
from matplotlib.animation import FuncAnimation
import matplotlib.pyplot as plt
from datetime import datetime
import pandas as pd
import threading
import argparse
import struct
import serial
import time
import sys

# Global
running = True

# Chart data
w0 = []
w1 = []
w2 = []
w3 = []
w4 = []
b_low = []
b_high = []
Al = []
tau = []
hr_reg = []
predicted_values = []
ground_truth_values = []

# Plot setup (global figure)
fig, ax = plt.subplots()
ax.set_title("Heart Rate: Prediction vs Ground Truth")
ax.set_xlabel("Seconds")
ax.set_ylabel("BPM")
line_w0, = ax.plot([], [], label="W0")
line_w1, = ax.plot([], [], label="W1")
line_w2, = ax.plot([], [], label="W2")
line_w3, = ax.plot([], [], label="W3")
line_w4, = ax.plot([], [], label="W4")
line_b_high, = ax.plot([], [], label="B_HIGH")
line_b_low, = ax.plot([], [], label="B_LOW")
line_al, = ax.plot([], [], label="AL")
line_tau, = ax.plot([], [], label="TAU")
line_hr_reg, = ax.plot([], [], label="HR_reg")
line_pred, = ax.plot([], [], label="HR_next")
line_gt, = ax.plot([], [], label="HR_gt")
ax.legend()


# CSV builder -----------------------
def save_csv_with_pandas(timestamp):

    arrays = [
        w0, w1, w2, w3, w4,
        b_low, b_high,
        Al, tau, hr_reg,
        predicted_values, ground_truth_values
    ]

    min_len = min(len(a) for a in arrays)

    df = pd.DataFrame({
        "w0": w0[:min_len],
        "w1": w1[:min_len],
        "w2": w2[:min_len],
        "w3": w3[:min_len],
        "w4": w4[:min_len],
        "b_low": b_low[:min_len],
        "b_high": b_high[:min_len],
        "Al": Al[:min_len],
        "tau": tau[:min_len],
        "hr_reg": hr_reg[:min_len],
        "predicted": predicted_values[:min_len],
        "ground_truth": ground_truth_values[:min_len],
    })

    df.to_csv(f"session_{timestamp}.csv", index=False)
    print(f"Saved to session_{timestamp}.csv with {min_len} rows.")


# Write data routine ----------------------------------------
def serial_write(ser: serial.Serial, data_generator) -> None:
    global running

    for data in data_generator:
        if not running:
            break

        time.sleep(0.5)

        ground_truth = data[-1]
        ground_truth_values.append(ground_truth)
        print("GT:", ground_truth)

        bin_packet = struct.pack("<76f", *data) # float32 | little endian
        
        try:
            ser.write(bin_packet)
        except serial.SerialException as e:
            print(f"Serial write error: {e}")
            break
    
    print("\nData transmission from file complete. Shutting down global flag.")
    running = False


# Read data routine ------------------------
def serial_read(ser: serial.Serial) -> None:
    global running
    while running:
        time.sleep(0.05)
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
        
        except serial.SerialException as e:
            print(f"Serial read error: {e}")
            break
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


# ---------------------------------------------------------------------------
# REAL-TIME GRAPH UPDATE
# ---------------------------------------------------------------------------
def update_plot(frame):
    line_w0.set_data(range(len(w0)), w0)
    line_w1.set_data(range(len(w1)), w1)
    line_w2.set_data(range(len(w2)), w2)
    line_w3.set_data(range(len(w3)), w3)
    line_w4.set_data(range(len(w4)), w4)
    line_b_high.set_data(range(len(b_high)), b_high)
    line_b_low.set_data(range(len(b_low)), b_low)
    line_al.set_data(range(len(Al)), Al)
    line_tau.set_data(range(len(tau)), tau)
    line_hr_reg.set_data(range(len(hr_reg)), hr_reg)
    line_pred.set_data(range(len(predicted_values)), predicted_values)
    line_gt.set_data(range(len(ground_truth_values)), ground_truth_values)

    ax.relim()
    ax.autoscale_view()

    return line_pred, line_gt, line_w0, line_w1, line_w2, line_w3, line_w4, line_b_high, line_b_low, line_al, line_tau, line_hr_reg


# ---------------------------------------------------------------------------
# MAIN
# ---------------------------------------------------------------------------
def main(args):
    global running
    CHUNK_SIZE = 25

    print("--- Data Preparation ---")
    
    df_acc = get_acc_data_from_csv(args.file)
    data_gen = acc_data_chunker(df_acc, CHUNK_SIZE) 
    
    print("Generator created. Starting serial routine...")
    print("------------------------\n\n")

    ani = FuncAnimation(fig, update_plot, interval=200)

    try:
        with serial.Serial(port=args.port, baudrate=args.baud_rate, timeout=1) as ser:
            write_thread = threading.Thread(target=serial_write, name="write_thread", args=(ser, data_gen))
            read_thread = threading.Thread(target=serial_read, name="read_thread", args=(ser,))

            write_thread.start()
            read_thread.start()
            
            plt.show()

            timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
            print("Saving final plot to final_plot.png...")
            fig.savefig(f"final_plot_{timestamp}.png")
            save_csv_with_pandas(timestamp)
            running = False

            write_thread.join()
            read_thread.join()
            
            # Use it if it has no animation
            # while True:
            #     time.sleep(0.2)

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
