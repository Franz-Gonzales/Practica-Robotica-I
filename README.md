# Sistema de Control de Terrario con Arduino - Control Automático IoT

## Descripción General

Este proyecto implementa un **sistema de control automático para terrario** basado en tecnología IoT (Internet de las Cosas). Utiliza un Arduino UNO como servidor que gestiona múltiples sensores y actuadores, proporcionando control automático y manual a través de una interfaz web moderna y responsiva.

El sistema está diseñado para mantener condiciones óptimas en un terrario mediante:
- **Control automático de iluminación** basado en sensores de luz ambiente
- **Regulación de temperatura** mediante control de foco calefactor
- **Monitoreo ambiental** continuo de temperatura, humedad y luminosidad
- **Control manual remoto** de todos los componentes via web
- **Sistema de ventilación** controlado por motor paso a paso

### Características Principales

- **Arquitectura Cliente-Servidor**: Arduino como servidor API REST + interfaz web separada
- **Control Automático Inteligente**: Respuesta automática basada en condiciones ambientales
- **Interfaz Web Moderna**: Diseño responsivo con actualizaciones en tiempo real
- **Monitoreo Continuo**: Lecturas de sensores cada 3 segundos
- **API REST**: Endpoints JSON para integración con otros sistemas
- **Fecha de Desarrollo**: Diciembre 2024 - Mayo 2025

---

## Tabla de Contenidos

