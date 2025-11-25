import time
import serial
import threading
from queue import Queue, Empty


class SerialProvider:
    
    def __init__(self, port: str, baud_rate: int, on_rx_callback = None):
        self.port = port
        self.baud_rate = baud_rate
        self.callback = on_rx_callback

        self.serial = None

        self.stop_event = threading.Event()
        self.write_queue = Queue()

        self.read_thread = None
        self.write_thread = None

    def start(self):
        print("[Serial] Starting...")
        self.serial = serial.Serial(port=self.port, baudrate=self.baud_rate, timeout=1)
        self.stop_event.clear()

        self.read_thread = threading.Thread(target=self._reader_routine, daemon=True, name="SerialReader")
        self.write_thread = threading.Thread(target=self._writer_routine, daemon=True, name="SerialWriter")

        self.read_thread.start()
        self.write_thread.start()

    def stop(self):
        print("[Serial] Stopping...")
        self.stop_event.set()

        if self.read_thread:
            print("[SerialRead] Thread exit requested.")
            self.read_thread.join()
        
        if self.write_thread:
            print("[SerialWrite] Thread exit requested.")
            self.write_thread.join()
        
        if self.serial and self.serial.is_open:
            self.serial.close()
            
        print("[Serial] Closed!")

    def send(self, data: bytes):
        self.write_queue.put(data)

    # ---------------------------------------------------------------------
    # READ THREAD
    # ---------------------------------------------------------------------
    def _reader_routine(self):
        print("[SerialRead] Thread running.")
        while not self.stop_event.is_set():
            try:
                data = self.serial.readline()
                if data and self.callback:
                    self.callback(data)
            except Exception as e:
                print(f"Reader thread error: {e}")
                break
            
            time.sleep(0.01)
    
    # ---------------------------------------------------------------------
    # WRITE THREAD
    # ---------------------------------------------------------------------
    def _writer_routine(self):
        print("[SerialWrite] Thread running.")
        while not self.stop_event.is_set():
            try:
                data = self.write_queue.get(timeout=0.1)
            except Empty:
                continue

            try:
                self.serial.write(data)
            except Exception as e:
                print(f"Writer thread error: {e}")
                break
