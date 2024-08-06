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

//Bomba

byte bomba = 4;

// WiFi
const char *ssid = "Oi EAE6";           // Enter your WiFi name
const char *password = "5rN7BswLfj";  // Enter WiFi password

// MQTT Broker
const char *mqtt_broker = "broker.emqx.io";

const char *topic1 = "/umidade_ar";
const char *topic2 = "/umidade_solo1";
const char *topic3 = "/umidade_solo2";
const char *topic4 = "/umidade_solo3";
const char *topic5 = "/umidade_solo4";
const char *topic6 = "/temperatura";

const char *mqtt_username = "gardenbot";
const char *mqtt_password = "Carro.2005";
const int mqtt_port = 1883;


float temp, humi;

int solo1Value, solo2Value, solo3Value, solo4Value, solo1Porcentagem, solo2Porcentagem, solo3Porcentagem, solo4Porcentagem;


WiFiClient espClient;
PubSubClient client(espClient);

DHT dht(15, DHT22);

SemaphoreHandle_t xSemaphore;
SemaphoreHandle_t semaforoBomba;

void vaso1Task(void *parameter){
  
  bool bomba_ativada = false;
  
  while (1) {
    solo1Value = analogRead(solo1);
    solo1Porcentagem = map(solo1Value, 0, 4095, 100, 0);
  
    if (solo1Porcentagem <= 60){
      if(xSemaphoreTake(semaforoBomba, (TickType_t)10) == pdTRUE){
        vTaskDelay(5000);
        Serial.println("Vaso 1");
        digitalWrite(bomba, HIGH);
        xSemaphoreGive(semaforoBomba); 
      }
    }
    else{
      digitalWrite(bomba, LOW);
      bomba_ativada = false;
    }
    if (bomba_ativada && solo1Porcentagem == 80) {
      break;
    }
    vTaskDelay(100000);
  }
}

void vaso2Task(void *parameter){

  bool bomba_ativada = false;
  
  while (1){
    solo2Value = analogRead(solo2);
    solo2Porcentagem = map(solo2Value, 0, 4095, 100, 0);
  
    if (solo2Porcentagem <= 60){
      if (xSemaphoreTake(semaforoBomba, (TickType_t)1) == pdTRUE){
        vTaskDelay(5000);
        Serial.println("Vaso 2");
        digitalWrite(bomba, HIGH);
        xSemaphoreGive(semaforoBomba); 
      }
    }
    else{
      digitalWrite(bomba, LOW);
      bomba_ativada = false;
    }
    if (bomba_ativada && solo1Porcentagem == 80) {
      break;
    }
    vTaskDelay(100000);
  }
}

void vaso3Task(void *parameter){

  bool bomba_ativada = false;
  
  while(1){
    solo3Value = analogRead(solo3);
    solo3Porcentagem = map(solo3Value, 0, 4095, 100, 0);
  
    if (solo3Porcentagem <= 60){
      if (xSemaphoreTake(semaforoBomba, (TickType_t)1) == pdTRUE){
        vTaskDelay(5000);
        Serial.println("Vaso 3");
        digitalWrite(bomba, HIGH);
        xSemaphoreGive(semaforoBomba); 
      }
    }
    else{
      digitalWrite(bomba, LOW);
      bomba_ativada = false;
    }
    if (bomba_ativada && solo1Porcentagem == 80) {
      break;
    }
    vTaskDelay(100000);
  }
}

void vaso4Task(void *parameter){

  bool bomba_ativada = false;
  
  while(1){
    solo4Value = analogRead(solo4);
    solo4Porcentagem = map(solo4Value, 0, 4095, 100, 0);
  
    if (solo4Porcentagem <= 60){
      if (xSemaphoreTake(semaforoBomba, (TickType_t)1) == pdTRUE){
        vTaskDelay(5000);
        Serial.println("Vaso 4");
        digitalWrite(bomba, HIGH);
        xSemaphoreGive(semaforoBomba); 
      }
    }
    else{
      digitalWrite(bomba, LOW);
      bomba_ativada = false;
    }
    if (bomba_ativada && solo1Porcentagem == 80) {
      break;
    }
    vTaskDelay(100000);
  }
}

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
  pinMode(bomba, OUTPUT);

  dht.begin();

  xSemaphore = xSemaphoreCreateMutex();
  semaforoBomba = xSemaphoreCreateMutex();

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
    vaso1Task,
    "Vaso1Task",
    10000,
    NULL,
    1,
    NULL,
    1);

  xTaskCreatePinnedToCore(
    vaso2Task,
    "Vaso2Task",
    10000,
    NULL,
    1,
    NULL,
    1);

  xTaskCreatePinnedToCore(
    vaso3Task,
    "Vaso3Task",
    10000,
    NULL,
    1,
    NULL,
    1);

  xTaskCreatePinnedToCore(
    vaso4Task,
    "Vaso4Task",
    10000,
    NULL,
    1,
    NULL,
    1);
}

void loop() {
  vTaskSuspend(NULL);
}
