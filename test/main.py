import time
import argparse
from lib.ea_monitor import EAModelMonitor


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('-f', '--file', type=str, required=True, help='Data file name')
    parser.add_argument("-d", "--debug", action="store_true", help="Activate detailed logs")
    parser.add_argument("-rt", "--realtime", action="store_true", help="Turns on real time chart")
    parser.add_argument('-p', '--port', type=str, required=True, help='The port of serial communication')
    parser.add_argument('-o', '--output', type=str, required=False, default="output", help='The output path')
    parser.add_argument('-b', '--baud-rate', type=int, required=True, help='The baud rate of serial communication')
    
    args = parser.parse_args()    
    
    monitor = EAModelMonitor(
        port=args.port, 
        baud_rate=args.baud_rate, 
        csv_path=args.file, 
        output_path=args.output,
        real_time=args.realtime,
        debug=args.debug
    )

    monitor.start()

    try:
        if not monitor.real_time_flag:
            while True:
                time.sleep(1)

    except KeyboardInterrupt:
        monitor.stop()
