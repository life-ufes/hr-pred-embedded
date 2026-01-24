from datetime import datetime
import pandas as pd
import os

class ModelMetricsBuffer:

    METRIC_KEYS = [
        "hr",
        "hr_reg",
        "hr_gt",
        "al",
        "al_raw",
    ]


    def __init__(self, timestamp: str):
        self.buffer = { key: [] for key in self.METRIC_KEYS }
        self.timestamp = timestamp


    def add_sample(self, sample: dict):
        for key, value in sample.items():
            if key in self.buffer:
                self.buffer[key].append(value)
            else:
                print(f"[WARN] Ignoring unknown metric: {key}")


    def to_csv(self, path):
        os.makedirs(path, exist_ok=True)
        df = pd.DataFrame(self.buffer)

        csv = f"{path}/session_{self.timestamp}.csv"
        df.to_csv(csv, index=False)
        
        print(f"[OK] saved CSV: {csv}")


    def reset(self):
        for key in self.buffer:
            self.buffer[key] = []


    @classmethod
    def parse_sample(cls, sample: bytes) -> dict:                
        sample_str = sample.decode(encoding="utf-8").strip()
        sample_list = [float(metric) for metric in sample_str.split(",")]
        
        if len(sample_list) != len(cls.METRIC_KEYS):
            raise ValueError(f"Invalid sample length: expected {len(cls.METRIC_KEYS)}, got {len(sample_list)}.")
        
        parsed_sample = { key: value for key, value in zip(cls.METRIC_KEYS, sample_list) }
        return parsed_sample
