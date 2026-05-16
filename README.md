# HR Predictor

### Overview
**HR Predictor** (`embedded-hr-estimator`) is an embedded systems project built on top of the ESP-IDF framework. It focuses on estimating and predicting Heart Rate through Edge AI (Machine Learning on the edge). Designed to run on microcontrollers such as the **ESP32-S3**, the system processes raw sensor data (from an Accelerometer or Gyroscope) and uses on-device predictive models — currently two variants:

- **Exponential Approximation Model (EAM)** – a lightweight, resource-constrained model using exponential functions with online parameter adaptation.
- **Dynamic Exponential Model (DEM)** – a dual-tau exponential model offering richer dynamics.

The firmware architecture is highly modular and asynchronous, leveraging FreeRTOS to manage dedicated concurrent tasks for each pipeline stage:

| Task | Description |
|---|---|
| `task_rx` | Receives incoming data packets from the host over UART |
| `task_preprocess` | Applies FIR high-pass filtering, computes accelerometer magnitude, and calculates Activity Level (AL) |
| `task_inference_eam` / `task_inference_dem` | Executes the selected ML model to predict Heart Rate |
| `task_tx` | Transmits telemetry packets (predictions + metrics) back to the host |

### Research Paper

This project was developed as part of a **research project** and the resulting paper was published at **SEMISH** (*Seminário Integrado de Software e Hardware*), powered by **SBC** (*Sociedade Brasileira de Computação*). The paper covers the dataset, system architecture, model design, and experimental results.

> 📄 **Paper:** *Link coming soon*

---

### Requirements

It's highly recommended to install the **ESP-IDF VSCode extension**. Follow the [official documentation](https://docs.espressif.com/projects/vscode-esp-idf-extension/en/latest/installation.html) instructions. It's also recommended to install the `Microsoft C/C++ Extension Pack`.

> **Note:** The firmware was developed exclusively for the **ESP32-S3**. It will not work on other devices without modifications.

---

### Firmware Configuration (Kconfig)

Before building, you can configure the firmware via `menuconfig` (`> ESP-IDF: SDK Configuration Editor`). The following options are available under **"ML Model Configuration"**:

#### ML Model Selection
| Option | Description |
|---|---|
| `EXPONENTIAL_APPROXIMATION_MODEL` *(default)* | Builds with the EAM model (`task_inference_eam`). |
| `DYNAMIC_EXPONENTIAL_MODEL` | Builds with the DEM model (`task_inference_dem`). |

#### Sensor Source
| Option | Description |
|---|---|
| `SENSOR_ACC` *(default)* | Uses Accelerometer axes (X, Y, Z) for activity level computation. |
| `SENSOR_GYRO` | Uses Gyroscope axes for activity level computation. |

These options are declared in `main/Kconfig.projbuild` and take effect at compile time via `#ifdef` guards in `main/main.c`.

---

### Firmware Usage

1. Clone the repository and open VSCode within the workspace directory.
2. Open the command palette and select:
   ```
   > ESP-IDF: Add VS Code Configuration Folder
   ```
   This resolves the compiler's path and enables syntax highlighting.

3. Connect your device and select the port:
   ```
   > ESP-IDF: Select Port to Use
   ```

4. Set the device target (ESP32-S3):
   ```
   > ESP-IDF: Set Espressif Device Target
   ```
   OpenOCD board configuration should use `builtin USB-JTAG`.

5. Build, flash and monitor:
   ```
   > ESP-IDF: Build Your Project
   > ESP-IDF: Flash Your Project    (use UART as flash method)
   ```

---

### Communication Protocol

The firmware and the host Python monitor communicate over **UART** using a custom framed binary protocol. Every packet follows the structure below:

```
[ 0xDE 0xAD 0xBE 0xEF ] [ TYPE (1B) ] [ LEN_LOW (1B) ] [ LEN_HIGH (1B) ] [ PAYLOAD (N B) ] [ CHECKSUM (1B) ]
```

