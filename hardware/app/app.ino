#include <WiFi.h>
#include <PubSubClient.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <DHT.h>

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

const char *mqtt_username = "bot";
const char *mqtt_password = "12345";
const int mqtt_port = 1883;


float temp, humi;

int solo1Value, solo2Value, solo3Value, solo4Value, solo1Porcentagem, solo2Porcentagem, solo3Porcentagem, solo4Porcentagem;


WiFiClient espClient;
PubSubClient client(espClient);

DHT dht(4, DHT22);

SemaphoreHandle_t xSemaphore;

//void ledTask(void *parameter){
//  while (1) {
//    //ldrValue = analogRead(ldr);
//    tempValue = analogRead(temp);
//    //ldrPercentage = map(ldrValue, 0, 4095, 100, 0);
//    temperature = convertToTemperature(tempValue);
//    // receber do broker do topico temperatura
//    // if (ldrPercentage < 20) {
//    //   digitalWrite(led, 1);
//    // } else {
//    //   digitalWrite(led, 0);
//    // }
//    // if (temperature > 27) {
//    //   dacWrite(cooler, 4095);
//    // } else {
//    //   dacWrite(cooler, 0);
//    // }
//  }
//}

void connectWiFiTask(void *parameter) {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
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
      temp = dht.readTemperature();
      humi = dht.readHumidity();
      
      solo1Value = analogRead(solo1);
      solo2Value = analogRead(solo2);
      solo3Value = analogRead(solo3);
      solo4Value = analogRead(solo4);

      solo1Porcentagem = map(solo1Value, 0, 4095, 100, 0);
      solo2Porcentagem = map(solo2Value, 0, 4095, 100, 0);
      solo3Porcentagem = map(solo3Value, 0, 4095, 100, 0);
      solo4Porcentagem = map(solo4Value, 0, 4095, 100, 0);

     
      client.publish(topic1, String(humi).c_str());
      client.publish(topic2, String(solo1Porcentagem).c_str());
      client.publish(topic3, String(solo2Porcentagem).c_str());
      client.publish(topic4, String(solo3Porcentagem).c_str());
      client.publish(topic5, String(solo4Porcentagem).c_str());
      client.publish(topic6, String(temp).c_str());
      

      vTaskDelay(1000);
  }
}

void setup() {
  Serial.begin(115200);
  
  pinMode(solo1, INPUT);
  pinMode(solo2, INPUT);
  pinMode(solo3, INPUT);
  pinMode(solo4, INPUT);

  dht.begin();

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
}

void loop() {
  vTaskSuspend(NULL);
}
