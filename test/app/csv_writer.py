import os
import pandas as pd
from app.plotter import fig

from app.globals import (
    w0, w1, w2, w3, w4,
    b_low, b_high,
    Al, tau, hr_reg,
    predicted_values, ground_truth_values
)

# ------------------------------------------------------------------
def save_results(timestamp: str, path: str, realtime: bool) -> None:
    arrays = [
        w0, w1, w2, w3, w4,
        b_low, b_high,
        Al, tau, hr_reg,
        predicted_values, ground_truth_values
    ]

    min_len = min(len(a) for a in arrays)

    df = pd.DataFrame({
        "w0": w0[:min_len],
        "w1": w1[:min_len],
        "w2": w2[:min_len],
        "w3": w3[:min_len],
        "w4": w4[:min_len],
        "b_low": b_low[:min_len],
        "b_high": b_high[:min_len],
        "Al": Al[:min_len],
        "tau": tau[:min_len],
        "hr_reg": hr_reg[:min_len],
        "predicted": predicted_values[:min_len],
        "ground_truth": ground_truth_values[:min_len],
    })

    print(f"Saving final plot and params to ./{path}...")
    if not os.path.exists(path):
        os.mkdir(path)

    df.to_csv(f"{path}/session_{timestamp}.csv", index=False)
    
    if realtime:
        fig.savefig(f"{path}/plot_{timestamp}.png")
