# 🅿️ CityFlow - Smart Parking System

<div align="center">

![CityFlow Banner](https://img.shields.io/badge/CityFlow-Smart%20Parking-38bdf8?style=for-the-badge&logo=car&logoColor=white)

[![Arduino](https://img.shields.io/badge/Arduino-00979D?style=flat-square&logo=Arduino&logoColor=white)](https://www.arduino.cc/)
[![ESP8266](https://img.shields.io/badge/ESP8266-E7352C?style=flat-square&logo=espressif&logoColor=white)](https://www.espressif.com/)
[![License](https://img.shields.io/badge/License-MIT-green?style=flat-square)](LICENSE)

**Sistema inteligente de gestão de estacionamento com monitorização em tempo real via Web App**

[Funcionalidades](#-funcionalidades) •
[Arquitetura](#-arquitetura) •
[Componentes](#-componentes) •
[Instalação](#-instalação) •
[Utilização](#-utilização)

</div>

---

## 📋 Descrição

O **CityFlow** é um sistema de estacionamento inteligente que permite monitorizar a ocupação de vagas em tempo real através de uma aplicação web acessível por telemóvel. O sistema utiliza sensores ultrassónicos para detetar a presença de veículos, LEDs para indicação visual local, uma cancela automática controlada por servo motor, e comunicação WiFi para disponibilizar os dados numa interface web moderna.

### 🎯 Objetivos

- Monitorização em tempo real de 3 lugares de estacionamento
- Indicação visual local com LEDs (verde = livre, vermelho = ocupado)
- Cancela automática para entrada e saída de veículos
- Display LCD com informação de vagas disponíveis
- Web App responsiva acessível via WiFi
- Interface moderna com estatísticas e mapa visual das vagas

---

## ✨ Funcionalidades

| Funcionalidade | Descrição |
|----------------|-----------|
| 🚗 **Deteção de Veículos** | Sensores ultrassónicos HC-SR04 para cada vaga |
| 💡 **LEDs Indicadores** | Verde (livre) / Vermelho (ocupado) por vaga |
| 🚧 **Cancela Automática** | Servo motor SG90 com abertura/fecho automático |
| 📺 **Display LCD** | Informação de vagas livres e IP de acesso |
| 📱 **Web App** | Interface responsiva com atualização em tempo real |
| 📊 **Estatísticas** | Vagas livres, ocupadas e percentagem de ocupação |
| 🗺️ **Mapa Visual** | Representação gráfica do estado das vagas |

---

## 🏗️ Arquitetura

O sistema é composto por 3 microcontroladores que comunicam entre si:

```
┌─────────────────┐         I2C          ┌─────────────────┐        Serial        ┌─────────────────┐
│                 │ ◄──────────────────► │                 │ ◄──────────────────► │                 │
│   ARDUINO 1     │                      │   ARDUINO 2     │                      │    ESP8266      │
│    (Slave)      │                      │    (Master)     │                      │   (WiFi AP)     │
│                 │                      │                 │                      │                 │
│ • 3x Sensores   │                      │ • LCD 16x2      │                      │ • Access Point  │
│ • 6x LEDs       │                      │ • Servo Motor   │                      │ • Web Server    │
│                 │                      │ • Sensor Rua    │                      │ • API REST      │
│                 │                      │ • LED Amarelo   │                      │                 │
└─────────────────┘                      └─────────────────┘                      └─────────────────┘
                                                                                           │
                                                                                           ▼
                                                                                    ┌─────────────┐
                                                                                    │  📱 Web App │
                                                                                    │ 192.168.4.1 │
                                                                                    └─────────────┘
```

### Fluxo de Dados

1. **Arduino 1** lê os sensores das vagas e atualiza os LEDs
2. **Arduino 2** solicita dados via I2C e controla LCD/cancela
3. **Arduino 2** envia estado via Serial para o **ESP8266**
4. **ESP8266** serve a Web App com dados em tempo real

---

## 🔧 Componentes

### Lista de Materiais

| Qtd | Componente | Descrição |
|:---:|------------|-----------|
| 2 | Arduino Uno | Microcontroladores principais |
| 1 | ESP8266 NodeMCU | Módulo WiFi para web server |
| 4 | HC-SR04 | Sensores ultrassónicos (3 vagas + 1 rua) |
| 3 | LED Verde 5mm | Indicador vaga livre |
| 3 | LED Vermelho 5mm | Indicador vaga ocupada |
| 1 | LED Amarelo 5mm | Indicador cancela em movimento |
| 1 | LCD 16x2 | Display com interface I2C ou 4-bit |
| 1 | Servo SG90 | Motor para cancela |
| 7 | Resistência 220Ω | Para LEDs |
| 1 | Resistência 1kΩ | Divisor de tensão TX |
| 1 | Resistência 2kΩ | Divisor de tensão TX |
| - | Jumper Wires | Fios de ligação |
| - | Breadboard | Placa de prototipagem |
| 1 | Fonte 5V | Alimentação externa (recomendado) |

---

## 📌 Esquema de Ligações

### Arduino 1 (Slave) - Vagas

| Componente | Pino Arduino |
|------------|:------------:|
| **Sensor Vaga 1** | |
| TRIG | D2 |
| ECHO | D3 |
| **Sensor Vaga 2** | |
| TRIG | D4 |
| ECHO | D5 |
| **Sensor Vaga 3** | |
| TRIG | D6 |
| ECHO | D7 |
| **LEDs Vaga 1** | |
| Vermelho | D8 |
| Verde | D9 |
| **LEDs Vaga 2** | |
| Vermelho | D10 |
| Verde | D11 |
| **LEDs Vaga 3** | |
| Vermelho | D12 |
| Verde | D13 |
| **I2C** | |
| SDA | A4 |
| SCL | A5 |

### Arduino 2 (Master) - Cancela/LCD

| Componente | Pino Arduino |
|------------|:------------:|
| **LCD 16x2** | |
| RS | D12 |
| EN | D11 |
| D4 | D5 |
| D5 | D4 |
| D6 | D3 |
| D7 | D2 |
| **Servo (Cancela)** | |
| Sinal | D9 |
| **Sensor Rua** | |
| TRIG | D6 |
| ECHO | D7 |
| **LED Amarelo** | D8 |
| **I2C** | |
| SDA | A4 |
| SCL | A5 |
| **Serial → ESP8266** | |
| TX | Pin 1 |
| RX | Pin 0 |

### ESP8266 NodeMCU

| Componente | Pino ESP8266 |
|------------|:------------:|
| **Serial (Arduino 2)** | |
| RX (via divisor) | D5 (GPIO14) |
| TX | D6 (GPIO12) |

### ⚠️ Divisor de Tensão (OBRIGATÓRIO)

O Arduino opera a 5V e o ESP8266 a 3.3V. É **obrigatório** usar um divisor de tensão na linha TX do Arduino → RX do ESP8266:

```
Arduino 2 TX (5V) ────[1kΩ]────┬────► ESP8266 D5 (3.3V)
                               │
                            [2kΩ]
                               │
                              GND
```

---

## 💻 Instalação

### 1. Clonar o Repositório

```bash
git clone https://github.com/seu-username/CityFlow.git
cd CityFlow
```

### 2. Estrutura de Ficheiros

```
CityFlow/
├── README.md
├── LICENSE
├── arduino/
│   ├── CityFlow_Arduino1/
│   │   └── CityFlow_Arduino1.ino
│   ├── CityFlow_Arduino2/
│   │   └── CityFlow_Arduino2.ino
│   └── CityFlow_ESP8266/
│       └── CityFlow_ESP8266.ino
├── docs/
│   ├── esquema_ligacoes.png
│   └── arquitetura.png
└── webapp/
    └── preview.html
```

### 3. Configurar Arduino IDE

#### Instalar Bibliotecas Necessárias

- **Wire** (incluída por defeito)
- **LiquidCrystal** (incluída por defeito)
- **Servo** (incluída por defeito)
- **ESP8266WiFi** (via Board Manager)
- **ESP8266WebServer** (via Board Manager)

#### Adicionar Suporte ESP8266

1. Abrir **Arduino IDE** → **File** → **Preferences**
2. Em "Additional Board Manager URLs" adicionar:
   ```
   http://arduino.esp8266.com/stable/package_esp8266com_index.json
   ```
3. Ir a **Tools** → **Board** → **Board Manager**
4. Pesquisar "ESP8266" e instalar

### 4. Upload dos Códigos

#### Arduino 1 (Slave)
1. Selecionar **Board**: `Arduino Uno`
2. Selecionar **Port**: (porta do Arduino 1)
3. Abrir `CityFlow_Arduino1.ino`
4. Clicar em **Upload**

#### Arduino 2 (Master)
1. ⚠️ **Desconectar fios TX/RX** antes do upload
2. Selecionar **Board**: `Arduino Uno`
3. Selecionar **Port**: (porta do Arduino 2)
4. Abrir `CityFlow_Arduino2.ino`
5. Clicar em **Upload**
6. **Reconectar fios TX/RX** após upload

#### ESP8266
1. Selecionar **Board**: `NodeMCU 1.0 (ESP-12E Module)`
2. Selecionar **Upload Speed**: `115200`
3. Selecionar **Port**: (porta do ESP8266)
4. Abrir `CityFlow_ESP8266.ino`
5. Clicar em **Upload**

---

## 📱 Utilização

### Ligar ao Sistema

1. **Alimentar** todos os componentes (Arduino 1, Arduino 2, ESP8266)
2. No telemóvel, ir a **Definições WiFi**
3. Ligar à rede: **`CityFlow`**
4. Password: **`12345678`**
5. Abrir browser e aceder a: **`http://192.168.4.1`**

### Interface Web

A Web App apresenta duas páginas principais:

#### 🏠 Página Início
- Saudação ao utilizador
- Card do estacionamento com:
  - Estado (ABERTO/CHEIO)
  - Estatísticas (Livres, Ocupadas, % Ocupação)
  - Barra de progresso visual
  - Mapa de vagas em tempo real
- Card de estacionamento futuro (Em Breve)

#### 👤 Página Perfil
- Informações do utilizador
- Estatísticas de utilização
- Definições (Dados Pessoais, Notificações, Idioma)

### Display LCD

O LCD alterna entre duas mensagens a cada 5 segundos:

```
┌────────────────┐        ┌────────────────┐
│Lugares vagos:  │  ↔ 5s  │IP:192.168.4.1  │
│3               │        │Pass:12345678   │
└────────────────┘        └────────────────┘
```

---

## 🔌 API REST

O ESP8266 disponibiliza um endpoint API para obter o estado das vagas:

### GET /api/vagas

**Resposta:**
```json
{
  "vagas": [false, true, false]
}
```

- `false` = Vaga livre
- `true` = Vaga ocupada

**Exemplo com curl:**
```bash
curl http://192.168.4.1/api/vagas
```

---

## 🖼️ Screenshots

<div align="center">

| Página Início | Página Perfil |
|:-------------:|:-------------:|
| ![Home](docs/screenshot_home.png) | ![Profile](docs/screenshot_profile.png) |

</div>

---

## 🔧 Resolução de Problemas

| Problema | Solução |
|----------|---------|
| LCD não mostra texto | Verificar ligações e ajustar contraste (potenciómetro) |
| ESP8266 não cria rede WiFi | Verificar alimentação (usar fonte externa 5V) |
| Vagas não atualizam na app | Verificar divisor de tensão e ligações Serial |
| Servo não mexe | Verificar alimentação e pino D9 |
| Upload falha no Arduino 2 | Desconectar fios TX/RX durante upload |
| Sensores dão leituras erradas | Verificar distância mínima (2cm) e máxima (400cm) |

---

## 🚀 Melhorias Futuras

- [ ] Adicionar mais estacionamentos
- [ ] Sistema de reserva de vagas
- [ ] Histórico de ocupação
- [ ] Notificações push
- [ ] Integração com Google Maps
- [ ] Pagamento integrado
- [ ] Sensores de peso como backup
- [ ] Câmaras com reconhecimento de matrículas

---

## 📄 Licença

Este projeto está licenciado sob a **MIT License** - veja o ficheiro [LICENSE](LICENSE) para detalhes.

---

## 👥 Autores

- **Gonçalo Alegria** - *Desenvolvimento* - [@goncaloalegria](https://github.com/goncaloalegria)

---

## 🙏 Agradecimentos

- [Arduino](https://www.arduino.cc/) - Plataforma de desenvolvimento
- [ESP8266 Community](https://github.com/esp8266/Arduino) - Suporte ESP8266
- [Universidade Lusófona](https://www.ulusofona.pt/) - Instituição de ensino

---

<div align="center">

**Feito com ❤️ para o projeto académico**

[![GitHub stars](https://img.shields.io/github/stars/seu-username/CityFlow?style=social)](https://github.com/goncaloalegria/CityFlow)

</div>