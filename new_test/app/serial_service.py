import threading
import serial
import struct
import time
import queue

class SerialService:
    def __init__(self, port, baud_rate, tx_queue, rx_queue, stop_event, debug=False):
        self.port = port
        self.baud = baud_rate
        self.tx_queue = tx_queue
        self.rx_queue = rx_queue
        self.stop_event = stop_event
        self.debug = debug  # Nova flag
        self.ser = None
        self._threads = []

    def log(self, msg):
        """Helper para imprimir só se debug=True"""
        if self.debug:
            print(f"[SERIAL] {msg}")

    def start(self):
        try:
            self.ser = serial.Serial(self.port, self.baud, timeout=1)
            print(f"[Serial] Conectado em {self.port}")
        except serial.SerialException as e:
            print(f"[Serial] Erro de conexão: {e}")
            return False

        t_read = threading.Thread(target=self._reader_loop, daemon=True, name="SerialReader")
        t_write = threading.Thread(target=self._writer_loop, daemon=True, name="SerialWriter")
        
        self._threads = [t_read, t_write]
        for t in self._threads: t.start()
        
        return True

    def stop(self):
        print("[Serial] Parando serviço...")
        self.stop_event.set()
        if self.ser and self.ser.is_open:
            self.ser.close()

    def _reader_loop(self):
        while not self.stop_event.is_set() and self.ser.is_open:
            try:
                line = self.ser.readline().decode("utf-8", errors="ignore").strip()
                if line:
                    self.log(f"RX Raw: {line}") # Debug
                    self.rx_queue.put(line)
            except Exception as e:
                print(f"[Serial Read Error] {e}")
                break

    def _writer_loop(self):
        while not self.stop_event.is_set() and self.ser.is_open:
            try:
                chunk = self.tx_queue.get(timeout=0.1)
                time.sleep(0.5) 
                
                bin_packet = struct.pack("<76f", *chunk)
                self.ser.write(bin_packet)
                
                self.log(f"TX Packet sent (HR_GT: {chunk[-1]})") # Debug

            except queue.Empty:
                continue
            except Exception as e:
                print(f"[Serial Write Error] {e}")
                break