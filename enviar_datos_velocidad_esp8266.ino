#include <ESP8266WiFi.h>

// ----------------------
// CONFIGURACIÓN WIFI
// ----------------------
const char* ssid = "ErickP";
const char* password = "12345678";

WiFiServer server(5000);   // Puerto donde tu PC recibirá los datos
WiFiClient client;


bool detener = false;

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

  delay(10000); // espera 10 segundos a iniciar el loop
}

void loop() {
  if (!client || !client.connected()) {
    client = server.available();
  }

  unsigned long ahora = millis();

  // Motores funcionando
  if(detener == false) { // si todavia no se ha parado
    digitalWrite(motorDer_IN1, HIGH);
    digitalWrite(motorIzq_IN4, HIGH);
  }
  
  // Cada 1 segundo calcular velocidades
  if (ahora - ultimoTiempo >= 1000) {

    float circ = 3.1416 * diametroRueda;

    rpmIzq = (pulsosIzq / numHuecosEncoder) * 60.0;
    rpmDer = (pulsosDer / numHuecosEncoder) * 60.0;

    velocidadIzq = (circ * rpmIzq) / 60.0;  // m/s
    velocidadDer = (circ * rpmDer) / 60.0;  // m/s

    // ---- CÁLCULO DE POSICIÓN ----
    float velProm = (velocidadIzq + velocidadDer) / 2.0;  // velocidad del robot

    float dt = (ahora - ultimoTiempo) / 1000.0;  // segundos
    float distancia = velProm * dt;

    acumularDistancia(distancia);

    // Enviar datos
    if (client && client.connected()) {
      client.print(xTotal, 3); client.print(",");
      client.print(yTotal, 3); client.print(",");
      client.print(velocidadIzq, 3); client.print(",");
      client.print(velocidadDer, 3); client.print(",");
      client.println(velProm, 3);
    }

    Serial.print("VL: "); Serial.print(velocidadIzq, 3); 
    Serial.print(" | VR: "); Serial.print(velocidadDer, 3);
    Serial.print(" | Pos X: "); Serial.print(xTotal);
    Serial.print(" | Pos Y: "); Serial.println(yTotal);

    pulsosIzq = 0;
    pulsosDer = 0;
    ultimoTiempo = ahora;
  }

  movimientoPredefinido(); // ejecuta movimiento predefinido
}

void parar() {
  digitalWrite(motorDer_IN1, LOW);
  digitalWrite(motorDer_IN2, LOW);
  digitalWrite(motorIzq_IN3, LOW);
  digitalWrite(motorIzq_IN4, LOW);
}

void movimientoPredefinido() {
  static bool g1 = false;
  static bool g2 = false;
  static bool g3 = false;

  unsigned long t = millis();

  if (!g1 && t >= 25000) {
      g1 = true;
      girarIzquierda90Grad();
  }

  if (!g2 && t >= 35000) {
      g2 = true;
      girarIzquierda90Grad();
  }

  if (!g3 && t >= 40000) {
      g3 = true;
      girarIzquierda90Grad();
  }

  if (!detener && t >= 60000) {
      detener = true;
      parar();
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
