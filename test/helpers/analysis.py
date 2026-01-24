import pandas as pd

PATH = "data/HOOSEOK_FILTERED/Subject_8_whole_session.csv"
PATH_2 = "output/session_20260106_203813.csv"

df = pd.read_csv(PATH)
target_df = df[["AL_raw", "AL_raw_norm", "AL_raw_gyro", "AL_raw_gyro_norm"]]
# target_df = df["Al"]

print(target_df.describe())
