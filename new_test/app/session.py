import os
import pandas as pd
from datetime import datetime
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation


# --- CLASSE BASE (Funciona sem gráfico) ---
class DataSession:
    def __init__(self, rx_queue, debug=False):
        self.rx_queue = rx_queue
        self.debug = debug
        self.data = {
            "w0": [], "w1": [], "w2": [], "w3": [], "w4": [],
            "b_low": [], "b_high": [], 
            "Al": [], "tau": [], 
            "hr_reg": [], "predicted": []
        }

    def process_batch(self):
        """Esvazia a fila e salva na memória"""
        count = 0
        while not self.rx_queue.empty():
            try:
                msg = self.rx_queue.get_nowait()
                parts = [float(p) for p in msg.split(",")]
                
                if len(parts) >= 11:
                    self.data["w0"].append(parts[0])
                    self.data["w1"].append(parts[1])
                    self.data["w2"].append(parts[2])
                    self.data["w3"].append(parts[3])
                    self.data["w4"].append(parts[4])
                    self.data["b_high"].append(parts[5])
                    self.data["b_low"].append(parts[6])
                    self.data["Al"].append(parts[7])
                    self.data["tau"].append(parts[8])
                    self.data["hr_reg"].append(parts[9])
                    self.data["predicted"].append(parts[10])
                    
                    if self.debug:
                        print(f"[DATA] Parsed: HR_Pred={parts[10]} | Al={parts[7]}")
                    count += 1
            except ValueError:
                if self.debug: print(f"[DATA] Ignored garbage: {msg}")
                pass
        return count

    def save_session(self, output_dir):
        """Salva o CSV"""
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        if not os.path.exists(output_dir):
            os.makedirs(output_dir)
            
        min_len = min(len(v) for v in self.data.values())
        if min_len == 0:
            print("Nenhum dado recebido para salvar.")
            return

        clean_data = {k: v[:min_len] for k, v in self.data.items()}
        df = pd.DataFrame(clean_data)
        
        csv_path = f"{output_dir}/session_{timestamp}.csv"
        df.to_csv(csv_path, index=False)
        print(f"CSV salvo em: {csv_path}")
        return csv_path, timestamp


# --- CLASSE COM GRÁFICO (Herda da Base) ---
class GraphSession(DataSession):
    def __init__(self, rx_queue, debug=False):
        super().__init__(rx_queue, debug) # Inicializa a base
        
        # Setup do Matplotlib
        self.fig, self.ax = plt.subplots()
        self.ax.set_title("Realtime Monitor")
        self.ax.set_xlabel("Seconds")
        self.ax.set_ylabel("BPM")
        
        self.lines = {}
        for key in self.data.keys():
            line, = self.ax.plot([], [], label=key)
            self.lines[key] = line
        self.ax.legend(loc="upper left", fontsize='small')

    def _update_plot(self, frame):
        self.process_batch() # Chama lógica da classe pai
        
        x_len = len(self.data["predicted"])
        x_axis = range(x_len)
        
        for key, line in self.lines.items():
            vals = self.data[key]
            line.set_data(x_axis[:len(vals)], vals)

        self.ax.relim()
        self.ax.autoscale_view()
        return self.lines.values()

    def start(self):
        ani = FuncAnimation(self.fig, self._update_plot, interval=200, cache_frame_data=False)
        plt.show()

    def save_session(self, output_dir):
        # Chama o save do pai para o CSV
        path, timestamp = super().save_session(output_dir)
        # Adiciona salvamento da imagem
        if path:
            img_path = f"{output_dir}/plot_{timestamp}.png"
            self.fig.savefig(img_path)
            print(f"Plot salvo em: {img_path}")