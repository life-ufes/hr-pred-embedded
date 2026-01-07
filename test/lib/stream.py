import time
import struct
import threading
import pandas as pd
from .serial import SerialProvider


class DataStreamer:
    
    # TODO: add desired cols
    def __init__(self, csv_path: str, chunk_size: int=25, interval: float=0.5, on_data_ready_callback=None):
    
        self.chunk_size = chunk_size
        self.csv_path = csv_path
        self.interval = interval

        self.stop_event = threading.Event()
        self.thread = None

        self.df = None
        self._load_and_prepare()
        self.on_data_ready = on_data_ready_callback

    def _load_and_prepare(self):        
        # desired_cols = ["acc_x", "acc_y", "acc_z", "timestamp", "hr", "timestamp_hr"]
        desired_cols = ["gyro_x", "gyro_y", "gyro_z", "timestamp", "hr", "timestamp_hr"]

        df = pd.read_csv(self.csv_path, usecols=desired_cols)

        # sync timestamp
        SHIFT_PERIODS = 17
        df["timestamp_hr"] = df["timestamp_hr"].shift(SHIFT_PERIODS)
        df = df.dropna()

        self.df = df.reset_index(drop=True)


    def _chunk_generator(self):        
        num_rows = len(self.df)

        for start in range(0, num_rows, self.chunk_size):
            end = start + self.chunk_size
            chunk = self.df.iloc[start:end]

            if len(chunk) < self.chunk_size:
                print(f"[DataStreamer] Skipping final partial chunk ({len(chunk)} rows).")
                break

            # acc_data = chunk[["acc_x", "acc_y", "acc_z"]].values.flatten().tolist()
            acc_data = chunk[["gyro_x", "gyro_y", "gyro_z"]].values.flatten().tolist()


            hr_value = float(chunk["hr"].iloc[-1])
            acc_data.append(hr_value)

            yield acc_data


    def start(self):
        print("[DataStreamer] Starting...")
        self.stop_event.clear()

        self.thread = threading.Thread(target=self._run, daemon=True, name="DataStreamer")
        self.thread.start()


    def stop(self):
        print("[DataStreamer] Stopping...")
        self.stop_event.set()

        if self.thread:
            self.thread.join()
            print("[DataStreamer] Stopped.")

    # ---------------------------------------------------------
    # THREAD
    # ---------------------------------------------------------
    def _run(self):
        print("[DataStreamer] Thread running. Beginning CSV streaming...")

        for packet in self._chunk_generator():
            if self.stop_event.is_set():
                print("[DataStreamer] Thread exit requested.")
                break

            self.on_data_ready(packet)
            time.sleep(self.interval)

        print("[DataStreamer] Finished sending CSV.")
