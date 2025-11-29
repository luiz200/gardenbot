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
byte solo4 = 39;

//Sensor de chuva
byte pino_d = 18;
byte pino_a = 36;

// WiFi
const char *ssid = "SIMONE";           // Enter your WiFi name
const char *password = "simone1234";  // Enter WiFi password

// MQTT Broker
const char *mqtt_broker = "broker.emqx.io";
const char *mqtt_username = "...";
const char *mqtt_password = "...";
const int mqtt_port = 1883;

//Tópicos
const char *topic1 = "/umidade_ar";
const char *topic2 = "/umidade_solo1";
const char *topic3 = "/umidade_solo2";
const char *topic4 = "/umidade_solo3";
const char *topic5 = "/umidade_solo4";
const char *topic6 = "/temperatura";
const char *topic7 = "/chuva";
const char *topic8 = "/intensidade_chuva";

float temp, humi;

int val_d, val_a, solo1Value, solo2Value, solo3Value, solo4Value, solo1Porcentagem, solo2Porcentagem, solo3Porcentagem, solo4Porcentagem;

DHT dht(15, DHT22);

WiFiClient espClient;
PubSubClient client(espClient);

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

      val_d = digitalRead(pino_d);
      val_a = analogRead(pino_a);
      
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
      client.publish(topic7, String(val_d).c_str());
      client.publish(topic8, String(val_a).c_str());
      

      vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void setup() {
  Serial.begin(115200);
  
  pinMode(solo1, INPUT);
  pinMode(solo2, INPUT);
  pinMode(solo3, INPUT);
  pinMode(solo4, INPUT);

  pinMode(pino_d, INPUT);
  pinMode(pino_a, INPUT);

  dht.begin();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");
  
  xTaskCreatePinnedToCore(mqttTask, "MqttTask", 10000, NULL, 1, NULL, 1);
  
}

void loop() {
  vTaskSuspend(NULL);
}
