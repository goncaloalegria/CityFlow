/*
 * ============================================
 *     CITYFLOW - ARDUINO 1 (SLAVE)
 *     Monitorizacao das Vagas + LEDs
 * ============================================
 * 
 * Este Arduino:
 *   - Monitoriza 3 vagas com sensores ultrasonicos
 *   - Controla LEDs indicadores (verde/vermelho)
 *   - Comunica estado via I2C com Arduino 2
 * 
 * Ligacoes dos Sensores:
 *   Sensor Vaga 1: TRIG=D2, ECHO=D3
 *   Sensor Vaga 2: TRIG=D4, ECHO=D5
 *   Sensor Vaga 3: TRIG=D6, ECHO=D7
 * 
 * Ligacoes dos LEDs:
 *   LEDs Vaga 1: Vermelho=D8,  Verde=D9
 *   LEDs Vaga 2: Vermelho=D10, Verde=D11
 *   LEDs Vaga 3: Vermelho=D12, Verde=D13
 * 
 * Ligacoes I2C (para Arduino 2):
 *   SDA -> A4
 *   SCL -> A5
 *   Endereco I2C: 0x10
 */

#include <Wire.h>

// ----- Sensores Ultrasonicos -----
const int TRIG_PINS[3] = {2, 4, 6};
const int ECHO_PINS[3] = {3, 5, 7};

// ----- LEDs (vermelho e verde para cada vaga) -----
const int LED_VERMELHO[3] = {8, 10, 12};
const int LED_VERDE[3]    = {9, 11, 13};

// ----- Configuracao -----
const int LIMIAR_OCUPADO_CM = 9;

// ----- Estado de ocupacao -----
// bit0=vaga1, bit1=vaga2, bit2=vaga3
volatile byte ocupacaoMask = 0;

// ---------- Funcoes auxiliares ----------

float lerDistanciaCm(int trigPin, int echoPin) {
  // Pulso de trigger
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Echo (timeout para evitar bloqueios)
  long duracao = pulseIn(echoPin, HIGH, 20000UL); // 20ms

  if (duracao == 0) {
    // Sem leitura válida → considera muito longe
    return 9999.0;
  }

  // Conversão para cm (aprox.)
  float distancia = duracao / 58.0;
  return distancia;
}

// Handler chamado quando o master (Arduino 2) pede dados
void onI2CRequest() {
  Wire.write(ocupacaoMask); // envia 1 byte com bits de ocupação
}

// ---------- Setup e Loop ----------

void setup() {
  // Configurar pinos dos sensores
  for (int i = 0; i < 3; i++) {
    pinMode(TRIG_PINS[i], OUTPUT);
    pinMode(ECHO_PINS[i], INPUT);
  }

  // Configurar pinos dos LEDs
  for (int i = 0; i < 3; i++) {
    pinMode(LED_VERMELHO[i], OUTPUT);
    pinMode(LED_VERDE[i], OUTPUT);
  }

  // Iniciar I2C como slave no endereço 0x10
  Wire.begin(0x10);
  Wire.onRequest(onI2CRequest);

  // Opcional: Serial para debug
  Serial.begin(9600);
}

void loop() {
  byte novaMask = 0;

  for (int i = 0; i < 3; i++) {
    float dist = lerDistanciaCm(TRIG_PINS[i], ECHO_PINS[i]);

    bool ocupado = (dist < LIMIAR_OCUPADO_CM);
    if (ocupado) {
      novaMask |= (1 << i);
    }

    // Controlar LEDs: verde = livre, vermelho = ocupado
    if (ocupado) {
      digitalWrite(LED_VERMELHO[i], HIGH);
      digitalWrite(LED_VERDE[i], LOW);
    } else {
      digitalWrite(LED_VERMELHO[i], LOW);
      digitalWrite(LED_VERDE[i], HIGH);
    }

    // Debug opcional
    // Serial.print("Vaga "); Serial.print(i+1);
    // Serial.print(" dist="); Serial.print(dist);
    // Serial.print("cm ocupado="); Serial.println(ocupado ? "SIM" : "NAO");
  }

  ocupacaoMask = novaMask;

  delay(200); // leitura ~5x por segundo
}
