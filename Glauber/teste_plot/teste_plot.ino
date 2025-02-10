#include <Wire.h>
#include <MPU6050_tockn.h>
#include <WiFi.h>
#include <PubSubClient.h>

// Configurações do Wi-Fi
const char* ssid = "Redmi 14C";
const char* password = "12345678";

// Configurações do MQTT
const char* mqtt_broker = "192.168.127.239";
const int mqtt_port = 1883;
const char* mqtt_topic = "MPU6050";

WiFiClient espClient;
PubSubClient client(espClient);
MPU6050 mpu6050(Wire);

// Variáveis para velocidade e posição
float velocity_x = 0.0, velocity_y = 0.0;
float position_x = 0.0, position_y = 0.0;

// Offsets da aceleração
float offset_ax = 0.0, offset_ay = 0.0;

// Tempo da última leitura
unsigned long last_time = 0;

// Filtro passa-baixa
float alpha = 0.1; // Ajuste conforme necessário
float filtered_ax = 0.0, filtered_ay = 0.0;

// Limiares e limites
const float ACCEL_THRESHOLD = 0.06; // Limiar para considerar o robô parado
const float MAX_VELOCITY = 20.0;    // Limite de velocidade
const float MAX_POSITION = 100.0;  // Limite de posição

// Intervalos de tempo
const unsigned long loop_interval = 10;   // Intervalo de leitura do sensor (10 ms)
const unsigned long mqtt_interval = 1000; // Intervalo de publicação MQTT (1 segundo)
unsigned long last_mqtt_time = 0;

void setup() {
    Serial.begin(115200);
    Wire.begin();
    mpu6050.begin();
    mpu6050.calcGyroOffsets(true);
    calibrateAccelerometer();
    last_time = millis();

    // Conectar ao Wi-Fi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Conectando ao WiFi...");
    }
    Serial.println("Conectado ao WiFi");

    // Configurar MQTT
    client.setServer(mqtt_broker, mqtt_port);
    while (!client.connected()) {
        Serial.println("Conectando ao broker MQTT...");
        if (client.connect("ESP32Client")) {
            Serial.println("Conectado ao broker MQTT");
            client.subscribe(mqtt_topic);
        } else {
            Serial.print("Falha na conexão, rc=");
            Serial.print(client.state());
            Serial.println(" Tentando novamente em 5 segundos");
            delay(5000);
        }
    }
}

void loop() {
    unsigned long current_time = millis();

    // Verificar conexões Wi-Fi e MQTT
    check_connection();

    // Executar leitura e processamento em intervalos fixos
    if (current_time - last_time >= loop_interval) {
        float dt = (current_time - last_time) / 1000.0; // Converter para segundos
        last_time = current_time;

        // Atualizar leituras do MPU6050
        mpu6050.update();
        float ax = mpu6050.getAccX() - offset_ax;
        float ay = mpu6050.getAccY() - offset_ay;

        // Aplicar filtro passa-baixa
        filtered_ax = alpha * ax + (1 - alpha) * filtered_ax;
        filtered_ay = alpha * ay + (1 - alpha) * filtered_ay;

        // Integração para velocidade
        if (abs(filtered_ax) < ACCEL_THRESHOLD) velocity_x = 0;
        else velocity_x += filtered_ax * dt;

        if (abs(filtered_ay) < ACCEL_THRESHOLD) velocity_y = 0;
        else velocity_y += filtered_ay * dt;

        // Limitar velocidade
        velocity_x = constrain(velocity_x, -MAX_VELOCITY, MAX_VELOCITY);
        velocity_y = constrain(velocity_y, -MAX_VELOCITY, MAX_VELOCITY);

        // Integração para posição
        position_x += velocity_x * dt;
        position_y += velocity_y * dt;

        // Limitar posição
        position_x = constrain(position_x, -MAX_POSITION, MAX_POSITION);
        position_y = constrain(position_y, -MAX_POSITION, MAX_POSITION);

        // Exibir dados no Serial Monitor
        Serial.print("Posição X: ");
        Serial.print(position_x);
        Serial.print(" m, Y: ");
        Serial.print(position_y);
        Serial.println(" m");
    }

    // Publicar dados via MQTT em intervalos fixos
    if (current_time - last_mqtt_time >= mqtt_interval) {
        last_mqtt_time = current_time;

        char payload[50];
        snprintf(payload, sizeof(payload), "{\"x\": %.2f, \"y\": %.2f}", position_x, position_y);

        if (client.publish(mqtt_topic, payload)) {
            Serial.println("📡 Dados enviados com sucesso!");
        } else {
            Serial.println("❌ Erro ao enviar dados via MQTT!");
        }
    }

    client.loop();
}

// Função para reconectar ao Wi-Fi e MQTT automaticamente
void check_connection() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Wi-Fi desconectado! Reconectando...");
        WiFi.disconnect();
        WiFi.reconnect();
        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 10) {
            delay(1000);
            Serial.print(".");
            attempts++;
        }
        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("\nWi-Fi reconectado!");
        } else {
            Serial.println("\nFalha ao reconectar Wi-Fi!");
        }
    }

    if (!client.connected()) {
        Serial.println("MQTT desconectado! Tentando reconectar...");
        int attempts = 0;
        while (!client.connected() && attempts < 5) {
            if (client.connect("ESP32Client")) {
                Serial.println("Reconectado ao MQTT!");
                client.subscribe(mqtt_topic);
            } else {
                Serial.print(".");
                delay(5000);
                attempts++;
            }
        }
        if (!client.connected()) {
            Serial.println("\nFalha ao reconectar MQTT!");
        }
    }
}

// Função para calibrar o acelerômetro
void calibrateAccelerometer() {
    Serial.println("Calibrando acelerômetro... Mantenha o sensor parado.");
    delay(2000);
    float sum_ax = 0.0, sum_ay = 0.0;
    int num_samples = 100;

    for (int i = 0; i < num_samples; i++) {
        mpu6050.update();
        sum_ax += mpu6050.getAccX();
        sum_ay += mpu6050.getAccY();
        delay(10);
    }

    offset_ax = sum_ax / num_samples;
    offset_ay = sum_ay / num_samples;
    Serial.println("Calibração concluída.");
}