| Field | Size | Description |
|---|---|---|
| Header | 4 bytes | Magic sync word: `0xDEADBEEF` |
| Type | 1 byte | `0x01` = Data/Telemetry, `0x02` = ESP32 Log string |
| Length | 2 bytes | Payload length in bytes (little-endian) |
| Payload | N bytes | Binary payload (see below) |
| Checksum | 1 byte | XOR of Type, Len_Low, Len_High, and all Payload bytes |

#### Packet Type `0x01` — Sensor Data (Host → ESP32)

Sent by the Python monitor to inject a window of accelerometer samples plus ground-truth HR. Payload is **77 floats** (little-endian):

```
[ acc_x[0..24] (25 floats) ] [ acc_y[0..24] (25 floats) ] [ acc_z[0..24] (25 floats) ] [ hr_gt (1 float) ] [ train_flag (1 float) ]
```

#### Packet Type `0x01` — Telemetry (ESP32 → Host)

Sent by the ESP32 after each inference cycle. Payload is **9 fields** packed as `<ffffffIfI` (little-endian):

| Field | C Type | Description |
|---|---|---|
| `hr` | float | Predicted Heart Rate (BPM) |
| `hr_reg` | float | Regression component of the predicted HR |
| `hr_gt` | float | Ground Truth HR received from the host |
| `al` | float | Normalized Activity Level |
| `al_raw` | float | Raw (unnormalized) Activity Level |
| `pre_process_time` | float | Preprocessing stage execution time (µs) |
| `pre_process_hwm` | uint32 | Preprocessing task stack High Water Mark (bytes) |
| `inference_time` | float | Inference stage execution time (µs) |
| `inference_hwm` | uint32 | Inference task stack High Water Mark (bytes) |

#### Packet Type `0x02` — Log (ESP32 → Host)

UTF-8 string logged via `ESP_LOG`. Displayed in the console with cyan color formatting.

---

## Testing and Monitoring

The `/test` directory contains a complete **host-side Python validation environment**. Its purpose is to:
1. **Inject** pre-recorded sensor data from a CSV file into the ESP32 via UART.
2. **Receive** and decode telemetry packets from the ESP32.
3. **Store** all received metrics in a CSV output file.
4. **Visualize** predictions in real time (optional).

### Setup

Navigate to the `/test` directory and install the dependencies using `pip`:

```bash
cd test
pip install -r requirements.txt
```

The required packages are:

| Package | Version | Purpose |
|---|---|---|
| `pyserial` | 3.5 | UART communication with the ESP32 |
| `pandas` | 2.3.3 | CSV loading and data manipulation |
| `matplotlib` | 3.10.8 | Real-time plotting and figure saving |
| `numpy` | 2.4.0 | Numerical operations |

---

### Running the Monitor (`test/main.py`)

`main.py` is the entry point of the monitoring session. It instantiates and starts the `EAModelMonitor`, which orchestrates all sub-systems (serial, streaming, metrics, plotting).

#### Arguments

| Argument | Short | Required | Default | Description |
|---|---|---|---|---|
| `--file` | `-f` | ✅ Yes | — | Path to the input CSV dataset file |
| `--port` | `-p` | ✅ Yes | — | Serial port (e.g. `/dev/ttyUSB0` or `COM3`) |
| `--baud-rate` | `-b` | ✅ Yes | — | Serial baud rate (must match firmware: `115200`) |
| `--output` | `-o` | No | `output` | Directory where output CSV and plots are saved |
| `--label` | `-l` | No | `""` | Prefix label for output files (e.g. subject ID) |
| `--debug` | `-d` | No | `False` | Enable detailed debug logs to stdout |
| `--realtime` | `-rt` | No | `False` | Open a live matplotlib chart during the session |

#### Basic Usage

```bash
# Minimal — required arguments only
python test/main.py \
  --file data/subject_01.csv \
  --port /dev/ttyUSB0 \
  --baud-rate 115200
```

