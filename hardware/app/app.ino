#include <WiFi.h>
#include <PubSubClient.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <DHT.h>
#include <AccelStepper.h>

//Sensores de umidade solo
byte solo1 = 32;
byte solo2 = 33;
byte solo3 = 34;
byte solo4 = 35;

//Motor de passo 1
const int stepPin1 = 2;
const int dirPin1 = 3;

//Motor de passo 2
const int stepPin2 = 4;
const int dirPin2 = 5;

// Definindo os dois motores no modo DRIVER
AccelStepper motor1(AccelStepper::DRIVER, stepPin1, dirPin1);
AccelStepper motor2(AccelStepper::DRIVER, stepPin2, dirPin2);

// Posições (em passos) para cada vaso
long posicoes[4][2] = {
  {0, 0}, //vaso1
  {400, 200}, //vaso2
  {800, -200}, //vaso3
  {1200, 0} //vaso4
};

void moverParaPosicao(int index){
  motor1.moveTo(posicoes[index][0]);
  motor2.moveTo(posicoes[index][1]);

  while (motor1.isRunning() || motor2.isRunning()){
    motor1.run();
    motor2.run();
    delay(1);
  }
}

//Bomba
byte bomba = 4;

// WiFi
const char *ssid = "...";           // Enter your WiFi name
const char *password = "...";  // Enter WiFi password

// MQTT Broker
const char *mqtt_broker = "broker.emqx.io";
const char *mqtt_username = "";
const char *mqtt_password = "";
const int mqtt_port = 1883;

//Tópicos
const char *topic1 = "/umidade_ar";
const char *topic2 = "/umidade_solo1";
const char *topic3 = "/umidade_solo2";
const char *topic4 = "/umidade_solo3";
const char *topic5 = "/umidade_solo4";
const char *topic6 = "/temperatura";

float temp, humi;

int solo1Value, solo2Value, solo3Value, solo4Value, solo1Porcentagem, solo2Porcentagem, solo3Porcentagem, solo4Porcentagem;

DHT dht(15, DHT22);

WiFiClient espClient;
PubSubClient client(espClient);


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
        
        moverParaPosicao(0);
        digitalWrite(bomba, HIGH);
        bomba_ativada = true;
        xSemaphoreGive(semaforoBomba); 
      }
    }
    else{
      digitalWrite(bomba, LOW);
      bomba_ativada = false;
    }
    if (bomba_ativada && solo1Porcentagem == 80) {
      digitalWrite(bomba, LOW); // desliga bomba
      moverParaPosicao(0);      // volta à posição inicial
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
        
        moverParaPosicao(1);
        digitalWrite(bomba, HIGH);
        bomba_ativada = true;
        xSemaphoreGive(semaforoBomba); 
      }
    }
    else{
      digitalWrite(bomba, LOW);
      bomba_ativada = false;
    }
    if (bomba_ativada && solo1Porcentagem == 80) {
      digitalWrite(bomba, LOW); // desliga bomba
      moverParaPosicao(0);      // volta à posição inicial
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
        
        moverParaPosicao(2);
        digitalWrite(bomba, HIGH);
        bomba_ativada = true;
        xSemaphoreGive(semaforoBomba); 
      }
    }
    else{
      digitalWrite(bomba, LOW);
      bomba_ativada = false;
    }
    if (bomba_ativada && solo1Porcentagem == 80) {
      digitalWrite(bomba, LOW); // desliga bomba
      moverParaPosicao(0);      // volta à posição inicial
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
        
        moverParaPosicao(3);
        digitalWrite(bomba, HIGH);
        bomba_ativada = true;
        xSemaphoreGive(semaforoBomba); 
      }
    }
    else{
      digitalWrite(bomba, LOW);
      bomba_ativada = false;
    }
    if (bomba_ativada && solo1Porcentagem == 80) {
      digitalWrite(bomba, LOW); // desliga bomba
      moverParaPosicao(0);      // volta à posição inicial
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
      

      vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void setup() {
  Serial.begin(115200);

  //Configurações do motor 1
  motor1.setMaxSpeed(200);
  motor1.setAcceleration(100);

  // Configurações do motor 2
  motor2.setMaxSpeed(200);
  motor2.setAcceleration(100);

  pinMode(stepPin1, OUTPUT);
  pinMode(dirPin1, OUTPUT);
  pinMode(stepPin2, OUTPUT);
  pinMode(dirPin2, OUTPUT);
  
  pinMode(solo1, INPUT);
  pinMode(solo2, INPUT);
  pinMode(solo3, INPUT);
  pinMode(solo4, INPUT);
  pinMode(bomba, OUTPUT);

  dht.begin();

  xSemaphore = xSemaphoreCreateMutex();
  semaforoBomba = xSemaphoreCreateMutex();

  xTaskCreatePinnedToCore(connectWiFiTask, "ConnectWiFiTask", 10000, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(mqttTask, "MqttTask", 10000, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(vaso1Task, "VasoTask1", 10000, (void *)&solo1, 1, NULL, 1);
  xTaskCreatePinnedToCore(vaso2Task, "VasoTask2", 10000, (void *)&solo2, 1, NULL, 1);
  xTaskCreatePinnedToCore(vaso3Task, "VasoTask3", 10000, (void *)&solo3, 1, NULL, 1);
  xTaskCreatePinnedToCore(vaso4Task, "VasoTask4", 10000, (void *)&solo4, 1, NULL, 1);
  
}

void loop() {
  vTaskSuspend(NULL);
}
