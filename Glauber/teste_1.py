import matplotlib.pyplot as plt
import paho.mqtt.client as mqtt
import json

# Configurações do MQTT
mqtt_broker = "localhost"  # Endereço do broker MQTT
mqtt_port = 1883
mqtt_topic = "MPU"

# Variáveis para armazenar a trajetória
positions_x = []
positions_y = []

# Função chamada quando a conexão ao MQTT for estabelecida
def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Conectado ao MQTT com sucesso!")
        client.subscribe(mqtt_topic)
    else:
        print(f"Falha na conexão com código de retorno {rc}")

# Função chamada quando uma mensagem é recebida no MQTT
def on_message(client, userdata, msg):
    try:
        payload = json.loads(msg.payload.decode())
        px = payload.get("Px", 0)
        py = payload.get("Py", 0)

        positions_x.append(px)
        positions_y.append(py)

        print(f"Posição recebida: Px = {px}, Py = {py}")

        # Atualiza o gráfico
        plt.clf()
        plt.plot(positions_x, positions_y, marker='o', color='b', markersize=3, label="Trajetória")
        plt.xlabel("Posição X (m)")
        plt.ylabel("Posição Y (m)")
        plt.title("Trajetória do MPU6050")
        plt.legend()
        plt.xlim(min(positions_x, default=0) - 1, max(positions_x, default=1) + 1)
        plt.ylim(min(positions_y, default=0) - 1, max(positions_y, default=1) + 1)
        plt.draw()
        plt.pause(0.1)
    except Exception as e:
        print(f"Erro ao processar a mensagem: {e}")

# Configura o cliente MQTT
client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

try:
    client.connect(mqtt_broker, mqtt_port, 60)
    
    # Configuração do gráfico
    plt.ion()
    plt.figure(figsize=(10, 6))
    plt.xlabel("Posição X (m)")
    plt.ylabel("Posição Y (m)")
    plt.title("Trajetória do MPU6050")
    plt.show()

    # Loop MQTT que roda de forma síncrona
    client.loop_forever()  # Aguarda e processa as mensagens de forma contínua

except KeyboardInterrupt:
    print("Interrompido pelo usuário.")
finally:
    client.disconnect()
    plt.ioff()
    plt.show()
    