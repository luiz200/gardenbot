#include <AccelStepper.h>

// Definição dos pinos para o A4988
const int stepPin = 25; // Pino STEP
const int dirPin = 27;  // Pino DIR

// Configurando o motor no modo DRIVER (apenas 2 pinos: STEP e DIR)
AccelStepper stepper(AccelStepper::DRIVER, stepPin, dirPin);

void setup() {
  Serial.begin(9600);

  // Configurando a velocidade e aceleração do motor
  stepper.setMaxSpeed(500);     // Velocidade máxima em passos por segundo
  stepper.setAcceleration(300); // Aceleração em passos por segundo ao quadrado

  // Configurando os pinos como saída
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
}

void loop() {
  // Movimenta 900 passos para frente
  stepper.moveTo(9000); 
  stepper.runToPosition(); // Executa o movimento até a posição definida
  Serial.println("Movendo para frente");

  delay(5000); // Pausa de 5 segundos

  // Movimenta de volta para a posição inicial (0 passos)
  stepper.moveTo(0); 
  stepper.runToPosition();
  Serial.println("Movendo para trás");

  delay(5000); // Pausa de 5 segundos
}
