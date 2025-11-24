import argparse
import queue
import threading
import sys
import time

from app.utils import get_acc_data_from_csv, acc_data_chunker
from app.session import DataSession, GraphSession 
from app.serial_service import SerialService

#===============================================
# HELPER
#===============================================
def run_headless_loop(session: DataSession, tx_queue, debug=False):
    """
    Manages execution loop without graphic interface.
    """
    print("Headless Mode (CTRL+C to stop)")
    
    try:
        # Step 1
        while not tx_queue.empty():
            session.process_batch()
            time.sleep(0.05) 

        print("\nEnvio concluído. Aguardando respostas restantes...")

        # FASE 2: Cooldown (Espera Inteligente)
        last_data_time = time.time()
        timeout = 2.0  # Tempo de silêncio para considerar fim
        
        while (time.time() - last_data_time) < timeout:
            processed = session.process_batch()
            
            if processed > 0:
                last_data_time = time.time() # Reseta timer
                if debug: print(f"[WAIT] +{processed} pacotes recebidos.")
            
            time.sleep(0.1)

    except KeyboardInterrupt:
        print("\nInterrupção pelo usuário.")


#===============================================
# MAIN
#===============================================
def main(args):

    # Queues
    tx_queue = queue.Queue()
    rx_queue = queue.Queue()
    stop_event = threading.Event()

    print(f"--- Initializing (Debug={'ON' if args.debug else 'OFF'}) ---")

    # 1. Data loading
    try:
        print("Loading dataset...")
        df_acc = get_acc_data_from_csv(args.file)
        chunks = acc_data_chunker(df_acc, 25)
        count = 0
        for chunk in chunks:
            tx_queue.put(chunk)
            count += 1
        print(f"Transmission queue: {count} packets ready.")
    except Exception as e:
        print(f"Error reading file: {e}")
        sys.exit(1)

    # 2. Serial starting
    serial_service = SerialService(args.port, args.baud_rate, tx_queue, rx_queue, stop_event, debug=args.debug)
    if not serial_service.start():
        sys.exit(1)

    # 3. Running
    if args.realtime:
        session = GraphSession(rx_queue, debug=args.debug)
        try:
            session.start()
        except KeyboardInterrupt:
            pass
    else:
        session = DataSession(rx_queue, debug=args.debug)
        run_headless_loop(session, tx_queue, debug=args.debug)

    # 4. Finishing
    serial_service.stop()
    print("Saving data...")
    session.save_session(args.output)
    print("Done!")


#===============================================
# ENTRYPOINT
#===============================================
if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('-p', '--port', type=str, required=True, help='Porta Serial')
    parser.add_argument('-f', '--file', type=str, required=True, help='CSV de entrada')
    parser.add_argument('-b', '--baud-rate', type=int, default=115200, help='Baud Rate')
    parser.add_argument('-o', '--output', type=str, default="output", help='Pasta de saída')
    parser.add_argument("-d", "--debug", action="store_true", help="Ativar logs detalhados")
    parser.add_argument("-rt", "--realtime", action="store_true", help="Ativar janela de gráfico")
    
    args = parser.parse_args()    
    main(args)