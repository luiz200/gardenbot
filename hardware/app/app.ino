#include <WiFi.h>
#include <PubSubClient.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

//Sensores de umidade solo

byte solo1 = 32;
byte solo2 = 33;
byte solo3 = 34;
byte solo4 = 35;

//Motores de passo

byte motor1 = 25;
byte motor2 = 26;
byte motor3 = 27;

//Eletroimã

byte ima1 = 12;
byte ima2 = 13;
byte ima3 = 14;
byte ima4 = 23;

//Bomba

byte bomba = 22;

//Micro switcher

byte switcher = 21;

//Temperatura

byte temp = 19;

//Umidade do ar

byte ar = 18;

// WiFi
const char *ssid = "Simone";           // Enter your WiFi name
const char *password = "simone234";  // Enter WiFi password

// MQTT Broker
const char *mqtt_broker = "broker.emqx.io";

const char *topic1 = "/umidade_ar";
const char *topic2 = "/umidade_solo1";
const char *topic3 = "/umidade_solo2";
const char *topic4 = "/umidade_solo3";
const char *topic5 = "/umidade_solo4";
const char *topic6 = "/temperatura";

const char *mqtt_username = "teste";
const char *mqtt_password = "12345";
const int mqtt_port = 1883;


float temperature;

int tempValue, solo1Value, solo2Value, solo3Value, solo4Value, solo1Porcentagem, solo2Porcentagem, solo3Porcentagem, solo4Porcentagem;

String temperatureStr, solo1PorcentagemStr, solo2PorcentagemStr, solo3PorcentagemStr, solo4PorcentagemStr;

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
    //ldrValue = analogRead(ldr);
    tempValue = analogRead(temp);
    //ldrPercentage = map(ldrValue, 0, 4095, 100, 0);
    temperature = convertToTemperature(tempValue);
    // receber do broker do topico temperatura
    // if (ldrPercentage < 20) {
    //   digitalWrite(led, 1);
    // } else {
    //   digitalWrite(led, 0);
    // }
    // if (temperature > 27) {
    //   dacWrite(cooler, 4095);
    // } else {
    //   dacWrite(cooler, 0);
    // }
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
      tempValue = analogRead(temp);
      solo1Value = analogRead(solo1);
      solo2Value = analogRead(solo2);
      solo3Value = analogRead(solo3);
      solo4Value = analogRead(solo4);

      solo1Porcentagem = map(solo1Value, 0, 4095, 100, 0);
      solo2Porcentagem = map(solo2Value, 0, 4095, 100, 0);
      solo3Porcentagem = map(solo3Value, 0, 4095, 100, 0);
      solo4Porcentagem = map(solo4Value, 0, 4095, 100, 0);

      temperature = convertToTemperature(tempValue);

      temperatureStr = String(temperature);
      solo1PorcentagemStr = String(solo1Porcentagem);
      solo2PorcentagemStr = String(solo2Porcentagem);
      solo3PorcentagemStr = String(solo3Porcentagem);
      solo4PorcentagemStr = String(solo4Porcentagem);
      
      client.publish(topic2, solo1PorcentagemStr.c_str());
      Serial.println(solo1PorcentagemStr.c_str());
      client.publish(topic3, solo2PorcentagemStr.c_str());
      Serial.println(solo2PorcentagemStr.c_str());
      client.publish(topic4, solo3PorcentagemStr.c_str());
      Serial.println(solo3PorcentagemStr.c_str());
      client.publish(topic5, solo4PorcentagemStr.c_str());
      Serial.println(solo4PorcentagemStr.c_str());
      client.publish(topic6, temperatureStr.c_str());
      Serial.println(temperatureStr.c_str());

      vTaskDelay(1000);
  }
}

void setup() {
  Serial.begin(115200);
  //pinMode(led, OUTPUT);
  //pinMode(led2, OUTPUT);
  //pinMode(ldr, INPUT);
  pinMode(solo1, INPUT);
  pinMode(solo2, INPUT);
  pinMode(solo3, INPUT);
  pinMode(solo4, INPUT);
  pinMode(temp, INPUT);
  //pinMode(cooler, OUTPUT);

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