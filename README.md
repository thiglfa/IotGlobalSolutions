# 🌱 Plataforma de Monitoramento Ambiental com Node-RED

Este projeto implementa uma solução IoT para monitoramento ambiental em tempo real, utilizando sensores conectados a dispositivos ESP32/Arduino, com comunicação via MQTT e visualização em um dashboard Node-RED. A plataforma oferece alertas visuais e sonoros para condições críticas, gráficos históricos e indicadores LED virtuais.

---

## 📸 Visão Geral


- Indicadores de temperatura, umidade e chuva
- Alertas visuais (LEDs virtuais) e sonoros em tempo real
- Gráficos de histórico por sensor
- Dashboard responsivo e acessível

---

## ⚙️ Arquitetura da Solução

```plaintext
[Dispositivos IoT (ESP32/Arduino)]
          |
       MQTT (HiveMQ)
          |
      [Node-RED Gateway]
          |
     [Dashboard UI - Node-RED]
```

- Cada dispositivo coleta dados (temperatura, umidade, chuva)
- Os dados são publicados nos tópicos MQTT:
  - `iotfrontier/temperature`
  - `iotfrontier/humidity`
  - `iotfrontier/rain`
- O Node-RED se conecta ao broker MQTT, processa os dados, verifica condições de alerta e os exibe no dashboard.

---

## 🚀 Como Executar o Projeto

### 1. Requisitos

- Node.js instalado
- Node-RED instalado globalmente (`npm install -g node-red`)
- Navegador web (recomendado: Chrome ou Firefox)
- Conexão com broker MQTT (utiliza-se o broker público HiveMQ)

### 2. Instalar e Iniciar o Node-RED

```bash
node-red
```

Abra o navegador e acesse: [http://localhost:1880](http://localhost:1880)

### 3. Importar o Fluxo

- Clique no menu (☰) > "Import"
- Cole o conteúdo do fluxo JSON (ver seção abaixo)
- Clique em "Deploy"

### 4. Acessar o Dashboard

Acesse [http://localhost:1880/ui](http://localhost:1880/ui)

### 5. Testes

- Use um simulador de publicação MQTT (ex: MQTT Explorer ou MQTTBox) ou dispositivos reais
- Ou mude os valores de temperatura, umidade e chuva diretamente pelo potenciometro e dht do através do Wokw.
- Publique valores nos tópicos:
  - `iotfrontier/temperature` com valores > 30°C para acionar alerta
  - `iotfrontier/humidity` com valores > 70%
  - `iotfrontier/rain` com valores < 1500 (indicando chuva)

---

## 🔁 Fluxo Node-RED Explicado

### Entradas MQTT

- Três nós MQTT (`iotfrontier/temperature`, `iotfrontier/humidity`, `iotfrontier/rain`)
- Recebem dados dos sensores e encaminham para:
  - Gráficos (`ui_chart`)
  - Medidores (`ui_gauge`)
  - Função `Verifica Alertas`

### Função `Verifica Alertas`

```js
const val = parseFloat(msg.payload);
const topic = msg.topic;

let alert = {};

if (topic.includes("temperature") && val > 30) {
    alert.sound = true;
    alert.led_temp = true;
} else if (topic.includes("temperature")) {
    alert.led_temp = false;
}

if (topic.includes("humidity") && val > 70) {
    alert.led_humid = true;
} else if (topic.includes("humidity")) {
    alert.led_humid = false;
}

if (topic.includes("rain") && val < 1500) {
    alert.sound = true;
    alert.led_rain = true;
} else if (topic.includes("rain")) {
    alert.led_rain = false;
}

return { payload: alert };
```

### Saídas

- `ui_template (LEDs)`: exibe três LEDs virtuais (vermelho, azul, verde)
- `ui_template (Som de Alerta)`: toca som se `alert.sound` for verdadeiro

---

## 🧪 Testando com Simulador MQTT

```bash
# Exemplo usando MQTT CLI
mqtt pub -t iotfrontier/temperature -h broker.hivemq.com -m "31"
mqtt pub -t iotfrontier/humidity -h broker.hivemq.com -m "75"
mqtt pub -t iotfrontier/rain -h broker.hivemq.com -m "1400"
```

---

## 📁 Código-Fonte

- O fluxo principal está no arquivo `node-red-flow.json`
- O código dos dispositivos IoT se encontra no arquivo sketch.ino
- Todos os recursos visuais estão embutidos nos nós `ui_template`
- O projeto não depende de bibliotecas externas no lado do servidor

---

## 👥 Acesso Externo ao Dashboard

Por padrão, o Node-RED roda localmente. Para permitir o acesso externo:

1. Verifique seu IP local (ex: `192.168.1.10`)
2. Compartilhe o link `http://192.168.1.10:1880/ui`
3. Certifique-se de que:
   - O firewall libera a porta 1880
   - Os dispositivos estão na mesma rede

> Para acesso remoto via internet, considere usar **Ngrok**, **port forwarding** ou hospedar em um servidor cloud.
