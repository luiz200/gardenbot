#include <WiFi.h>
#include <PubSubClient.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

byte led = 33;
byte led2 = 2;
byte ldr = 35;
byte temp = 34;
byte cooler = 25;

// WiFi
const char *ssid = "moto";           // Enter your WiFi name
const char *password = "123456789";  // Enter WiFi password

// MQTT Broker
const char *mqtt_broker = "broker.emqx.io";
const char *topic2 = "/luminosidade";
const char *topic3 = "/temperatura";
const char *mqtt_username = "teste";
const char *mqtt_password = "12345";
const int mqtt_port = 1883;
int ldrValue, tempValue, ldrPercentage;
float temperature;

String ldrPercentageStr, temperatureStr;

const int termistorPin = temp;         // Pino analógico onde o termistor está conectado
const float termistorNominal = 10000;  // Resistência nominal do termistor
const float tempNominal = 25.0;        // Temperatura nominal do termistor em graus Celsius
const float beta = 3950;

WiFiClient espClient;
PubSubClient client(espClient);

SemaphoreHandle_t xSemaphore;

float convertToTemperature(int termistorValue) {
  const float seriesResistor = 10000.0;
  const float termistorNominal = 10000.0;
  const float termistorCoefficient = 3950.0;
  const float termistorTemperature = 25.0;

  float voltage = termistorValue * (3.3 / 4095.0);
  float resistance = (3.3 * seriesResistor / voltage) - seriesResistor;
  float temperature = (resistance - termistorNominal) / termistorCoefficient;
  temperature += termistorTemperature;

  return temperature;
}

void ledTask(void *parameter){
  while (1) {
    ldrValue = analogRead(ldr);
    tempValue = analogRead(temp);
    ldrPercentage = map(ldrValue, 0, 4095, 100, 0);
    temperature = convertToTemperature(tempValue);
    // receber do broker do topico temperatura
    if (ldrPercentage < 20) {
      digitalWrite(led, 1);
    } else {
      digitalWrite(led, 0);
    }
    if (temperature > 27) {
      dacWrite(cooler, 4095);
    } else {
      dacWrite(cooler, 0);
    }
  }
}

void connectWiFiTask(void *parameter) {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {

    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");
  vTaskDelete(NULL);
}

void mqttTask(void *parameter) {
  client.setServer(mqtt_broker, mqtt_port);
  while (!client.connected()) {
    String client_id = "esp32-client-";
    client_id += String(WiFi.macAddress());
    Serial.printf("The client %s connects to the public mqtt broker\n", client_id.c_str());

    if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("Public emqx mqtt broker connected");
    } else {
      Serial.print("failed with state ");
      Serial.print(client.state());
    }
  }
  
  while (1) {
    client.loop();
      // publish and subscribe
      ldrValue = analogRead(ldr);
      tempValue = analogRead(temp);
      ldrPercentage = map(ldrValue, 0, 4095, 100, 0);
      temperature = convertToTemperature(tempValue);
      ldrPercentageStr = String(ldrPercentage);
      temperatureStr = String(temperature);
      client.publish(topic2, ldrPercentageStr.c_str());
      Serial.println(ldrPercentageStr.c_str());
      client.publish(topic3, temperatureStr.c_str());
      Serial.println(temperatureStr.c_str());
      vTaskDelay(1000);
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(led, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(ldr, INPUT);
  pinMode(temp, INPUT);
  pinMode(cooler, OUTPUT);

  xSemaphore = xSemaphoreCreateMutex();

  xTaskCreatePinnedToCore(
    connectWiFiTask,
    "ConnectWiFiTask",
    10000,
    NULL,
    1,
    NULL,
    0);

  xTaskCreatePinnedToCore(
    mqttTask,
    "MqttTask",
    10000,
    NULL,
    1,
    NULL,
    0);
  xTaskCreatePinnedToCore(
    ledTask,
    "LedTask",
    10000,
    NULL,
    1,
    NULL,
    1);
}

void loop() {
  vTaskSuspend(NULL);
}