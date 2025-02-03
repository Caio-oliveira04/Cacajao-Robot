#include <Arduino.h>
#include <Wire.h>
#include <MPU6050_tockn.h>
#include <WiFi.h>
#include <WebSocketsServer.h>

// Configurações WiFi
const char* ssid = "CASA";
const char* password = "14062021";

// Definição dos pinos I2C para ESP32
#define MPU_SDA 21
#define MPU_SCL 22

MPU6050 mpu(Wire);
WebSocketsServer webSocket(81); // Servidor WebSocket na porta 81

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
    // Evento WebSocket (opcional, pode ser usado para comandos)
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    pinMode(MPU_SDA, INPUT_PULLUP);
    pinMode(MPU_SCL, INPUT_PULLUP);
    
    Wire.begin(MPU_SDA, MPU_SCL);
    Serial.println("\n=== Inicializando o MPU6050 ===");
    mpu.begin();
    
    Serial.println("✅ MPU6050 conectado com sucesso!");
    Serial.println("⌛ Calibrando sensor... Mantenha o sensor imóvel.");
    delay(1000);
    mpu.calcGyroOffsets(true);
    Serial.println("✅ Calibração concluída!");

    // Conectar ao WiFi
    WiFi.begin(ssid, password);
    Serial.print("Conectando ao WiFi...");
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }
    Serial.println("\n✅ WiFi Conectado!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());

    // Inicializa WebSocket
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);
}

void loop() {
    mpu.update();
    
    float accX = mpu.getAccX();
    float accY = mpu.getAccY();
    float accZ = mpu.getAccZ();
    float gyroX = mpu.getGyroX();
    float gyroY = mpu.getGyroY();
    float gyroZ = mpu.getGyroZ();

    // Monta uma string JSON para enviar os dados
    String data = "{";
    data += "\"accX\":" + String(accX) + ",";
    data += "\"accY\":" + String(accY) + ",";
    data += "\"accZ\":" + String(accZ) + ",";
    data += "\"gyroX\":" + String(gyroX) + ",";
    data += "\"gyroY\":" + String(gyroY) + ",";
    data += "\"gyroZ\":" + String(gyroZ);
    data += "}";

    printf("A(x,y,z) = (%.2f, %.2f, %.2f) \n", accX, accY, accZ);
    
    // Envia os dados para todos os clientes conectados
    webSocket.broadcastTXT(data);

    webSocket.loop(); // Mantém o WebSocket ativo
    delay(100); // Pequeno delay para não sobrecarregar a rede
}
