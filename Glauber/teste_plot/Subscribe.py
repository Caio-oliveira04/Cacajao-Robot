import paho.mqtt.client as mqtt
import matplotlib.pyplot as plt
import json

# Inicializa listas para armazenar os valores de x e y
x_values = []
y_values = []

# Configurações do MQTT
mqtt_broker = "localhost"  # Altere para o IP do broker, se necessário
mqtt_port = 1883
mqtt_topic = "MPU6050"

# Configura o gráfico
plt.ion()  # Ativa o modo interativo
fig, ax = plt.subplots()
scatter = ax.scatter([], [], color='red')
ax.set_xlabel('X')
ax.set_ylabel('Y')
ax.set_title('Plotagem Contínua de Pontos (X, Y)')

# Função para atualizar os limites do gráfico dinamicamente
def update_axes():
    if x_values and y_values:  # Só atualiza se houver dados
        ax.set_xlim(min(x_values) - 1, max(x_values) + 1)
        ax.set_ylim(min(y_values) - 1, max(y_values) + 1)
        ax.relim()
        ax.autoscale_view()

# Função para atualizar o gráfico
def update_plot():
    scatter.set_offsets(list(zip(x_values, y_values)))
    update_axes()
    plt.draw()

# Função chamada ao receber uma mensagem no tópico MQTT
def on_message(client, userdata, msg):
    try:
        data = json.loads(msg.payload.decode())
        x = float(data.get("x"))
        y = float(data.get("y"))
        print(f"Novo valor recebido - X: {x}, Y: {y}")
        x_values.append(x)
        y_values.append(y)
        update_plot()
    except Exception as e:
        print(f"Erro ao processar a mensagem MQTT: {e}")

# Função chamada ao conectar ao broker MQTT
def on_connect(client, userdata, flags, rc, properties=None):
    if rc == 0:
        print(f"✅ Conectado ao broker MQTT (Código {rc})")
        print(f"📡 Inscrito no tópico: {mqtt_topic}")
        client.subscribe(mqtt_topic)
    else:
        print(f"❌ Falha na conexão. Código: {rc}")

# Configuração do cliente MQTT
client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

# Conecta ao broker MQTT
print("🔄 Conectando ao broker MQTT...")
client.connect(mqtt_broker, mqtt_port, 60)

# Inicia o loop MQTT em uma thread
client.loop_start()
print("📡 Aguardando dados do MQTT...")

# Atualiza o gráfico em um loop não bloqueante
try:
    while True:
        plt.pause(0.1)  # Atualiza o gráfico sem bloquear
except KeyboardInterrupt:
    print("Encerrando o script.")
    client.loop_stop()