#### With Label and Custom Output Directory

```bash
# Session labeled "subject_08_acc" saved to results/
python test/main.py \
  --file data/Subject_8.csv \
  --port /dev/ttyUSB0 \
  --baud-rate 115200 \
  --output results/ \
  --label subject_08_acc
```

The output files will be named:
- `results/subject_08_acc_<YYYYMMDD_HHMMSS>.csv` — metrics CSV
- `results/plot_<YYYYMMDD_HHMMSS>.png` — plot image (if `--realtime` was active)

#### With Debug Logs

```bash
# Shows ground-truth values being sent and checksum errors (if any)
python test/main.py \
  --file data/subject_01.csv \
  --port /dev/ttyUSB0 \
  --baud-rate 115200 \
  --debug
```

With `--debug`, each data packet sent to the ESP32 prints:
```
[TX - Ground Truth] 72.5 BPM
```
And checksum errors on received packets are also reported.

#### With Real-Time Chart

```bash
# Opens a live matplotlib window showing HR, HR_REG, HR_GT during the session
python test/main.py \
  --file data/subject_01.csv \
  --port /dev/ttyUSB0 \
  --baud-rate 115200 \
  --realtime
```

> **Note:** When `--realtime` is active, the script is driven by the matplotlib event loop. The session ends automatically when the plot window is closed, which also triggers saving the metrics CSV and figure to disk.  
> Without `--realtime`, the script runs in background threads and you must press **Ctrl+C** to stop it and save the outputs.

#### Help

```bash
python test/main.py --help
```

---

### Input CSV Format

The dataset CSV must contain at minimum the following columns:

| Column | Type | Description |
|---|---|---|
| `acc_x` | float | Accelerometer X-axis sample |
| `acc_y` | float | Accelerometer Y-axis sample |
| `acc_z` | float | Accelerometer Z-axis sample |
| `timestamp` | float | Sample timestamp |
| `hr` | float | Ground-truth Heart Rate (BPM) |
| `timestamp_hr` | float | Timestamp of the HR measurement |
| `train` | int | `1` if this window belongs to the training phase, `0` for test |

The streamer reads data in windows of **25 samples** (`WINDOW_LEN`). Each window yields one packet sent to the ESP32. Partial windows at the end of the file are discarded.

> **HR Timestamp Synchronization:** The streamer applies a shift of **17 periods** to `timestamp_hr` to compensate for the latency between sensor samples and the corresponding HR measurement. This is handled automatically in `DataStreamer._load_and_prepare()`.

---

### Architecture: `lib/` Modules

The monitoring system is composed of four cooperating classes:

```
main.py
  └── EAModelMonitor (ea_monitor.py)
        ├── SerialProvider  (serial.py)     — UART read/write threads
        ├── DataStreamer    (stream.py)      — CSV → packet generator thread
        ├── ModelMetricsBuffer (store.py)   — in-memory metrics accumulator
        └── LivePlotter    (plotter.py)     — real-time matplotlib chart
```

#### `SerialProvider` (`lib/serial.py`)

Manages the serial connection using two dedicated threads:

- **Reader thread** (`SerialReader`): Polls `serial.in_waiting` at 10ms intervals and delivers all available bytes to the `on_rx_callback`. The callback is `EAModelMonitor._on_serial_rx()`.
- **Writer thread** (`SerialWriter`): Drains an internal `Queue` and writes bytes to the serial port. Packets are enqueued via `send(data: bytes)`.

```python
serial = SerialProvider(port="/dev/ttyUSB0", baud_rate=115200, on_rx_callback=my_callback)
serial.start()
serial.send(packet_bytes)
serial.stop()
```

#### `DataStreamer` (`lib/stream.py`)

Reads the input CSV, aligns timestamps, and streams windows of sensor data at a controlled rate.

