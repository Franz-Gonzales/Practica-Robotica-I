#include <SPI.h>
#include <Ethernet.h>
#include <DHT.h>
#include <Stepper.h>

// Dirección MAC e IP
byte MAC[] = {0xDE, 0xAD, 0xBE, 0xED, 0xAE, 0xAD};
IPAddress ip(192, 168, 1, 200);
EthernetServer server(80);

// Pines LED
const byte ledRojo = 4;
const byte ledAzul = 5;
const byte pinRelay = 7;  // Pin para el relay (ajusta según tu conexión)

// Estado de LEDs y Relay (usando bytes en lugar de Strings para ahorrar memoria)
byte estadoLedRojo = 0;  // 0 = apagado, 1 = encendido
byte estadoLedAzul = 0;  // 0 = apagado, 1 = encendido
byte estadoRelay = 0;    // 0 = apagado, 1 = encendido

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

// Constantes para textos (usar PROGMEM para guardar strings en memoria de programa)
const char HTTP_HEAD[] PROGMEM = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: close\r\n\r\n";
const char HTML_HEAD[] PROGMEM = "<!DOCTYPE html><html><head><title>Control Arduino</title><style>body{font-family:Arial;margin:10px}h1{color:#333;font-size:18px}a{color:blue;margin:5px;display:inline-block}</style></head><body>";
const char HTML_END[] PROGMEM = "</body></html>";

