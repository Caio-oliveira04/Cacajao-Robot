import paho.mqtt.client as mqtt
import json

BROKER_IP = "BROKER_IP"  # Exemplo: "192.168.1.100" ou "mqtt.eclipse.org"
TOPIC = "esp32/mpu6050"

def on_connect(client, userdata, flags, rc):
    print(f"Conectado ao broker MQTT com código {rc}")
    client.subscribe(TOPIC)

def on_message(client, userdata, msg):
    data = json.loads(msg.payload.decode())
    print(f"Aceleração: X={data['ax']} Y={data['ay']} Z={data['az']}")
    print(f"Giroscópio: X={data['gx']} Y={data['gy']} Z={data['gz']}\n")

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

client.connect(BROKER_IP, 1883, 60)

client.loop_forever()
