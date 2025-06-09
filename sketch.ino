#include <WiFi.h>
#include <PubSubClient.h>
#include "DHTesp.h"
#include <LiquidCrystal_I2C.h>

// Configurações do DHT
const int DHT_PIN = 9;
DHTesp dhtSensor;

// Configurações do LCD (I2C)
LiquidCrystal_I2C lcd(0x27, 16, 2); // Endereço 0x27 ou 0x3F

// Pinos dos dispositivos
#define LED_TEMP 5      // LED vermelho (temperatura alta)
#define LED_HUMID 18    // LED azul (umidade alta) - GPIO18 no ESP32
#define BUZZER_PIN 19   // Buzzer (ativo)
#define RAIN_SENSOR_PIN 34 // Pino analógico para potenciômetro (simula chuva)

// Limites para alertas
const float TEMP_ALERT = 30.0;   // Acima de 30°C
const float HUMID_ALERT = 70.0;  // Acima de 70%
const int RAIN_ALERT = 1500;     // Abaixo disso = chuva (potenciômetro)

// WiFi e MQTT
const char* ssid = "Wokwi-GUEST";
const char* password = "";
const char* mqtt_server = "broker.hivemq.com";

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Conectando a ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi conectado");
  Serial.println("Endereço IP: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  // Função para receber mensagens MQTT (opcional)
  Serial.print("Mensagem recebida: [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Conectando ao MQTT...");
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    
    if (client.connect(clientId.c_str())) {
      Serial.println("Conectado");
      client.subscribe("iotfrontier/commands");
    } else {
      Serial.print("Falha, rc=");
      Serial.print(client.state());
      Serial.println(" Tentando novamente em 5s");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  
  // Inicializa dispositivos
  pinMode(LED_TEMP, OUTPUT);
  pinMode(LED_HUMID, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RAIN_SENSOR_PIN, INPUT);
  
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Iniciando...");
  
  dhtSensor.setup(DHT_PIN, DHTesp::DHT22);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void controlAlerts(float temp, float humid, bool isRaining) {
  // LEDs
  digitalWrite(LED_TEMP, temp > TEMP_ALERT ? HIGH : LOW);
  digitalWrite(LED_HUMID, humid > HUMID_ALERT ? HIGH : LOW);
  
  // Buzzer (aciona para chuva OU temperatura crítica)
  if (isRaining || temp > TEMP_ALERT) {
    tone(BUZZER_PIN, 1000, 500); // 1kHz por 500ms
  } else {
    noTone(BUZZER_PIN);
  }
}

void publishSensorData(float temp, float humid, int rainValue) {
  client.publish("iotfrontier/temperature", String(temp).c_str());
  client.publish("iotfrontier/humidity", String(humid).c_str());
  client.publish("iotfrontier/rain", String(rainValue).c_str());
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  if (millis() - lastMsg > 2000) {
    lastMsg = millis();
    
    // Lê sensores
    TempAndHumidity data = dhtSensor.getTempAndHumidity();
    int rainValue = analogRead(RAIN_SENSOR_PIN);
    bool isRaining = rainValue < RAIN_ALERT;

    // Exibe no LCD
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Temp: " + String(data.temperature, 1) + "C");
    lcd.setCursor(0, 1);
    lcd.print("Umid: " + String(data.humidity, 1) + "%");
    lcd.setCursor(12, 1);
    lcd.print(isRaining ? "Chuva!" : "Seco");

    // Ativa alertas
    controlAlerts(data.temperature, data.humidity, isRaining);

    // Publica no MQTT
    publishSensorData(data.temperature, data.humidity, rainValue);

    // Log no Serial Monitor
    Serial.print("Temp: ");
    Serial.print(data.temperature);
    Serial.print("C | Umid: ");
    Serial.print(data.humidity);
    Serial.print("% | Chuva: ");
    Serial.println(rainValue);
  }
}