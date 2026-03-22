/*
 * ============================================
 *     CITYFLOW - ARDUINO 2 (MASTER)
 *     LCD + Cancela + Sensor Rua
 * ============================================
 * 
 * MODIFICADO: Adicionado envio de dados ao ESP32 via Serial
 * 
 * Ligações ao ESP32:
 *   Arduino 2 TX (pin 1) -> ESP32 GPIO16 (RX2)
 *   Arduino 2 RX (pin 0) -> ESP32 GPIO17 (TX2)  [opcional]
 *   Arduino 2 GND        -> ESP32 GND
 */

#include <Wire.h>
#include <LiquidCrystal.h>
#include <Servo.h>

// ----- I2C -----
const byte SLAVE_ADDR = 0x10; // Arduino 1

// ----- LCD (Ligação em 4 bits) -----
// LiquidCrystal(rs, en, d4, d5, d6, d7)
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

// ----- Servo (cancela) -----
Servo cancela;
const int SERVO_PIN = 9;
const int ANGULO_FECHADO = 0;
const int ANGULO_ABERTO  = 90;

// ----- LED amarelo -----
const int LED_AMARELO_PIN = 8;

// ----- Sensor ultrassónico da rua -----
const int TRIG_RUA = 6;
const int ECHO_RUA = 7;

// Limiares de distância (em cm) para detectar carro
const float DISTANCIA_CM = 10.0;

// ----- Estado de ocupação recebido do Arduino 1 -----
byte ocupacaoMaskAtual = 0;
int vagasLivresAtual   = 3;

int vagasLivresAnterior = -1; // para detectar saídas

// Flag para saber se estamos a mexer na cancela
bool cancelaOcupada = false;

// ----- Gestão de mensagens do LCD (vagas <-> Boas Festas) -----
bool mostrarMensagemFestas = false;
unsigned long proximaMudancaMensagem = 0;
int ultimoVagasMostradas = -1;
bool ultimaEraFestas = false;

// ----- NOVO: Para envio ao ESP32 -----
byte ocupacaoMaskAnterior = 255;  // valor impossível para forçar primeiro envio
unsigned long ultimoEnvioESP32 = 0;
const unsigned long INTERVALO_ENVIO = 500;  // enviar a cada 500ms

// ---------- Funções auxiliares ----------

float lerDistanciaRua() {
  digitalWrite(TRIG_RUA, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_RUA, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_RUA, LOW);

  long duracao = pulseIn(ECHO_RUA, HIGH, 20000UL); // timeout 20ms

  if (duracao == 0) {
    return 9999.0;
  }

  float distancia = duracao / 58.0;
  return distancia;
}

bool carroNaRua(float distCm) {
  return (distCm > 0 && distCm < DISTANCIA_CM);
}

int contarBitsOcupados(byte mask) {
  int c = 0;
  for (int i = 0; i < 3; i++) {
    if (mask & (1 << i)) c++;
  }
  return c;
}

// Ler estado do parque do Arduino 1
void atualizarEstadoParque() {
  Wire.requestFrom(SLAVE_ADDR, (byte)1);
  if (Wire.available()) {
    ocupacaoMaskAtual = Wire.read();
    int ocupados = contarBitsOcupados(ocupacaoMaskAtual);
    vagasLivresAtual = 3 - ocupados;
  }
}

// ====== NOVO: Enviar dados ao ESP32 ======
void enviarDadosESP32() {
  unsigned long agora = millis();
  
  // Enviar se mudou OU se passou o intervalo
  if (ocupacaoMaskAtual != ocupacaoMaskAnterior || agora - ultimoEnvioESP32 >= INTERVALO_ENVIO) {
    Serial.write(ocupacaoMaskAtual);  // Envia 1 byte (bitmask)
    ocupacaoMaskAnterior = ocupacaoMaskAtual;
    ultimoEnvioESP32 = agora;
  }
}

// Mostrar vagas no LCD (estado normal: vagas ou parque cheio)
void mostrarLCDVagas() {
  lcd.clear();
  if (vagasLivresAtual > 0) {
    lcd.setCursor(0, 0);
    lcd.print("Lugares vagos:");
    lcd.setCursor(0, 1);
    lcd.print("                ");
    lcd.setCursor(0, 1);
    lcd.print(vagasLivresAtual);
  } else {
    lcd.setCursor(0, 0);
    lcd.print("PARQUE CHEIO   ");
    lcd.setCursor(0, 1);
    lcd.print("Sem vagas      ");
  }
}

// Mostrar mensagem de parque cheio quando vem um carro da rua
void mostrarLCDParqueCheio() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("PARQUE CHEIO   ");
  lcd.setCursor(0, 1);
  lcd.print("Aguarde        ");
}

// Mostrar mensagem de saída (STOP)
void mostrarLCDSaidaStop() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("SAIDA          ");
  lcd.setCursor(0, 1);
  lcd.print("STOP           ");
}

// Mensagem quando cancela abre para ENTRADA (aguardar)
void mostrarLCDAguardarEntrada() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Entrada autoriz.");
  lcd.setCursor(0, 1);
  lcd.print("Aguarde cancela");
}

// Mensagem de Boas Festas (creativa mas 16 chars/linha)
void mostrarLCDMensagemFestas() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("IP:192.168.4.1");
  lcd.setCursor(0, 1);
  lcd.print("Pass:12345678");
}

