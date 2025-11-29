#include <ESP8266WiFi.h>

// ----------------------
// CONFIGURACIÓN WIFI
// ----------------------
const char* ssid = "ErickP";
const char* password = "12345678";

WiFiServer server(5000);   // Puerto donde tu PC recibirá los datos
WiFiClient client;

// --------------------------
// Pines ESP8266
// --------------------------

const unsigned motorDer_IN1 = D1;
const unsigned motorDer_IN2 = D2;

const unsigned motorIzq_IN4 = D6;
const unsigned motorIzq_IN3 = D0;

// Sensores IR
const unsigned sensorPinDer = D5;
const unsigned sensorPinIzq = D7;

// --------------------------
// Variables encoder
// --------------------------
volatile unsigned int pulsosIzq = 0;
volatile unsigned int pulsosDer = 0;

// Datos físicos
const double numHuecosEncoder = 20.0;
const double diametroRueda = 0.06;

// Cálculos
unsigned long ultimoTiempo = 0;
float rpmIzq = 0.0;
float rpmDer = 0.0;
float velocidadIzq = 0.0;
float velocidadDer = 0.0;

// variables posicion
float xTotal = 0.0;
float yTotal = 0.0;
int theta = 0;   // grados: 0, 90, 180, 270

unsigned long ultimoCalculoDist = 0;

// ISR
void ICACHE_RAM_ATTR contarPulsoIzq() { pulsosIzq++; }
void ICACHE_RAM_ATTR contarPulsoDer() { pulsosDer++; }

void setup() {
  Serial.begin(9600);

  // Motores
  pinMode(motorDer_IN1, OUTPUT);
  pinMode(motorDer_IN2, OUTPUT);
  pinMode(motorIzq_IN4, OUTPUT);
  pinMode(motorIzq_IN3, OUTPUT);

  // Sensores
  pinMode(sensorPinDer, INPUT);
  pinMode(sensorPinIzq, INPUT);

  // Interrupciones
  attachInterrupt(digitalPinToInterrupt(sensorPinDer), contarPulsoDer, RISING);
  attachInterrupt(digitalPinToInterrupt(sensorPinIzq), contarPulsoIzq, RISING);

  Serial.println("Conectando a WiFi...");

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWIFI CONECTADO");
  Serial.print("IP del ESP8266: ");
  Serial.println(WiFi.localIP());

  server.begin();
}

void loop() {

  if (!client || !client.connected()) {
    client = server.available();
  }

  if (client && client.available()) {

    char c = client.read();       // lee 1 byte
    String cmd = String(c);       // conviértelo a String
    cmd.toUpperCase();            // mayúsculas siempre

    Serial.print("Comando recibido: ");
    Serial.println(cmd);

    if (cmd == "W") {
      avanzar(1000);
    }
    else if (cmd == "A") {
      girarIzquierda90Grad();
    }
    else if (cmd == "D") {
      girarDerecha90Grad();
    }
  }
}


/**
  * @brief gira a la izquierda 90 grados respecto a la linea de trayectoria lineal (movimiento cuando los dos motores estan activos).
    Lograr girar encendiendo solo el motor derecho por un determinado intervalo de tiempo (750 milisegundos).
*/
void girarIzquierda90Grad() {
  // apagar todo antes de encender
  digitalWrite(motorIzq_IN4, LOW);
  digitalWrite(motorIzq_IN3, LOW);
  digitalWrite(motorDer_IN2, LOW);

  digitalWrite(motorDer_IN1,HIGH);
  delay(750);
  digitalWrite(motorDer_IN1, LOW);

  theta += 90;
  if(theta >= 360) theta -= 360;
}

/**
  * @brief gira a la derecha 90 grados respecto a la linea de trayectoria lineal (movimiento cuando los dos motores estan activos).
    Logra girar encendiendo solo el motor izquierdo por un determinado intervalo de tiempo (750 milisegundos).
*/
void girarDerecha90Grad() {
  // apagar todo antes de encender
  digitalWrite(motorIzq_IN3, LOW);
  digitalWrite(motorDer_IN2, LOW);
  digitalWrite(motorDer_IN1, LOW);

  digitalWrite(motorIzq_IN4,HIGH);
  delay(750);
  digitalWrite(motorIzq_IN4, LOW);

  theta -= 90;
  if(theta < 0) theta += 360;
}

void avanzar(unsigned long ms) {
  digitalWrite(motorDer_IN1, HIGH);
  digitalWrite(motorIzq_IN4, HIGH);
  delay(ms);
  digitalWrite(motorDer_IN1, LOW);
  digitalWrite(motorIzq_IN4, LOW);
}

void acumularDistancia(float d) {
  switch (theta) {

    case 0:   // mirando +X
      xTotal += d;
      break;

    case 90:  // mirando +Y
      yTotal += d;
      break;

    case 180: // mirando -X
      xTotal -= d;
      break;

    case 270: // mirando -Y
      yTotal -= d;
      break;
  }
}
