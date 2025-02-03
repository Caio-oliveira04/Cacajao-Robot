import websocket
import json
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation

# IP do ESP32 (pegue no Serial Monitor)
ESP_IP = "ws://192.168.0.200:81/"  # Substitua pelo IP do seu ESP

# Variáveis para armazenar os dados da trajetória
x_pos, y_pos = [0], [0]  # Posição inicial
vel_x, vel_y = 0, 0  # Velocidade inicial
dt = 0.1  # Intervalo de tempo entre as leituras

def on_message(ws, message):
    global x_pos, y_pos, vel_x, vel_y
    
    try:
        data = json.loads(message)
        accX, accY = data["accX"], data["accY"]
        # Integração numérica para calcular a posição
        vel_x += accX * dt
        vel_y += accY * dt
        
        new_x = x_pos[-1] + vel_x * dt
        new_y = y_pos[-1] + vel_y * dt
        
        x_pos.append(new_x)
        y_pos.append(new_y)
        print(f"Posição: ({new_x:.2f}, {new_y:.2f})")

    except json.JSONDecodeError:
        print("Erro ao decodificar JSON")

def on_open(ws):
    print("Conectado ao ESP32!")

def plot_graph(i):
    plt.cla()
    plt.plot(x_pos, y_pos, marker='o', linestyle='-')
    plt.xlabel("X (metros)")
    plt.ylabel("Y (metros)")
    plt.title("Trajetória do Robô")
    plt.grid(True)

# Configuração do WebSocket
ws = websocket.WebSocketApp(ESP_IP, on_message=on_message, on_open=on_open)

# Rodar WebSocket em paralelo
import threading
thread = threading.Thread(target=ws.run_forever)
thread.daemon = True
thread.start()

# Criar gráfico interativo
fig = plt.figure()
ani = FuncAnimation(fig, plot_graph, interval=200)
plt.show()
