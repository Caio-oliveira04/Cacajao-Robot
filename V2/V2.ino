#include <Arduino.h>
#include <ESP32Servo.h>
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

Servo servo;
int servoPos = 90;

void setup() {
    servo.attach(13);

    pinMode(MOTOR_DIREITA_FRENTE, OUTPUT);
    pinMode(MOTOR_DIREITA_TRAS, OUTPUT);
    pinMode(MOTOR_ESQUERDA_FRENTE, OUTPUT);
    pinMode(MOTOR_ESQUERDA_TRAS, OUTPUT);

    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
    pinMode(SENSOR_IR_DIREITA, INPUT);
    pinMode(SENSOR_IR_ESQUERDA, INPUT);

    Serial.begin(9600);

    servo.write(90);
    delay(150);
}

void loop() {
    int distancia = medirDistancia();
    bool sensorDireita = digitalRead(SENSOR_IR_DIREITA);
    bool sensorEsquerda = digitalRead(SENSOR_IR_ESQUERDA);

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
