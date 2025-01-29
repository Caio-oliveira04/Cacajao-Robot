#include <Arduino.h>
#include <ESP32Servo.h>
#include <MPU6050.h>
#include <Wire.h>
#include <WiFi.h>
#include <PubSubClient.h>

// Definição dos pinos
#define TRIG_PIN 2
#define ECHO_PIN 15
#define MOTOR_DIREITA_FRENTE 26
#define MOTOR_DIREITA_TRAS 27
#define MOTOR_ESQUERDA_FRENTE 32
#define MOTOR_ESQUERDA_TRAS 33
#define SENSOR_IR_DIREITA 4
#define SENSOR_IR_ESQUERDA 12

#define MAX_DISTANCE 400
#define DISTANCIA_SEGURANCA 20

// Pinos I2C
#define SDA_PIN 22
#define SCL_PIN 21

// Configurações WiFi e MQTT
const char* ssid = "Edge";       // Substitua pelo seu WiFi
const char* password = "1234567890"; // Substitua pela sua senha
const char* mqtt_server = "BROKER_IP"; // Substitua pelo IP do seu broker MQTT

WiFiClient espClient;
PubSubClient client(espClient);

Servo servo;
MPU6050 mpu;
int servoPos = 90;

void setup() {
    Serial.begin(9600);

    // Conectar ao WiFi
    WiFi.begin(ssid, password);
    Serial.print("Conectando ao WiFi...");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println(" Conectado!");

    // Conectar ao broker MQTT
    client.setServer(mqtt_server, 1883);

    // Inicializa servo, sensores e motores
    servo.attach(14);
    pinMode(MOTOR_DIREITA_FRENTE, OUTPUT);
    pinMode(MOTOR_DIREITA_TRAS, OUTPUT);
    pinMode(MOTOR_ESQUERDA_FRENTE, OUTPUT);
    pinMode(MOTOR_ESQUERDA_TRAS, OUTPUT);
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
    pinMode(SENSOR_IR_DIREITA, INPUT);
    pinMode(SENSOR_IR_ESQUERDA, INPUT);

    // Inicializa I2C e MPU6050
    Wire.begin(SDA_PIN, SCL_PIN);
    mpu.initialize();

    if (!mpu.testConnection()) {
        Serial.println("Falha na conexão com MPU6050!");
        while (1);
    }
    Serial.println("MPU6050 conectado!");

    servo.write(90);
    delay(150);
}

void loop() {
    if (!client.connected()) {
        reconnect();
    }
    client.loop();

    int distancia = medirDistancia();
    bool sensorDireita = digitalRead(SENSOR_IR_DIREITA);
    bool sensorEsquerda = digitalRead(SENSOR_IR_ESQUERDA);

    // Captura dados da MPU6050
    int ax, ay, az, gx, gy, gz;
    mpu.getAcceleration(&ax, &ay, &az);
    mpu.getRotation(&gx, &gy, &gz);

    // Exibe coordenadas no Serial Monitor
    Serial.print("Aceleração: X=");
    Serial.print(ax);
    Serial.print(" Y=");
    Serial.print(ay);
    Serial.print(" Z=");
    Serial.println(az);

    Serial.print("Giroscópio: X=");
    Serial.print(gx);
    Serial.print(" Y=");
    Serial.print(gy);
    Serial.print(" Z=");
    Serial.println(gz);

    // Publica os dados no MQTT
    String payload = "{";
    payload += "\"ax\":" + String(ax) + ",";
    payload += "\"ay\":" + String(ay) + ",";
    payload += "\"az\":" + String(az) + ",";
    payload += "\"gx\":" + String(gx) + ",";
    payload += "\"gy\":" + String(gy) + ",";
    payload += "\"gz\":" + String(gz);
    payload += "}";

    client.publish("esp32/mpu6050", payload.c_str());
    Serial.println("Publicado no MQTT: " + payload);

    // Controle de navegação do robô
    if (distancia <= DISTANCIA_SEGURANCA) {
        pararMovimento();
        moverParaTras(300);
        virarParaLadoMaisLivre();
    } else if (sensorDireita == LOW) {
        virarParaEsquerda(200);
    } else if (sensorEsquerda == LOW) {
        virarParaDireita(200);
    } else {
        moverParaFrente();
    }

    delay(1000);  // Aguarda 1 segundo antes do próximo ciclo
}

