from store import ModelMetricsBuffer
from .serial import SerialProvider
from plotter import LivePlotter
from stream import DataStreamer


class EAModelMonitor:
    def __init__(self, port, csv_path, output_path, baud_rate=115200, real_time=False):
        self.metrics = ModelMetricsBuffer()
        self.serial = SerialProvider(port=port, baud_rate=baud_rate, callback=self._on_serial_rx)
        self.streamer = DataStreamer(serial_provider=self.serial, csv_path=csv_path)
        self.plotter = None

        self.real_time_flag = real_time
        self.output = output_path

    def _on_serial_rx(self, data):
        sample = ModelMetricsBuffer.parse_sample(data)
        if sample: 
            self.metrics.add_sample(sample)

    def start(self):
        print("[START] System initialized.")
        self.serial.start()
        self.streamer.start()
        
        if self.real_time_flag:
            self.plotter = LivePlotter(self.metrics)
            self.plotter.start()

    def stop(self):
        self.serial.stop()
        self.streamer.stop()

        if self.plotter:
            self.plotter.stop_plot()
        
        self.metrics.to_csv(self.output)
