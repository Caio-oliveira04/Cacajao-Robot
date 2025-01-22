#include <Arduino.h>
#include <ESP32Servo.h>

#define trigPin 2
#define echoPin 15

#define motorDireitaFrente 26
#define motorDireitaTras 27
#define motorEsquerdaFrente 5
#define motorEsquerdaTras 18

#define max_distance 400

Servo servo;
int ServoPos = 90;
bool goesForward = false;
int distance;

void setup() {
    servo.attach(14);
    pinMode(motorDireitaFrente, OUTPUT);
    pinMode(motorDireitaTras, OUTPUT);
    pinMode(motorEsquerdaFrente, OUTPUT);
    pinMode(motorEsquerdaTras, OUTPUT);
    pinMode(echoPin, INPUT);
    pinMode(trigPin, OUTPUT);

    Serial.begin(9600);

    servo.write(90);  // Centralizar o servo no início
    delay(150);
}

void loop() {
    distance = readPing();
    if (distance <= 20) {
        pararMovimento();
        delay(100);
        moverParaTras();
        delay(2000);  // Corrigido para voltar por 2 segundos
        pararMovimento();
        delay(100);
        virarParaLadoMaisLivre();
    } else {
        moverParaFrente();
    }
}

long readPing() {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    long duration = pulseIn(echoPin, HIGH, 30000); // Timeout de 30ms para evitar loops travados
    long cm = duration / 58;
    if (cm == 0 || cm > max_distance) {
        cm = max_distance;
    }
    return cm;
}

void virarParaLadoMaisLivre() {
    int somaEsquerda = 0;
    int somaDireita = 0;
    int distance_temp;
    int countDireita = 0, countEsquerda = 0;

    for (ServoPos = 90; ServoPos >= 0; ServoPos -= 5) {
        servo.write(ServoPos);
        delay(50);
        distance_temp = readPing();
        if (distance_temp < max_distance) {
            somaDireita += distance_temp;
            countDireita++;
        }
    }

    for (ServoPos = 90; ServoPos <= 180; ServoPos += 5) {
        servo.write(ServoPos);
        delay(50);
        distance_temp = readPing();
        if (distance_temp < max_distance) {
            somaEsquerda += distance_temp;
            countEsquerda++;
        }
    }

    servo.write(90); // Centralizar o servo

    float mediaEsquerda = (countEsquerda > 0) ? (somaEsquerda / countEsquerda) : 0;
    float mediaDireita = (countDireita > 0) ? (somaDireita / countDireita) : 0;

    if (mediaEsquerda > mediaDireita) {
        virarParaEsquerda();
    } else {
        virarParaDireita();
    }
}

void pararMovimento() {
    digitalWrite(motorDireitaFrente, LOW);
    digitalWrite(motorDireitaTras, LOW);
    digitalWrite(motorEsquerdaFrente, LOW);
    digitalWrite(motorEsquerdaTras, LOW);
}

void virarParaDireita() {
    digitalWrite(motorDireitaFrente, LOW);
    digitalWrite(motorDireitaTras, LOW);
    digitalWrite(motorEsquerdaFrente, HIGH);
    digitalWrite(motorEsquerdaTras, LOW);
    delay(500); // Ajuste de tempo para virar
    pararMovimento();
}

void virarParaEsquerda() {
    digitalWrite(motorDireitaFrente, HIGH);
    digitalWrite(motorDireitaTras, LOW);
    digitalWrite(motorEsquerdaFrente, LOW);
    digitalWrite(motorEsquerdaTras, LOW);
    delay(500); // Ajuste de tempo para virar
    pararMovimento();
}

void moverParaFrente() {
    if (!goesForward) {
        goesForward = true;
        digitalWrite(motorDireitaFrente, HIGH);
        digitalWrite(motorDireitaTras, LOW);
        digitalWrite(motorEsquerdaFrente, HIGH);
        digitalWrite(motorEsquerdaTras, LOW);
    }
}

void moverParaTras() {
    goesForward = false;
    digitalWrite(motorDireitaFrente, LOW);
    digitalWrite(motorDireitaTras, HIGH);
    digitalWrite(motorEsquerdaFrente, LOW);
    digitalWrite(motorEsquerdaTras, HIGH);
}