- **Chunk size:** 25 samples per window (`chunk_size=25`).
- **Interval:** 100ms between packets (`interval=0.1`), approximately matching the real-time acquisition rate.
- Each packet contains: `acc_x[25] + acc_y[25] + acc_z[25] + [hr_gt, train_flag]` (77 floats total).
- Calls `on_data_ready_callback(packet: list)` for each generated window.

```python
streamer = DataStreamer(csv_path="data.csv", on_data_ready_callback=my_handler)
streamer.start()
streamer.stop()
```

#### `ModelMetricsBuffer` (`lib/store.py`)

Thread-safe (append-only) in-memory buffer for all received telemetry fields.

**Tracked metrics:**

| Key | Description |
|---|---|
| `hr` | Predicted HR from the model (BPM) |
| `hr_reg` | Regression component of HR |
| `hr_gt` | Ground Truth HR (BPM) |
| `al` | Normalized Activity Level |
| `al_raw` | Raw Activity Level |
| `pre_process_time` | Preprocessing execution time (µs) |
| `pre_process_hwm` | Preprocessing task stack HWM (bytes) |
| `inference_time` | Inference execution time (µs) |
| `inference_hwm` | Inference task stack HWM (bytes) |

The `to_csv(path, label)` method exports all buffered data to a timestamped CSV file:

```python
metrics = ModelMetricsBuffer(timestamp="20260106_203813")
metrics.add_sample({...})
metrics.to_csv("output/", label="subject_01")
# Saves: output/subject_01_20260106_203813.csv
```

The `reset()` method clears all buffers for reuse. `parse_sample(bytes)` is a class method to decode a comma-separated byte string (legacy format, not used by the current protocol).

#### `LivePlotter` (`lib/plotter.py`)

Displays a live `matplotlib` chart updating every **200ms**. By default, it plots the first 3 metrics: `hr`, `hr_reg`, and `hr_gt` — the most relevant for visual model validation.

- Shows up to `max_points=800` recent samples on the x-axis (sliding window).
- On window close, saves the figure as `plot_<timestamp>.png` at `dpi=200` and triggers the `on_close_callback` (which calls `monitor.stop()`).

```python
plotter = LivePlotter(
    model_metrics=metrics,
    output="output/",
    timestamp="20260106_203813",
    keys_to_plot=["hr", "hr_reg", "hr_gt"],
    max_points=800,
    on_close_callback=monitor.stop
)
plotter.start()   # Blocks — driven by plt.show()
```

#### `EAModelMonitor` (`lib/ea_monitor.py`)

The top-level orchestrator. Implements the packet framing/deframing logic and ties all sub-systems together.

**RX Packet Parsing (`_on_serial_rx`):**  
Uses a streaming state machine over a `bytearray` raw buffer:
1. Searches for `0xDEADBEEF` header.
2. Validates packet length from metadata bytes.
3. Verifies XOR checksum.
4. Dispatches to `_handle_valid_packet`.

**TX Packet Building (`_wrap_deadbeef_packet`):**  
Wraps a raw payload into the binary protocol format, computing the XOR checksum automatically.

**Lifecycle:**
```python
monitor = EAModelMonitor(
    port="/dev/ttyUSB0",
    baud_rate=115200,
    csv_path="data.csv",
    output_path="output/",
    real_time=True,
    debug=True,
    label="subject_01"
)
monitor.start()   # Starts serial, streamer, and optionally plotter
monitor.stop()    # Stops all threads and saves CSV output
```

---

### Typical End-to-End Workflow

```
1. Flash firmware onto ESP32-S3 (with desired model selected via Kconfig)
2. Connect ESP32 via USB-UART
3. Run monitoring session:
       python test/main.py -f data/Subject_8.csv -p /dev/ttyUSB0 -b 115200 -o output/ -l subject_8 --realtime
4. Press Ctrl+C (or close the plot window) to end the session
       → output/subject_8_<timestamp>.csv is saved automatically
```
