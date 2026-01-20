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
            keys_to_plot=ModelMetricsBuffer.METRIC_KEYS[-3:]
        )


    def _on_serial_rx(self, data: bytes):

        # sample_str = data.decode(encoding="utf-8").strip()
        # sample_str = sample_str[:-1]
        # sample_list = [float(metric) for metric in sample_str.split(",")]

        # # acc_x = sample_list[0:25]
        # # acc_y = sample_list[25:50]
        # # acc_z = sample_list[50:75]

        # # 2. Criar um DataFrame temporário para este lote
        # # df_batch = pd.DataFrame({
        # #     'acc_x': acc_x,
        # #     'acc_y': acc_y,
        # #     'acc_z': acc_z
        # # })
        # df_batch = pd.DataFrame({
        #     "agg": sample_list
        # })

        # # 3. Caminho do arquivo
        # csv_file = 'dados_micro_validacao.csv'

        # # 4. Salvar no arquivo
        # # Se o arquivo não existir, escreve com cabeçalho. 
        # # Se existir, apenas anexa (append) sem repetir o cabeçalho.
        # file_exists = os.path.isfile(csv_file)
        # df_batch.to_csv(csv_file, mode='a', index=False, header=not file_exists)

        # # Opcional: print para monitorar no terminal
        # print(f"Lote salvo. Total de linhas: {len(df_batch)}")

        sample = ModelMetricsBuffer.parse_sample(data)        
        if self.debug:
            print(f"[RX - Params] {sample}\n")
        self.metrics.add_sample(sample)


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
