#include <SPI.h>
#include <Ethernet.h>
#include <DHT.h>
#include <Stepper.h>

// Dirección MAC e IP
byte MAC[] = {0xDE, 0xAD, 0xBE, 0xED, 0xAE, 0xAD};
IPAddress ip(192, 168, 1, 200);
EthernetServer server(80);

// Pines LED y Relay
const byte ledRojo = 4;
const byte ledAzul = 5;
const byte pinRelay = 7;

// Estado de LEDs y Relay
byte estadoLedRojo = 0;  // 0 = apagado, 1 = encendido
byte estadoLedAzul = 0;  // 0 = apagado, 1 = encendido
byte estadoRelay = 0;    // 0 = apagado, 1 = encendido
bool controlAutomatico = false; // Estado del control automático

// Sensor de luz KY-018
const int pinSensorLuz = A0;
const int umbralLuz = 40;
int valorSensorLuz = 0;

// Sensor DHT11
#define DHTPIN 6          // Pin digital para el DHT11
#define DHTTYPE DHT11     // Tipo de sensor
DHT dht(DHTPIN, DHTTYPE); // Inicializa sensor

// Configuración del motor paso a paso
const int pasosPorRevolucion = 200; // Cambia según tu motor
const int velocidadMotor = 100; // RPM
const byte IN1 = 8;
const byte IN2 = 9;
const byte IN3 = 10;
const byte IN4 = 11;
Stepper motor(pasosPorRevolucion, IN1, IN2, IN3, IN4);
int ultimoAngulo = 0;

// Constantes para textos (usar PROGMEM para ahorrar memoria)
const char HTTP_HEAD[] PROGMEM = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: close\r\n\r\n";
const char HTML_HEAD[] PROGMEM = "<!DOCTYPE html><html><head><title>Control Arduino</title><style>body{font-family:Arial;margin:10px}h1{color:#333;font-size:18px}a{color:blue;margin:5px;display:inline-block}</style></head><body>";
const char HTML_END[] PROGMEM = "</body></html>";

void setup() {
  pinMode(ledRojo, OUTPUT);
  pinMode(ledAzul, OUTPUT);
  pinMode(pinRelay, OUTPUT);
  pinMode(pinSensorLuz, INPUT);

  // Inicializar apagado
  digitalWrite(ledRojo, LOW);
  digitalWrite(ledAzul, LOW);
  digitalWrite(pinRelay, LOW);

  Ethernet.begin(MAC, ip);
  server.begin();

  Serial.begin(9600);
  Serial.println(F("Iniciado"));
  Serial.println(Ethernet.localIP());

  dht.begin();  // Inicia el sensor
  motor.setSpeed(velocidadMotor); // Configura la velocidad del motor
}

// Función para enviar texto desde PROGMEM
void enviarTextoProgmem(EthernetClient &cliente, const char *texto) {
  char c;
  while ((c = pgm_read_byte(texto++))) {
    cliente.write(c);
  }
}

