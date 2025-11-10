import random

def fake_signal_gen():
    """
    Gera 75 floats simulando acelerações em G (x, y, z) — 25 amostras de cada eixo.
    """
    data = []
    for _ in range(25):
        # Simula leituras entre -2g e +2g
        ax = random.uniform(-2.0, 2.0)
        ay = random.uniform(-2.0, 2.0)
        az = random.uniform(-2.0, 2.0)
        data.extend([ax, ay, az])
    return data