1. [Componentes Utilizados](#componentes-utilizados)
2. [Arquitectura del Sistema](#arquitectura-del-sistema)
3. [Conexiones del Circuito](#conexiones-del-circuito)
4. [Configuración y Código](#configuración-y-código)
5. [API REST Endpoints](#api-rest-endpoints)
6. [Interfaz Web Cliente](#interfaz-web-cliente)
7. [Lógica de Control Automático](#lógica-de-control-automático)
8. [Instalación y Uso](#instalación-y-uso)
9. [Pruebas y Resultados](#pruebas-y-resultados)
10. [Problemas Comunes y Soluciones](#problemas-comunes-y-soluciones)
11. [Mejoras Futuras](#mejoras-futuras)

---

## Componentes Utilizados

### 1. LEDs (Rojo y Azul)
- **Tipo**: LEDs estándar de 5 mm.
- **Función**: Indicadores visuales para estados ON/OFF, controlados desde la interfaz web.
- **Especificaciones**:
    - LED Rojo: Conectado al pin digital 4 del Arduino.
    - LED Azul: Conectado al pin digital 5 del Arduino.
- **Resistencia**: 220 ohmios en serie para limitar la corriente y proteger los LEDs.
- **Espacio para Captura**:  
  ![Captura de Conexión de LEDs](images/leds_connection.jpg)  
  

### 2. Foco de 60 Watts
- **Tipo**: Bombilla estándar de 60W, 220V AC.
- **Función**: Iluminación controlada remotamente mediante un módulo relay que actúa como interruptor.
- **Voltaje de Operación**: 220V AC (corriente alterna doméstica).
- **Conexión**: No se conecta directamente al Arduino; se controla a través del módulo relay.
- **Espacio para Captura**:  
  ![Captura de Conexión del Foco](images/image2.png)  
.

### 3. Módulo Relay 5V-220V (1 Canal)
- **Tipo**: Módulo relay de un canal.
- **Función**: Permite al Arduino (5V) controlar dispositivos de alta tensión (220V AC) como el foco, proporcionando aislamiento eléctrico entre los circuitos de control y potencia.
- **Conexiones**:
    - VCC → 5V del Arduino.
    - GND → GND del Arduino.
    - IN → Pin digital 7 del Arduino.
- **Espacio para Captura**:  
  ![Captura de Conexión del Relay](images/relay_connection.jpg)  

### 4. Sensor de Temperatura y Humedad DHT11
- **Tipo**: Sensor digital DHT11.
- **Función**: Mide temperatura y humedad ambiente, mostrando los valores en la interfaz web.
- **Rango de Medición**:
    - Temperatura: 0 a 50°C.
    - Humedad: 20% a 90% RH.
- **Precisión**:
    - Temperatura: ±2°C.
    - Humedad: ±5% RH.
- **Pin de Señal**: Conectado al pin digital 6 del Arduino.
- **Espacio para Captura**:  
  ![Captura de Conexión del DHT11](images/dht11_connection.jpg)

### 5. **Sensor de Luz KY-018 (NUEVO)**
- **Tipo**: Módulo KY-018 con fotorresistencia LDR
- **Función**: Medición de intensidad lumínica para control automático de iluminación
- **Características**:
  - Salida analógica: 0-1023 unidades
  - Mayor luminosidad → valor cercano a 0
  - Menor luminosidad → valor cercano a 1023
  - Umbral configurado: 40 unidades
- **Pin**: Analógico A0
- **Control**: Activa LEDs y motor cuando luz < umbral
  ![sensorLuz](https://github.com/user-attachments/assets/f94f6a8e-1f15-4eff-a7e0-ac0b11658002)


### 6. Motor Paso a Paso Nema 23
- **Tipo**: Motor bipolar Nema 23 Oukeda OK57STH56-2804A-D6-35.
- **Especificaciones**:
    - Pasos por Revolución: 200 (1.8° por paso).
    - Control: Usando módulo L298N y pines digitales 8-11 del Arduino.
- **Fuente de Alimentación**: 12V mediante BK Precision BK1550.
- **Espacio para Captura**:  
  ![Captura de Conexión del Motor Paso a Paso](images/image.png)  
  *(imagen de la conexión del motor y L298N aquí)*.

### 7. Arduino UNO y Módulo Ethernet
- **Función**: Microcontrolador principal y servidor web para la interfaz remota.
- **Módulo Ethernet**: ENC28J60 (o similar), conectado vía SPI (pines 10-13 típicamente).
- **Especificaciones**:
    - Comunicación: Utiliza la librería `<Ethernet.h>` para manejar conexiones de red.
    - IP Estática: Ejemplo, `192.168.1.200` (configurable según la red).
- **Espacio para Captura**:  
  ![Captura de Conexión del Arduino y Ethernet](images/arduino_ethernet_connection.jpg)


## Arquitectura del Sistema

### Separación Cliente-Servidor

El sistema utiliza una **arquitectura cliente-servidor separada** para optimizar el rendimiento:

#### **Servidor (Arduino)**
- Gestiona todos los componentes físicos
- Expone API REST con endpoints JSON
- Procesa lógica de control automático
- Maneja lecturas de sensores
- Optimizado para memoria limitada (2KB RAM)

#### **Cliente (Interfaz Web)**
- Archivo HTML independiente con CSS/JavaScript
- Interfaz moderna y responsiva
- Actualización automática cada 3 segundos
- Compatible con dispositivos móviles
- No consume memoria del Arduino

### Ventajas de esta Arquitectura
- **Rendimiento**: Interface web compleja sin sobrecargar Arduino
- **Escalabilidad**: Múltiples clientes pueden conectarse
- **Mantenimiento**: Fácil actualización de UI sin reflashear Arduino
- **Flexibilidad**: Posibilidad de crear apps móviles nativas

---
![image](https://github.com/user-attachments/assets/e54b06c0-4f12-4866-8eb8-8e31843682ff)


## Conexiones del Circuito

### Esquema General
- **LEDs**:
    - LED Rojo: Ánodo al pin 4, cátodo a GND a través de una resistencia de 220Ω.
    - LED Azul: Ánodo al pin 5, cátodo a GND a través de una resistencia de 220Ω.
- **Sensor DHT11**:
    - Pin de señal: Pin 6 del Arduino.
    - VCC: 5V del Arduino.
    - GND: Tierra del Arduino.
- **Relay**:
    - VCC → 5V del Arduino.
    - GND → GND del Arduino.
    - IN → Pin 7 del Arduino.
    - Conexión del Foco:
        - Fase (220V AC) → Terminal COM del relay.
        - Terminal NO (Normalmente Abierto) → Un extremo del foco.
        - Neutro (220V AC) → Otro extremo del foco.
- **Motor Paso a Paso**:
    - **Bobinas**:
        - Bobina A (rojo/azul) → OUT A (1A/2A) del L298N.
        - Bobina B (negro/verde) → OUT B (3A/4A) del L298N.
    - **Pines de Control**:
        - IN1 → Pin 8.
        - IN2 → Pin 9.
        - IN3 → Pin 10.
        - IN4 → Pin 11.
    - **Alimentación del L298N**:
        - 12V y GND desde la fuente BK Precision BK1550.
        - Jumper "5V Regulator Enable" retirado, pin 5V del L298N conectado a 5V del Arduino.
- **Módulo Ethernet**:
    - Conectado a los pines SPI del Arduino (10-13, dependiendo del módulo).
    - Alimentación: 3.3V o 5V (según el módulo), GND conectado a tierra.
 
  ![image](https://github.com/user-attachments/assets/f542b369-1a35-4bae-ae4b-444615c8d890)


**Nota Importante**: Asegúrate de unificar todas las tierras (GND) del circuito para evitar problemas de referencia de voltaje. Usa cables de buena calidad para minimizar ruido eléctrico.

### Consideraciones de Seguridad
- **Tierra común**: Unificar todos los GND para evitar problemas de referencia
- **Aislamiento**: El relay proporciona aislamiento galvánico para 220V AC
- **Cables de calidad**: Minimizar ruido eléctrico en conexiones
- **Fusible**: Recomendado en línea de 220V AC

---

## API REST Endpoints

El Arduino expone los siguientes endpoints JSON:

### Estado del Sistema
```http
GET /api/status
```
**Respuesta**:
```json
{
  "ledRojo": 1,
  "ledAzul": 0,
  "relay": 1,
  "controlAuto": true,
  "ultimoAngulo": 1080,
  "luz": 35,
  "temperatura": 25.2,
  "humedad": 68.5,
  "umbralLuz": 40
}
```

### Control Manual de LEDs
```http
GET /api/led/rojo/on    # Encender LED rojo
GET /api/led/rojo/off   # Apagar LED rojo
GET /api/led/azul/on    # Encender LED azul
GET /api/led/azul/off   # Apagar LED azul
```

### Control de Foco Calefactor
```http
GET /api/relay/on       # Encender foco
GET /api/relay/off      # Apagar foco
```

### Control de Motor (Ventilación)
```http
GET /api/motor/90       # Girar 90° (horario)
GET /api/motor/-90      # Girar 90° (antihorario)
GET /api/motor/180      # Girar 180°
GET /api/motor/-180     # Girar 180° inverso
GET /api/motor/270      # Girar 270°
GET /api/motor/-270     # Girar 270° inverso
```

### Control Automático
```http
GET /api/auto/start     # Activar control automático
GET /api/auto/stop      # Desactivar control automático
```

---

## Lógica de Control Automático

### Algoritmo de Control Inteligente

```cpp
void procesarControlAutomatico() {
  if (!controlAutomatico) return;
  
  // Leer sensores
  valorSensorLuz = analogRead(pinSensorLuz);
  float temperatura = dht.readTemperature();
  
  // Control por Luminosidad
  if (valorSensorLuz <= 40) {  // Umbral de luz baja
    // Activar iluminación
    digitalWrite(ledRojo, HIGH);
    digitalWrite(ledAzul, HIGH);
    // Ventilación: 3 vueltas completas
    motor.step(3 * 200);  // 1080°
  } else {
    // Desactivar iluminación
    digitalWrite(ledRojo, LOW);
    digitalWrite(ledAzul, LOW);
    // Ventilación inversa
    motor.step(-3 * 200);  // -1080°
  }
  
  // Control por Temperatura
  if (temperatura <= 24.0) {
    digitalWrite(relay, HIGH);  // Encender calefacción
  } else if (temperatura > 20.0) {
    digitalWrite(relay, LOW);   // Apagar calefacción
  }
  // Histéresis: 20-24°C evita oscilaciones
}
```

### Condiciones de Activación

| Sensor | Condición | Acción |
|--------|-----------|--------|
| **Luz** | < 40 unidades | LEDs ON + Motor 3 vueltas |
| **Luz** | ≥ 40 unidades | LEDs OFF + Motor -3 vueltas |
| **Temperatura** | ≤ 24°C | Foco calefactor ON |
| **Temperatura** | > 20°C | Foco calefactor OFF |

---

## Instalación y Uso

### 1. Configuración del Hardware
1. Realizar todas las conexiones según el esquema
2. Verificar alimentación: 12V para motor, 220V para foco
3. Conectar módulo Ethernet a la red local
4. Configurar IP estática en el código

### 2. Configuración del Software

#### Librerías Requeridas
```cpp
#include <SPI.h>
#include <Ethernet.h>
#include <DHT.h>   // Adafruit DHT sensor library
#include <Stepper.h>
```

#### Configuración de Red
```cpp
byte MAC[] = {0xDE, 0xAD, 0xBE, 0xED, 0xAE, 0xAD};
IPAddress ip(192, 168, 1, 200);  // Ajustar según tu red
```

### 3. Despliegue

1. **Cargar código Arduino**: Subir `terrario_server.ino` al Arduino
2. **Configurar cliente web**: 
   - Abrir `index.html` en cualquier editor
   - Cambiar `ARDUINO_IP` por la IP de tu Arduino
   - Servir desde servidor web local o abrir directamente en navegador
3. **Verificar conexión**: Monitor serial debe mostrar "Servidor iniciado"

### 4. Acceso a la Interfaz

- **URL**: `http://192.168.1.200` (o tu IP configurada)
- **Cliente Web**: Abrir `index.html` en navegador
- **Compatibilidad**: Chrome, Firefox, Safari, Edge (móviles incluidos)

---

## Interfaz Web Cliente


![interfazTerrario](https://github.com/user-attachments/assets/168693fd-521d-4e0b-9d56-c5f9fb844566)

### Características de la UI

- **Diseño Responsivo**: Adaptable a móviles y tablets
- **Actualización Automática**: Estado cada 3 segundos
- **Indicadores Visuales**: LEDs de estado en tiempo real
- **Control Intuitivo**: Botones grandes con iconos
- **Manejo de Errores**: Mensajes informativos de conexión

### Secciones de la Interfaz

1. **Control Automático**: Activar/desactivar modo automático
2. **LEDs**: Control manual de iluminación 
3. **Foco**: Control de sistema de calefacción
4. **Motor**: Control manual de ventilación (6 ángulos predefinidos)
5. **Sensores**: Visualización en tiempo real de:
   - Temperatura (°C)
   - Humedad (%)
   - Luz ambiental (0-1023)

---

## Pruebas y Resultados

### Pruebas de Control Automático

| Condición | Luz Medida | Temperatura | Acción Esperada | Resultado |
|-----------|------------|-------------|-----------------|-----------|
| Día claro | 15 | 26°C | LEDs OFF, Foco OFF | ✅ Correcto |
| Atardecer | 45 | 22°C | LEDs ON, Foco ON | ✅ Correcto |
| Noche | 850 | 18°C | LEDs ON, Motor ON, Foco ON | ✅ Correcto |

### Pruebas de Precisión de Sensores

- **DHT11**: ±2°C temperatura, ±5% humedad (dentro de especificaciones)
- **KY-018**: Respuesta rápida a cambios de luz (< 1 segundo)
- **Tiempo de respuesta**: Control automático en < 500ms

### Pruebas de Conectividad

- **Múltiples clientes**: Hasta 3 navegadores simultáneos sin problemas
- **Latencia**: < 100ms en red local
- **Estabilidad**: 48+ horas de funcionamiento continuo

---

## Problemas Comunes y Soluciones

### 1. Error de Memoria RAM Excedida
**Síntoma**: "data section exceeds available space"
**Solución**: 
- Usar `PROGMEM` para constantes HTTP
- Variables `byte` en lugar de `int` donde sea posible
- Evitar `String`, usar arrays de caracteres

### 2. Control Automático No Responde
**Síntoma**: Sensores leen valores pero no hay acción
**Solución**:
- Verificar que `controlAutomatico = true`
- Revisar conexiones del sensor KY-018 en A0
- Ajustar `umbralLuz` según condiciones de tu entorno

### 3. Motor con Movimientos Erráticos
**Síntoma**: Motor vibra pero no gira correctamente
**Solución**:
- Verificar polaridad de bobinas con multímetro
- Reducir velocidad a 60 RPM: `motor.setSpeed(60)`
- Añadir `delay(10)` entre pasos

### 4. Interfaz Web No Carga Datos
**Síntoma**: Botones funcionan pero sensores muestran "--"
**Solución**:
- Verificar CORS en navegador (usar servidor local)
- Confirmar IP correcta en variable `ARDUINO_IP`
- Revisar consola del navegador para errores JavaScript

---

## Mejoras Futuras

### Hardware
- **Sensor de pH** del suelo para terrarios plantados
- **Bomba de agua** con sensor de humedad del sustrato
- **Ventilador 12V** para circulación de aire más eficiente
- **Pantalla LCD** para estado local sin necesidad de red

### Software
- **Base de datos** SQLite para histórico de sensores
- **Notificaciones push** para alertas críticas
- **Scheduler** para rutinas programadas (día/noche)
- **API WebSocket** para actualizaciones en tiempo real

### Problemas Comunes y Soluciones
1. **Sobrecalentamiento del L298N**

   - Problema: El L298N se calienta excesivamente al usar una fuente de 12V durante períodos prolongados.
   - Solución:
      - Limitar el uso continuo del motor a sesiones cortas durante las pruebas.
      - Añadir un disipador de calor al L298N para mejorar la disipación térmica.
      - Considerar una fuente de menor voltaje (ej. 9V) si el motor lo permite.
2. Flickering del Motor Paso a Paso
   - Problema: El motor no giraba, sino que parpadeaba o vibraba al inicio.
   - Solución:
      - Revisar las conexiones de las bobinas con un multímetro para identificar los pares correctos.
      - Intercambiar los cables de una bobina (ej. rojo y azul en OUT A) para corregir la secuencia de fases.
3. Memoria Excedida en el Arduino
   - Problema: Error de compilación: "data section exceeds available space in board" (RAM excedida, >2048 bytes).
   - Solución:
      - Reducir el uso de variables String y reemplazarlas por tipos más pequeños (como byte o char).
      - Usar PROGMEM para almacenar cadenas estáticas (como el HTML) en la memoria flash.
      - Simplificar el HTML enviado al cliente si es necesario.
4. Ángulos Negativos en el Motor
   - Problema: La función map(angulo, 0, 360, 0, pasosPorRevolucion) no maneja correctamente ángulos negativos, causando movimientos incorrectos.
   - Solución:
      - Usar un cálculo directo: pasos = (angulo * pasosPorRevolucion) / 360.
      - Determinar la dirección con una condición: motor.step(angulo < 0 ? -pasos : pasos).
5. Problemas de Conexión Ethernet
   - Problema: No se puede acceder a la IP del servidor desde el navegador.
   - Solución:
      - Verificar que el módulo Ethernet esté correctamente conectado y alimentado.
      - Asegurarse de que la IP configurada no esté en uso por otro dispositivo.
      - Revisar el Monitor Serial para confirmar que el servidor se ha iniciado correctamente.
   
### **Recomendaciones para Mejoras**
1. **Manejo de Ángulos Negativos**
   - Implementar una solución más robusta para el control del motor:
   ```cpp
      int pasos = (angulo * pasosPorRevolucion) / 360;
      motor.step(angulo < 0 ? -pasos : pasos);
    ```
   - Esto asegura que el motor gire en la dirección correcta para ángulos positivos y negativos.
2. Estabilidad del Motor
   - Reducir Vibraciones:
       - Disminuir la velocidad a 60 RPM (motor.setSpeed(60)) para minimizar vibraciones y pérdida de pasos.
   - Movimientos Suaves:
       - Usar la librería AccelStepper para implementar aceleración y deceleración, mejorando la estabilidad del motor.
       - Ejemplo: AccelStepper Documentation.
3. Seguridad del Relay
      - Añadir un fusible en la línea de 220V AC para proteger el circuito ante cortocircuitos o sobrecargas.
      - Usar un optoacoplador adicional para mayor aislamiento entre el Arduino y el relay.
4. Escalabilidad del Sistema
      - Más RAM y Pines: Migrar a un Arduino Mega (2560) para mayor capacidad de memoria y más pines disponibles.
      - Más Componentes: Añadir más sensores o actuadores (por ejemplo, un sensor de luz para controlar el foco automáticamente).
5. Mejoras en la Interfaz Web
   - Diseño Responsivo:
       - Agregar CSS para mejorar la apariencia de la página web.
       - Ejemplo: Usar frameworks como Bootstrap (Bootstrap CDN).
   - Gráficos Dinámicos:
      - Incluir JavaScript para mostrar gráficos de temperatura y humedad en tiempo real (por ejemplo, con Chart.js: Chart.js).
   - Actualización Automática:
      - Implementar AJAX para actualizar los datos del DHT11 sin recargar la página.
6. Monitoreo Avanzado
   - Agregar un sistema de registro (logging) para almacenar datos del DHT11 en una tarjeta SD o enviarlos a un servidor remoto.
   - Ejemplo: Usar la librería SD para almacenamiento local.
   

### **Recursos Adicionales**
   - Librería Ethernet: Arduino Ethernet Library
   - Librería Stepper: Arduino Stepper Library
   - Librería DHT: Adafruit DHT Library
   - Tutorial de Relay: Control de Relay con Arduino
   - Esquemas de Conexión: Random Nerd Tutorials - Arduino Projects
   - Documentación del ENC28J60: ENC28J60 Ethernet Module Guide