void setup() {
  pinMode(ledRojo, OUTPUT);
  pinMode(ledAzul, OUTPUT);
  pinMode(pinRelay, OUTPUT);
  
  // Inicializar relay apagado
  digitalWrite(pinRelay, LOW);

  Ethernet.begin(MAC, ip);
  server.begin();

  Serial.begin(9600);
  Serial.println(F("Iniciado")); // F() guarda strings en memoria flash
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
          // Control de LEDs
          if (peticion.indexOf(F("GET /ROJO_ON")) >= 0) {
            digitalWrite(ledRojo, HIGH);
            estadoLedRojo = 1;
          } else if (peticion.indexOf(F("GET /ROJO_OFF")) >= 0) {
            digitalWrite(ledRojo, LOW);
            estadoLedRojo = 0;
          }

          if (peticion.indexOf(F("GET /AZUL_ON")) >= 0) {
            digitalWrite(ledAzul, HIGH);
            estadoLedAzul = 1;
          } else if (peticion.indexOf(F("GET /AZUL_OFF")) >= 0) {
            digitalWrite(ledAzul, LOW);
            estadoLedAzul = 0;
          }

          // Control de TODOS (ambos LEDs y relay)
          if (peticion.indexOf(F("GET /TODOS_ON")) >= 0) {
            digitalWrite(ledRojo, HIGH);
            digitalWrite(ledAzul, HIGH);
            digitalWrite(pinRelay, HIGH);
            estadoLedRojo = 1;
            estadoLedAzul = 1;
            estadoRelay = 1;
          } else if (peticion.indexOf(F("GET /TODOS_OFF")) >= 0) {
            digitalWrite(ledRojo, LOW);
            digitalWrite(ledAzul, LOW);
            digitalWrite(pinRelay, LOW);
            estadoLedRojo = 0;
            estadoLedAzul = 0;
            estadoRelay = 0;
          }
          
          // Control solo de AMBOS LEDs (mantener para compatibilidad)
          if (peticion.indexOf(F("GET /AMBOS_ON")) >= 0) {
            digitalWrite(ledRojo, HIGH);
            digitalWrite(ledAzul, HIGH);
            estadoLedRojo = 1;
            estadoLedAzul = 1;
          } else if (peticion.indexOf(F("GET /AMBOS_OFF")) >= 0) {
            digitalWrite(ledRojo, LOW);
            digitalWrite(ledAzul, LOW);
            estadoLedRojo = 0;
            estadoLedAzul = 0;
          }
          
          // Control del Relay
          if (peticion.indexOf(F("GET /RELAY_ON")) >= 0) {
            digitalWrite(pinRelay, HIGH);
            estadoRelay = 1;
          } else if (peticion.indexOf(F("GET /RELAY_OFF")) >= 0) {
            digitalWrite(pinRelay, LOW);
            estadoRelay = 0;
          }

          // Control del motor paso a paso
          if (peticion.indexOf(F("GET /GIRAR_MOTOR?angulo=")) >= 0) {
            int inicioIndex = peticion.indexOf(F("GET /GIRAR_MOTOR?angulo=")) + 24;
            int finIndex = peticion.indexOf(F(" "), inicioIndex);
            String valorAngulo = peticion.substring(inicioIndex, finIndex);
            ultimoAngulo = valorAngulo.toInt();
            
            int pasos = map(ultimoAngulo, 0, 360, 0, pasosPorRevolucion);
            motor.step(pasos);
            Serial.print(F("Motor movido "));
            Serial.print(ultimoAngulo);
            Serial.println(F(" grados."));
          }

          // Leer valores del sensor
          float h = dht.readHumidity();
          float t = dht.readTemperature();

          // Enviar encabezado
          enviarTextoProgmem(cliente, HTTP_HEAD);
          enviarTextoProgmem(cliente, HTML_HEAD);

          // LED ROJO
          cliente.println(F("<h1>LED ROJO</h1>"));
          cliente.println(F("<a href='/ROJO_ON'>ON</a> | <a href='/ROJO_OFF'>OFF</a>"));
          cliente.print(F("<p>LED ROJO "));
          cliente.print(estadoLedRojo ? F("PRENDIDO") : F("APAGADO"));
          cliente.println(F("</p>"));

          // LED AZUL
          cliente.println(F("<h1>LED AZUL</h1>"));
          cliente.println(F("<a href='/AZUL_ON'>ON</a> | <a href='/AZUL_OFF'>OFF</a>"));
          cliente.print(F("<p>LED AZUL "));
          cliente.print(estadoLedAzul ? F("PRENDIDO") : F("APAGADO"));
          cliente.println(F("</p>"));
          
          // RELAY (FOCO)
          cliente.println(F("<h1>CONTROL FOCO</h1>"));
          cliente.println(F("<a href='/RELAY_ON'>ENCENDER</a> | <a href='/RELAY_OFF'>APAGAR</a>"));
          cliente.print(F("<p>RELAY "));
          cliente.print(estadoRelay ? F("ENCENDIDO") : F("APAGADO"));
          cliente.println(F("</p>"));

          // CONTROL TODOS
          cliente.println(F("<h1>CONTROL TODOS</h1>"));
          cliente.println(F("<a href='/TODOS_ON'>PRENDER TODOS</a> | <a href='/TODOS_OFF'>APAGAR TODOS</a>"));
          cliente.println(F("<p>Controla ambos LEDs y el foco simultáneamente</p>"));

          // MOTOR
          cliente.println(F("<h1>MOTOR PASO A PASO</h1>"));
          cliente.println(F("<form action='/GIRAR_MOTOR' method='get'>"));
          cliente.println(F("<label for='angulo'>Ángulo (-360 a 360):</label>"));
          cliente.println(F("<input type='number' id='angulo' name='angulo' min='-360' max='360' value='0'>"));
          cliente.println(F("<input type='submit' value='Girar'>"));
          cliente.println(F("</form>"));
          cliente.print(F("<p>Último movimiento: "));
          cliente.print(ultimoAngulo);
          cliente.println(F(" grados</p>"));

          // AMBIENTE
          cliente.println(F("<h1>MEDICION AMBIENTE</h1>"));
          if (isnan(t) || isnan(h)) {
            cliente.println(F("<p>Error al leer el sensor</p>"));
          } else {
            cliente.print(F("<p>Temperatura: "));
            cliente.print(t);
            cliente.println(F(" &deg;C</p>"));
            cliente.print(F("<p>Humedad: "));
            cliente.print(h);
            cliente.println(F(" %</p>"));
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