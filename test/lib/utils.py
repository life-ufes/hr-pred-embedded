import random
import pandas as pd

# --------------------
def fake_signal_gen():
    data = []
    for _ in range(25):
        # Simula leituras entre -2g e +2g
        ax = random.uniform(-2.0, 2.0)
        ay = random.uniform(-2.0, 2.0)
        az = random.uniform(-2.0, 2.0)
        data.extend([ax, ay, az])
    return data

# ---------------------------------------------------
def get_acc_data_from_csv(path: str) -> pd.DataFrame: 
    desired_cols = ["acc_x", "acc_y", "acc_z", "timestamp", "hr", "timestamp_hr"]
    df = pd.read_csv(path, usecols=desired_cols)
    
    # Sync timestamps
    SHIFT_PERIODS = 17
    df["timestamp_hr"] = df["timestamp_hr"].shift(SHIFT_PERIODS)
    df = df.dropna()
    return df


# -----------------------------------------------------
def acc_data_chunker(df: pd.DataFrame, chunk_size: int):

    num_rows = len(df)
    
    for start in range(0, num_rows, chunk_size):
        end = start + chunk_size
        chunk = df.iloc[start:end]

        # Only complete packets can be sent
        if len(chunk) < chunk_size:
            print(f"Skipping final partial chunk ({len(chunk)} rows). Transmission finished.")
            break
        
        # Flatten cols
        acc_data = chunk[["acc_x", "acc_y", "acc_z"]].values.flatten().tolist()      
        hr = chunk["hr"].iloc[-1]
        acc_data.append(float(hr))

        # Return list and save state
        yield acc_data