// Gestão de alternancia 5s entre vagas/parque cheio e Boas Festas
void atualizarLCDIdle() {
  unsigned long agora = millis();

  // Alternar de 5 em 5 segundos
  if (agora >= proximaMudancaMensagem) {
    mostrarMensagemFestas = !mostrarMensagemFestas;
    proximaMudancaMensagem = agora + 5000;
  }

  if (mostrarMensagemFestas) {
    // Só reescreve se mudou para modo "festas"
    if (!ultimaEraFestas) {
      mostrarLCDMensagemFestas();
      ultimaEraFestas = true;
    }
  } else {
    // Modo normal: mostrar vagas / parque cheio
    if (ultimaEraFestas || vagasLivresAtual != ultimoVagasMostradas) {
      mostrarLCDVagas();
      ultimaEraFestas = false;
      ultimoVagasMostradas = vagasLivresAtual;
    }
  }
}

// Piscar LED amarelo enquanto mexe a cancela
void moverCancelaComPisca(int anguloInicial, int anguloFinal) {
  int passo = (anguloFinal > anguloInicial) ? 2 : -2;

  for (int ang = anguloInicial; ang != anguloFinal; ang += passo) {
    cancela.write(ang);
    // Piscar LED
    digitalWrite(LED_AMARELO_PIN, HIGH);
    delay(60);
    digitalWrite(LED_AMARELO_PIN, LOW);
    delay(60);
  }
  // garantir ângulo final
  cancela.write(anguloFinal);
}

// Esperar que o carro passe o sensor da rua
void esperarCarroPassarRua() {
  bool viuPerto = false;
  unsigned long inicio = millis();
  const unsigned long TIMEOUT_MS = 8000; // 8s, ajusta se quiseres

  while (millis() - inicio < TIMEOUT_MS) {
    float dist = lerDistanciaRua();

    if (carroNaRua(dist)) {
      viuPerto = true; // já esteve perto
    }

    // Quando já esteve perto e depois fica longe, consideramos que passou
    if (viuPerto && dist > DISTANCIA_CM) {
      break;
    }

    delay(100);
  }
}

// Sequência de entrada (carro vem da rua, há vaga)
void sequenciaEntrada() {
  cancelaOcupada = true;

  // Cancela abre (mensagem de "aguarde" já foi mostrada antes de chamar esta função)
  moverCancelaComPisca(ANGULO_FECHADO, ANGULO_ABERTO);

  // Esperar que o carro passe
  esperarCarroPassarRua();

  // Cancela fecha
  moverCancelaComPisca(ANGULO_ABERTO, ANGULO_FECHADO);

  cancelaOcupada = false;
}

// Sequência de saída (vaga ficou livre → carro vai sair)
void sequenciaSaida() {
  cancelaOcupada = true;

  // LCD a dizer STOP enquanto o carro sai
  mostrarLCDSaidaStop();

  // Abrir cancela
  moverCancelaComPisca(ANGULO_FECHADO, ANGULO_ABERTO);

  // Esperar carro passar pelo sensor da rua
  esperarCarroPassarRua();

  // Fechar cancela
  moverCancelaComPisca(ANGULO_ABERTO, ANGULO_FECHADO);

  cancelaOcupada = false;
}

// ---------- Setup e loop ----------

void setup() {
  Wire.begin(); // Master

  lcd.begin(16, 2);

  pinMode(TRIG_RUA, OUTPUT);
  pinMode(ECHO_RUA, INPUT);

  pinMode(LED_AMARELO_PIN, OUTPUT);
  digitalWrite(LED_AMARELO_PIN, LOW);

  cancela.attach(SERVO_PIN);
  cancela.write(ANGULO_FECHADO); // começa fechada

  // Serial para comunicação com ESP32 (e debug)
  Serial.begin(9600);

  // Mostrar algo inicial
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Init Parque...");
  lcd.setCursor(0, 1);
  lcd.print("A preparar vaga");
  delay(1000);

  // Iniciar timer de alternancia de mensagens
  proximaMudancaMensagem = millis() + 5000;
}

void loop() {
  // 1) Atualizar estado do parque vindo do Arduino 1
  atualizarEstadoParque();

  // 2) NOVO: Enviar dados ao ESP32
  enviarDadosESP32();

  // 3) Detectar saída (quando aumentam vagas livres)
  bool haSaida = false;
  if (vagasLivresAnterior >= 0 && vagasLivresAtual > vagasLivresAnterior) {
    haSaida = true;
  }
  vagasLivresAnterior = vagasLivresAtual;

  // 4) Ler sensor da rua
  float distRua = lerDistanciaRua();
  bool carroRua = carroNaRua(distRua);

  // 5) Se a cancela não está a meio de uma manobra, decidir o que fazer
  if (!cancelaOcupada) {

    if (haSaida) {
      // Um carro saiu de uma vaga → sequência de saída
      sequenciaSaida();
      // No fim, volta a mostrar vagas (retoma ciclo normal)
      mostrarLCDVagas();
    } else {
      // Nenhuma saída nova, ver entradas / estado

      if (vagasLivresAtual == 0) {
        // Parque cheio
        if (carroRua) {
          // Carro na rua e parque cheio → avisar e não abrir
          mostrarLCDParqueCheio();
        } else {
          // Parque cheio mas sem carro na rua → alterna entre "cheio" e Boas Festas
          atualizarLCDIdle();
        }
      } else {
        // Ainda há vagas
        if (carroRua) {
          // Carro à porta e há vaga → entrada
          mostrarLCDAguardarEntrada();  // pede para aguardar enquanto cancela abre
          sequenciaEntrada();
          // Depois da entrada, volta ao ciclo normal de mensagens
          mostrarLCDVagas();
        } else {
          // Ninguém na rua, só mostrar estado normal + Boas Festas a rodar
          atualizarLCDIdle();
        }
      }
    }
  }

  delay(200);
}
