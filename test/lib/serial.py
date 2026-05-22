import time
import logging
import serial
import threading
from queue import Queue, Empty


logger = logging.getLogger(__name__)


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
        logger.info("[Serial] Starting...")
        self.serial = serial.Serial(port=self.port, baudrate=self.baud_rate, timeout=1)
        self.stop_event.clear()

        self.read_thread = threading.Thread(target=self._reader_routine, daemon=True, name="SerialReader")
        self.write_thread = threading.Thread(target=self._writer_routine, daemon=True, name="SerialWriter")

        self.read_thread.start()
        self.write_thread.start()

    def stop(self):
        logger.info("[Serial] Stopping...")
        self.stop_event.set()

        if self.read_thread:
            logger.debug("[SerialRead] Thread exit requested.")
            self.read_thread.join()
        
        if self.write_thread:
            logger.debug("[SerialWrite] Thread exit requested.")
            self.write_thread.join()
        
        if self.serial and self.serial.is_open:
            self.serial.close()
            
        logger.info("[Serial] Closed!")

    def send(self, data: bytes):
        self.write_queue.put(data)
        # print(f"[Serial] Queued {len(data)} bytes for transmission.")

    # ---------------------------------------------------------------------
    # READ THREAD
    # ---------------------------------------------------------------------
    def _reader_routine(self):
        logger.debug("[SerialRead] Thread running.")

        while not self.stop_event.is_set():
            try:
                # Checks for available data
                waiting = self.serial.in_waiting
                if waiting > 0:
                    # Reads exactly what is available now
                    data = self.serial.read(waiting)
                    if data and self.callback:
                        self.callback(data)
                else:
                    # Small pause to avoid CPU hogging if no data is available
                    time.sleep(0.01)

            except Exception as e:
                logger.debug("Reader thread error: %s", e)
                break
    
    # ---------------------------------------------------------------------
    # WRITE THREAD
    # ---------------------------------------------------------------------
    def _writer_routine(self):
        logger.debug("[SerialWrite] Thread running.")
        while not self.stop_event.is_set():
            try:
                data = self.write_queue.get(timeout=0.1)
            except Empty:
                continue

            try:
                self.serial.write(data)
                logger.debug("[Serial] Sent %s bytes.", len(data))
            except Exception as e:
                logger.debug("Writer thread error: %s", e)
                break
