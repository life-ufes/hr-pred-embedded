import time
import struct
import logging
import threading
import pandas as pd
from .serial import SerialProvider


logger = logging.getLogger(__name__)


class DataStreamer:
    
    # TODO: add desired cols
    def __init__(self, csv_path: str, chunk_size: int=25, interval: float=0.1, on_data_ready_callback=None):
    
        self.chunk_size = chunk_size
        self.csv_path = csv_path
        self.interval = interval

        self.stop_event = threading.Event()
        self.thread = None

        self.df = None
        self._load_and_prepare()
        self.on_data_ready = on_data_ready_callback


    def _load_and_prepare(self):        
        desired_cols = ["acc_x", "acc_y", "acc_z", "gyro_x", "gyro_y", "gyro_z", "timestamp", "hr", "timestamp_hr", "train"]

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
                logger.debug("[DataStreamer] Skipping final partial chunk (%s rows).", len(chunk))
                break
            
            acc_x = chunk["acc_x"].values.tolist()
            acc_y = chunk["acc_y"].values.tolist()
            acc_z = chunk["acc_z"].values.tolist()

            hr_value = chunk["hr"].iloc[-1]
            train_flag = chunk["train"].iloc[-1]

            packet_data = acc_x + acc_y + acc_z + [hr_value, train_flag]
            yield packet_data


    def start(self):
        logger.info("[DataStreamer] Starting...")
        self.stop_event.clear()

        self.thread = threading.Thread(target=self._run, daemon=True, name="DataStreamer")
        self.thread.start()


    def stop(self):
        logger.info("[DataStreamer] Stopping...")
        self.stop_event.set()

        if self.thread:
            self.thread.join()
            logger.info("[DataStreamer] Stopped.")

    # ---------------------------------------------------------
    # THREAD
    # ---------------------------------------------------------
    def _run(self):
        logger.info("[DataStreamer] Thread running. Beginning CSV streaming...")

        for packet in self._chunk_generator():
            if self.stop_event.is_set():
                logger.debug("[DataStreamer] Thread exit requested.")
                break

            self.on_data_ready(packet)
            time.sleep(self.interval)

        logger.info("[DataStreamer] Finished sending CSV.")
