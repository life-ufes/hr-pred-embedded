from .store import ModelMetricsBuffer
from .serial import SerialProvider
from .plotter import LivePlotter
from .stream import DataStreamer
from datetime import datetime
import struct
import pandas as pd
import os


class EAModelMonitor:
    
    def __init__(self, port: str, csv_path: str, output_path: str="./", baud_rate: int=115200, real_time: bool=False, debug: bool=False):
        
        self.timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        self.real_time_flag = real_time
        self.output = output_path
        self.debug = debug
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


    def _on_serial_rx(self, data: bytes):
        # sample = ModelMetricsBuffer.parse_sample(data)        
        # if self.debug:
        #     print(f"[RX - Params] {sample}\n")
        # self.metrics.add_sample(sample)

        print("Serial RX")
        # ---------------------------------------------------
        """Callback chamado sempre que chegam bytes na UART"""
        self.raw_buffer.extend(data)
        
        # Tentamos processar o buffer enquanto houver dados suficientes para um cabeçalho (4 bytes deadbeef + 3 bytes meta)
        while len(self.raw_buffer) >= 7:
            # 1. Sincronização: Procura o marcador 0xDEADBEEF
            header_index = self.raw_buffer.find(b'\xDE\xAD\xBE\xEF')
            
            if header_index == -1:
                # Se não achou o marcador, limpa o buffer deixando apenas os últimos 3 bytes 
                # (caso o 0xDE esteja no final do buffer)
                del self.raw_buffer[:-3]
                break
            
            # Se achou o marcador mas não no início, descarta o que veio antes
            if header_index > 0:
                del self.raw_buffer[:header_index]
                continue

            # 2. Verificação de Metadados (Tipo + Tamanho)
            # Se temos o header mas não temos os metadados completos (header + 3 bytes), aguardamos mais dados
            if len(self.raw_buffer) < 7:
                break
                
            pkt_type = self.raw_buffer[4]
            len_low = self.raw_buffer[5]
            len_high = self.raw_buffer[6]
            payload_len = len_low | (len_high << 8)
            
            # 3. Verificação do Pacote Completo (Meta + Payload + Checksum)
            total_packet_len = 7 + payload_len + 1
            if len(self.raw_buffer) < total_packet_len:
                break # Aguarda chegar o resto do payload
            
            # Extrai o pacote completo para processamento
            packet = self.raw_buffer[:total_packet_len]
            payload = packet[7:-1]
            received_checksum = packet[-1]
            
            # 4. Validação do Checksum XOR
            calc_checksum = pkt_type ^ len_low ^ len_high
            for b in payload:
                calc_checksum ^= b
            
            if calc_checksum == received_checksum:
                self._handle_valid_packet(pkt_type, payload)
            else:
                if self.debug:
                    print(f"[RX] Erro de Checksum no pacote tipo {pkt_type}")

            # Remove o pacote processado do buffer e continua procurando o próximo
            del self.raw_buffer[:total_packet_len]


    # -----------------------------------------------------------
    def _handle_valid_packet(self, pkt_type: int, payload: bytes):
        """Encaminha o pacote para o destino correto"""
        
        # print("Handle packet")

        if pkt_type == 0x01: # DATA (Telemetria)
            try:
                hr, hr_reg, hr_gt, al, al_raw = struct.unpack('<fffff', payload)    # 5 floats little endian
                sample = {
                    'hr': hr,
                    'hr_reg': hr_reg,
                    'hr_gt': hr_gt,
                    'al': al,
                    'al_raw': al_raw,
                }
                
                print(f"[RX - Data] HR: {hr:.2f} | HR_REG: {hr_reg:.2f} | HR_GT: {hr_gt:.2f}\nAL: {al:.2f} | AL_RAW: {al_raw:.2f}")
                self.metrics.add_sample(sample)
                
            except struct.error:
                print("[RX] Erro ao descompactar floats de telemetria")

        elif pkt_type == 0x02: # LOG (Strings do ESP_LOG)
            try:
                message = payload.decode('utf-8', errors='ignore').strip()
                # Imprime com cor para diferenciar do resto do terminal
                print(f"\033[96m[ESP32 LOG] {message}\033[0m")
            except Exception as e:
                print(f"[RX] Erro ao decodificar string de log: {e}")


    # ----------------------------------
    def _on_data_ready(self, data: list):
        if self.debug:
            print(f"[TX - Ground Truth] {data[-1]} BPM\n")
            
        payload = struct.pack("<77f", *data) 
        self.serial.send(payload)
        

    def start(self):
        print("[START] System initialized!")
        self.serial.start()
        self.data_streamer.start()
        
        if self.real_time_flag:
            self.plotter.start()


    def stop(self):
        print("[STOP] User interruption!")
        
        self.serial.stop()
        self.data_streamer.stop()
        self.metrics.to_csv(self.output)
