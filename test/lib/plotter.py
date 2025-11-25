from .store import ModelMetricsBuffer

import os
from datetime import datetime
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation


class LivePlotter:
    
    def __init__(self, model_metrics: ModelMetricsBuffer, output: str, keys_to_plot=None, max_points = 800, callback=None):
        self.keys_to_plot = keys_to_plot or list(model_metrics.buffer.keys())
        self.model_metrics = model_metrics
        self.close_callback = callback
        self.max_points = max_points
        self.output = output

        self.fig = None
        self.ax = None
        self.lines = {}
        self.ani = None

    def start(self):
        print("[LivePlotter] Starting...")

        plt.style.use("ggplot")
        self.fig, self.ax = plt.subplots()

        for key in self.keys_to_plot:
            (line,) = self.ax.plot([], [], label=key)
            self.lines[key] = line

        self.ax.legend()
        self.ax.set_xlabel("Samples")
        self.ax.set_ylabel("Value")
        self.ax.set_title("Model Params")

        self.fig.canvas.mpl_connect("close_event", self._on_close)

        # Create animation
        self.ani = FuncAnimation(
            self.fig,
            self._update,
            interval=200,      
            blit=False,
            cache_frame_data=False
        )

        try:
            plt.show()
        except KeyboardInterrupt:
            print("[LivePlotter] Interrupted, saving figure...")
            self._on_close()            

    def _update(self, frame):
        for key in self.keys_to_plot:
            data = self.model_metrics.buffer[key]
            if len(data) == 0:
                continue

            y = data[-self.max_points:]
            x = range(len(y))

            self.lines[key].set_data(x, y)

        self.ax.relim()
        self.ax.autoscale_view()

        return list(self.lines.values())

    def stop(self):
        print("[LivePlotter] Stopping...")  
        plt.close(self.fig)

    # ---------------------------------------------- #
    #     SALVAR FIGURA AO FECHAR JANELA OU CTRL-C   #
    # ---------------------------------------------- #
    def _on_close(self, event):
        print("[LivePlotter] Window closed — saving figure...")

        self._save_figure()
        plt.close(self.fig)

        if self.close_callback:
            self.close_callback()        

    def _save_figure(self):
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        filename = f"{self.output}/plot_{timestamp}.png"

        os.makedirs(self.output, exist_ok=True)
        self.fig.savefig(filename, dpi=200)

        print(f"[LivePlotter] Figure saved at: {filename}")
