import paho.mqtt.client as mqtt
import json

# Configuração do broker MQTT
BROKER = "localhost"  # IP do seu PC (onde o Mosquitto está rodando)
PORT = 1883  # Porta padrão do MQTT
TOPIC = "MPU"  # Tópico onde a ESP32 publica os dados

# Callback quando uma mensagem é recebida
def on_message(client, userdata, msg):
    try:
        payload = msg.payload.decode("utf-8")  # Decodifica a mensagem recebida
        data = json.loads(payload)  # Converte JSON para dicionário

        # Extrai os valores de aceleração X e Y
        ax = data.get("Ax", "N/A")
        ay = data.get("Ay", "N/A")

        print(f"Aceleração X: {ax} m/s² | Aceleração Y: {ay} m/s²")

    except Exception as e:
        print("Erro ao processar mensagem:", e)

# Configuração do cliente MQTT
client = mqtt.Client()
client.on_message = on_message

# Conecta ao broker MQTT
client.connect(BROKER, PORT, 60)

# Inscreve-se no tópico
client.subscribe(TOPIC)

# Mantém o cliente ouvindo mensagens
print(f"Aguardando mensagens no tópico '{TOPIC}'...")
client.loop_forever()
