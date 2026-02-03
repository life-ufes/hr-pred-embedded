from .store import ModelMetricsBuffer
from .serial import SerialProvider
from .plotter import LivePlotter
from .stream import DataStreamer
from datetime import datetime
import struct
import pandas as pd
import os


class EAModelMonitor:
    
    def __init__(
        self, 
        port: str, 
        csv_path: str, 
        output_path: str="./", 
        baud_rate: int=115200, 
        real_time: bool=False, 
        debug: bool=False,
        label: str = ""
    ):        
        self.timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        self.real_time_flag = real_time
        self.output = output_path
        self.debug = debug
        self.label = label
        self.raw_buffer = bytearray()


        self.metrics = ModelMetricsBuffer(
            timestamp=self.timestamp
        )


        self.serial = SerialProvider(
            port=port, 
            baud_rate=baud_rate, 
            on_rx_callback=self._on_serial_rx
        )


        self.data_streamer = DataStreamer( 
            csv_path=csv_path, 
            on_data_ready_callback=self._on_data_ready
        )


        self.plotter = LivePlotter(
            self.metrics, 
            output=self.output, 
            timestamp=self.timestamp,
            on_close_callback=self.stop,
            keys_to_plot=ModelMetricsBuffer.METRIC_KEYS[:3]
        )


    # ----------------------------------
    def _on_serial_rx(self, data: bytes):
        # print("Serial RX")
        self.raw_buffer.extend(data)
 
        while len(self.raw_buffer) >= 7:
            header_index = self.raw_buffer.find(b'\xDE\xAD\xBE\xEF')
            
            if header_index == -1:
                # If no header found, discard all but last 3 bytes (in case header is split)
                del self.raw_buffer[:-3]
                break
            
            # If header is not at the start, discard preceding bytes
            if header_index > 0:
                del self.raw_buffer[:header_index]
                continue

            # Metadata check
            if len(self.raw_buffer) < 7:
                break
                
            pkt_type = self.raw_buffer[4]
            len_low = self.raw_buffer[5]
            len_high = self.raw_buffer[6]
            payload_len = len_low | (len_high << 8)
            
            # Complete packet check (Meta + Payload + Checksum)
            total_packet_len = 7 + payload_len + 1
            if len(self.raw_buffer) < total_packet_len:
                break # Wait for the rest of the payload
            
            # Extract the complete packet for processing
            packet = self.raw_buffer[:total_packet_len]
            payload = packet[7:-1]
            received_checksum = packet[-1]
            
            # 4. Checksum Verification
            calc_checksum = pkt_type ^ len_low ^ len_high
            for b in payload:
                calc_checksum ^= b
            
            if calc_checksum == received_checksum:
                self._handle_valid_packet(pkt_type, payload)
            else:
                if self.debug:
                    print(f"[RX] Erro de Checksum no pacote tipo {pkt_type}")

            # Remove the processed packet from the buffer and continue looking for the next one
            del self.raw_buffer[:total_packet_len]


    # -----------------------------------------------------------
    def _handle_valid_packet(self, pkt_type: int, payload: bytes):        
        # print("Handle packet")

        if pkt_type == 0x01: # DATA (telemetry)
            try:
                hr, hr_reg, hr_gt, al, al_raw, pp_time, pp_hwm, inf_time, inf_hwm = struct.unpack('<ffffffIfI', payload)    # little endian
                sample = {
                    'hr': hr,
                    'hr_reg': hr_reg,
                    'hr_gt': hr_gt,
                    'al': al,
                    'al_raw': al_raw,
                    'pre_process_time': pp_time,
                    'pre_process_hwm': pp_hwm,
                    'inference_time': inf_time,
                    'inference_hwm': inf_hwm,
                }
                
                print(f"[RX - Data] HR: {hr:.2f} | HR_REG: {hr_reg:.2f} | HR_GT: {hr_gt:.2f}\nAL: {al:.2f} | AL_RAW: {al_raw:.2f}")
                self.metrics.add_sample(sample)
                
            except struct.error:
                print("[RX] Erro ao descompactar floats de telemetria")

        elif pkt_type == 0x02: # LOG (Strings do ESP_LOG)
            try:
                message = payload.decode('utf-8', errors='ignore').strip()
                print(f"\033[96m[ESP32 LOG] {message}\033[0m")
            except Exception as e:
                print(f"[RX] Erro ao decodificar string de log: {e}")


    # ----------------------------------
    def _on_data_ready(self, data: list):
        if self.debug:
            print(f"[TX - Ground Truth] {data[-1]} BPM\n")
            
        raw_payload = struct.pack("<77f", *data)         
        secure_packet = self._wrap_deadbeef_packet(0x01, raw_payload)        
        self.serial.send(secure_packet)


    #-----------------------------------------------------------------------
    def _wrap_deadbeef_packet(self, pkt_type: int, payload: bytes) -> bytes:
        header = b'\xDE\xAD\xBE\xEF'
        length = len(payload)
        len_low = length & 0xFF
        len_high = (length >> 8) & 0xFF
        
        # Initialize checksum with type and length bytes
        checksum = pkt_type ^ len_low ^ len_high
        for b in payload:
            checksum ^= b
            
        return header + struct.pack("B BB", pkt_type, len_low, len_high) + payload + struct.pack("B", checksum)


    # -------------
    def start(self):
        print("[START] System initialized!")
        self.serial.start()
        self.data_streamer.start()
        
        if self.real_time_flag:
            self.plotter.start()


    # -------------
    def stop(self):
        print("[STOP] User interruption!")
        
        self.serial.stop()
        self.data_streamer.stop()
        self.metrics.to_csv(self.output, label=self.label)
