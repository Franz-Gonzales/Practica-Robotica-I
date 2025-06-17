#include <Stepper.h>

const int pasosPorRevolucion = 200; // Cambia según tu motor
const int velocidadMotor = 100; // RPM

const int IN1 = 8;
const int IN2 = 9;
const int IN3 = 10;
const int IN4 = 11;

// Configura los pines del driver L298N
Stepper motor(pasosPorRevolucion, IN1, IN2, IN3, IN4);
bool girando = false; // Variable para controlar el giro infinito
int direccion = 0; // 1 para derecha, -1 para izquierda

void setup() {
  Serial.begin(9600); // Inicia la comunicación serial
  motor.setSpeed(velocidadMotor); // Configura la velocidad del motor
  Serial.println("Envíe 'DERECHA', 'IZQUIERDA', 'DETENER', o un ángulo para mover el motor.");
}

void loop() {
  if (Serial.available() > 0) {
    String comando = Serial.readStringUntil('\n'); // Lee el comando del puerto serial

    if (comando == "r") {
      girando = true;
      direccion = 1; // Girar hacia la derecha
      Serial.println("Giro infinito hacia la derecha iniciado.");
    } else if (comando == "l") {
      girando = true;
      direccion = -1; // Girar hacia la izquierda
      Serial.println("Giro infinito hacia la izquierda iniciado.");
    } else if (comando == "d") {
      girando = false; // Detiene el giro infinito
      Serial.println("Giro infinito detenido.");
    } else {
      int angulo = comando.toInt(); // Intenta convertir el comando a un número
      int pasos = map(angulo, 0, 360, 0, pasosPorRevolucion); // Calcula los pasos necesarios
      motor.step(pasos); // Gira el motor
      Serial.print("Motor movido ");
      Serial.print(angulo);
      Serial.println(" grados.");
    }
  }

  // Control del giro infinito
  if (girando) {
    motor.step(direccion); // Gira en la dirección especificada
  }
}