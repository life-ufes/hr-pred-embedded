from store import ModelMetricsBuffer

import threading
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation


class LivePlotter:
    def __init__(self, model_metrics: ModelMetricsBuffer, keys_to_plot=None, max_points = 800):
        self.model_metrics = model_metrics
        self.keys = keys_to_plot or list(model_metrics.buffer.keys())
        self.max_points = max_points

    def start(self):
        threading.Thread(target=self._run, daemon=True).start()

    # TODO: refactor
    def stop(self):
        pass

    def _run(self):
        plt.style.use("ggplot")
        fig, ax = plt.subplots()

        lines = {}
        for key in self.keys_to_plot:
            (line,) = ax.plot([], [], label=key)
            lines[key] = line

        ax.legend()
        ax.set_xlabel("Seconds")
        ax.set_ylabel("BPM")
        ax.set_title("Model Params")

        def update(frame):
            if self.stop_event.is_set():
                plt.close(fig)
                return

            for key in self.keys_to_plot:
                data = self.buffer.buffer[key]

                if len(data) == 0:
                    continue

                # max window size
                y = data[-self.max_points:]
                x = range(len(y))
                lines[key].set_data(x, y)

            ax.relim()
            ax.autoscale_view()

            return lines.values()
        
        ani = FuncAnimation(fig, update, interval=250)
        plt.show()
