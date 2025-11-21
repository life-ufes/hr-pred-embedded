import sys
import time
import serial
import argparse
from datetime import datetime

from lib.utils import get_acc_data_from_csv, acc_data_chunker
from app.serial_io import start_serial_threads
from app.plotter import start_realtime_plot
from app.csv_writer import save_results
import app.globals as app_globals


def main(args):
    CHUNK_SIZE = 25 # 25Hz

    print("Preparing data...")
    df_acc = get_acc_data_from_csv(args.file)
    data_gen = acc_data_chunker(df_acc, CHUNK_SIZE)
    
    try:
        with serial.Serial(port=args.port, baudrate=args.baud_rate, timeout=1) as ser:
            
            print("Starting serial routine...")
            read_thread, write_thread = start_serial_threads(ser, data_gen)
            
            if args.realtime:
                start_realtime_plot()
            else:
                while app_globals.running:
                    time.sleep(0.5)

    except KeyboardInterrupt:
        print("\nCTRL-C -> Interrupting program...")
        app_globals.running = False

    except serial.SerialException as err:
        print(f"Serial access error: {err}.")
        sys.exit(1)

    except Exception as err:
        print(f"Something went wrong: {err}.")
        sys.exit(1)

    finally:
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        try:
            save_results(timestamp, args.output, args.realtime)
        except Exception as err:
            print(f"Could not save results: {err}")
        print("Done!")


# ---------------------------------------------------------------------------
# ENTRYPOINT
# ---------------------------------------------------------------------------
if __name__ == '__main__':
    parser = argparse.ArgumentParser(prog="test.py")
    parser.add_argument('-f', '--file', type=str, required=True, help='Data file name')
    parser.add_argument("-rt", "--realtime", action="store_true", help="Turns on real time chart")
    parser.add_argument('-p', '--port', type=str, required=True, help='The port of serial communication')
    parser.add_argument('-o', '--output', type=str, required=False, default="output", help='The output path')
    parser.add_argument('-b', '--baud-rate', type=int, required=True, help='The baud rate of serial communication')
    
    args = parser.parse_args()    
    main(args)