long medirDistancia() {
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    long duracao = pulseIn(ECHO_PIN, HIGH, 30000);
    if (duracao == 0) return MAX_DISTANCE;

    long cm = duracao / 58;
    return (cm > MAX_DISTANCE) ? MAX_DISTANCE : cm;
}

void virarParaLadoMaisLivre() {
    int somaEsquerda = 0, somaDireita = 0, countEsquerda = 0, countDireita = 0;

    for (servoPos = 90; servoPos >= 0; servoPos -= 10) {
        servo.write(servoPos);
        delay(50);
        int dist = medirDistancia();
        if (dist < MAX_DISTANCE) {
            somaDireita += dist;
            countDireita++;
        }
    }

    for (servoPos = 90; servoPos <= 180; servoPos += 10) {
        servo.write(servoPos);
        delay(50);
        int dist = medirDistancia();
        if (dist < MAX_DISTANCE) {
            somaEsquerda += dist;
            countEsquerda++;
        }
    }

    servo.write(90);

    float mediaDireita = (countDireita > 0) ? somaDireita / countDireita : 0;
    float mediaEsquerda = (countEsquerda > 0) ? somaEsquerda / countEsquerda : 0;

    if (mediaEsquerda > mediaDireita) {
        virarParaEsquerda(500);
    } else {
        virarParaDireita(500);
    }
}

void pararMovimento() {
    digitalWrite(MOTOR_DIREITA_FRENTE, LOW);
    digitalWrite(MOTOR_DIREITA_TRAS, LOW);
    digitalWrite(MOTOR_ESQUERDA_FRENTE, LOW);
    digitalWrite(MOTOR_ESQUERDA_TRAS, LOW);
}

void virarParaDireita(int tempo) {
    digitalWrite(MOTOR_DIREITA_FRENTE, LOW);
    digitalWrite(MOTOR_DIREITA_TRAS, LOW);
    digitalWrite(MOTOR_ESQUERDA_FRENTE, HIGH);
    digitalWrite(MOTOR_ESQUERDA_TRAS, LOW);
    delay(tempo);
    pararMovimento();
}

void virarParaEsquerda(int tempo) {
    digitalWrite(MOTOR_DIREITA_FRENTE, HIGH);
    digitalWrite(MOTOR_DIREITA_TRAS, LOW);
    digitalWrite(MOTOR_ESQUERDA_FRENTE, LOW);
    digitalWrite(MOTOR_ESQUERDA_TRAS, LOW);
    delay(tempo);
    pararMovimento();
}

void moverParaFrente() {
    digitalWrite(MOTOR_DIREITA_FRENTE, HIGH);
    digitalWrite(MOTOR_DIREITA_TRAS, LOW);
    digitalWrite(MOTOR_ESQUERDA_FRENTE, HIGH);
    digitalWrite(MOTOR_ESQUERDA_TRAS, LOW);
}

void moverParaTras(int tempo) {
    digitalWrite(MOTOR_DIREITA_FRENTE, LOW);
    digitalWrite(MOTOR_DIREITA_TRAS, HIGH);
    digitalWrite(MOTOR_ESQUERDA_FRENTE, LOW);
    digitalWrite(MOTOR_ESQUERDA_TRAS, HIGH);
    delay(tempo);
    pararMovimento();
}

void reconnect() {
    while (!client.connected()) {
        Serial.print("Conectando ao MQTT...");
        if (client.connect("ESP32_Client")) {
            Serial.println(" Conectado!");
        } else {
            Serial.println(" Falha! Tentando novamente em 5s...");
            delay(5000);
        }
    }
}
