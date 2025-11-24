import pandas as pd


class ModelMetricsBuffer:
    
    def __init__(self):
        self.buffer = {
            "w0": [],
            "w1": [],
            "w2": [],
            "w3": [],
            "w4": [],
            "b_low": [],
            "b_high": [],
            "Al": [],
            "tau": [],
            "hr_reg": [],
            "predicted_values": [],
            # "ground_truth_values": []
        }

    def add_sample(self, sample: dict):
        for key, value in sample:
            if key in self.buffer:
                self.buffer[key].append(value)
            else:
                print(f"[WARN] Ignoring unknown metric: {key}")

    def to_csv(self, path):
        # TODO: mkdirs and timestamp
        df = pd.DataFrame(self.buffer)
        df.to_csv(path, index=False)
        print(f"[OK] saved CSV: {path}")

    def reset(self):
        for key in self.buffer:
            self.buffer[key] = []

    @classmethod
    def parse_sample(cls, sample):
        pass