void loop() {
  EthernetClient cliente = server.available();

  // Leer sensor de luz
  valorSensorLuz = analogRead(pinSensorLuz);

  // Leer valores del sensor DHT
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  // Control automático
  if (controlAutomatico) {
    // Condición para LEDs y motor basada en sensor de luz
    if (valorSensorLuz <= umbralLuz) {
      digitalWrite(ledRojo, HIGH);
      digitalWrite(ledAzul, HIGH);
      estadoLedRojo = 1;
      estadoLedAzul = 1;
      int pasos = 3 * pasosPorRevolucion; // 3 vueltas a la derecha
      motor.step(pasos);
      ultimoAngulo = 3 * 360;
      Serial.println(F("Luz <= 500: LEDs encendidos, motor 3 vueltas a la derecha"));
    } else {
      digitalWrite(ledRojo, LOW);
      digitalWrite(ledAzul, LOW);
      estadoLedRojo = 0;
      estadoLedAzul = 0;
      int pasos = -3 * pasosPorRevolucion; // 3 vueltas a la izquierda
      motor.step(pasos);
      ultimoAngulo = -3 * 360;
      Serial.println(F("Luz > 40: LEDs apagados, motor 3 vueltas a la izquierda"));
    }

    // Condición para el foco basada en temperatura
    if (t <= 24 && !isnan(t)) {
      digitalWrite(pinRelay, HIGH);
      estadoRelay = 1;
      Serial.println(F("Temperatura <= 20°C: Foco encendido"));
    } else if (t > 20 && !isnan(t)) {
      digitalWrite(pinRelay, LOW);
      estadoRelay = 0;
      Serial.println(F("Temperatura > 20°C: Foco apagado"));
    }
  }

  if (cliente) {
    Serial.println(F("Nueva petición!"));
    String peticion = "";
    boolean estadoActual = true;
    boolean finDeLinea = true;
    char c;

    while (cliente.connected()) {
      if (cliente.available()) {
        c = cliente.read();
        peticion += c;

        // Procesar petición cuando se detecte una línea en blanco
        if (c == '\n' && finDeLinea) {
          // Control automático
          if (peticion.indexOf(F("GET /INICIAR_AUTO")) >= 0) {
            controlAutomatico = true;
            Serial.println(F("Control automático iniciado"));
          } else if (peticion.indexOf(F("GET /TERMINAR_AUTO")) >= 0) {
            controlAutomatico = false;
            digitalWrite(ledRojo, LOW);
            digitalWrite(ledAzul, LOW);
            digitalWrite(pinRelay, LOW);
            estadoLedRojo = 0;
            estadoLedAzul = 0;
            estadoRelay = 0;
            Serial.println(F("Control automático terminado"));
          }

          // Enviar encabezado
          enviarTextoProgmem(cliente, HTTP_HEAD);
          enviarTextoProgmem(cliente, HTML_HEAD);

          // Estado LEDs
          cliente.println(F("<h1>ESTADO LEDs</h1>"));
          cliente.print(F("<p>LED ROJO: "));
          cliente.print(estadoLedRojo ? F("PRENDIDO") : F("APAGADO"));
          cliente.println(F("</p>"));
          cliente.print(F("<p>LED AZUL: "));
          cliente.print(estadoLedAzul ? F("PRENDIDO") : F("APAGADO"));
          cliente.println(F("</p>"));

          // Estado Foco
          cliente.println(F("<h1>ESTADO FOCO</h1>"));
          cliente.print(F("<p>RELAY: "));
          cliente.print(estadoRelay ? F("ENCENDIDO") : F("APAGADO"));
          cliente.println(F("</p>"));

          // Control Automático
          cliente.println(F("<h1>CONTROL AUTOMÁTICO</h1>"));
          cliente.println(F("<a href='/INICIAR_AUTO'>Iniciar Control Automático</a> | <a href='/TERMINAR_AUTO'>Terminar Control Automático</a>"));
          cliente.print(F("<p>Control Automático: "));
          cliente.print(controlAutomatico ? F("ACTIVADO") : F("DESACTIVADO"));
          cliente.println(F("</p>"));

          // MOTOR
          cliente.println(F("<h1>MOTOR PASO A PASO</h1>"));
          cliente.print(F("<p>Último movimiento: "));
          cliente.print(ultimoAngulo); // Corrección aquí
          cliente.println(F(" grados</p>"));

          // AMBIENTE
          cliente.println(F("<h1>MEDICIÓN AMBIENTE</h1>"));
          if (isnan(t) || isnan(h)) {
            cliente.println(F("<p>Error al leer el sensor</p>"));
          } else {
            cliente.print(F("<p>Temperatura: "));
            cliente.print(t);
            cliente.println(F(" °C</p>"));
            cliente.print(F("<p>Humedad: "));
            cliente.print(h);
            cliente.println(F(" %</p>"));
            cliente.print(F("<p>Luz: "));
            cliente.print(valorSensorLuz);
            cliente.println(F(" (Umbral: 500)</p>"));
          }

          enviarTextoProgmem(cliente, HTML_END);
          break;
        }

        if (c == '\n') {
          finDeLinea = true;
        } else if (c != '\r') {
          finDeLinea = false;
        }
      }
    }
    
    delay(5);
    cliente.stop();
    Serial.println(F("Cliente desconectado"));
  }
}