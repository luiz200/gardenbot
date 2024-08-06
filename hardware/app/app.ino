#include <WiFi.h>
#include <PubSubClient.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <DHT.h>
#include <Fuzzy.h>

//Sensores de umidade solo
byte solo1 = 32;
byte solo2 = 33;
byte solo3 = 34;
byte solo4 = 35;

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

// Variáveis Fuzzy
Fuzzy *fuzzy = new Fuzzy();
FuzzySet *umidadeSeca = new FuzzySet(0, 0, 20, 40);
FuzzySet *umidadeModerada = new FuzzySet(30, 50, 70);
FuzzySet *umidadeUmida = new FuzzySet(60, 80, 100);

FuzzySet *irrigarNenhuma = new FuzzySet(0, 0, 10, 20);
FuzzySet *irrigarPouca = new FuzzySet(10, 30, 50);
FuzzySet *irrigarMuita = new FuzzySet(40, 60, 100);

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

void vasoTask(void *parameter) {
  byte sensorPin = *((byte *)parameter);

  while (1) {
    int umidadeSolo = analogRead(sensorPin);
    int umidadePercentual = map(umidadeSolo, 0, 4095, 100, 0);

    fuzzy->setInput(1, umidadePercentual);
    fuzzy->fuzzify();

    float intensidadeIrrigacao = fuzzy->defuzzify(1);

    if (xSemaphoreTake(semaforoBomba, (TickType_t)10) == pdTRUE) {
      if (intensidadeIrrigacao > 10) {  // Ajuste o limiar conforme necessário
        digitalWrite(bomba, HIGH);
        delay(intensidadeIrrigacao * 10);  // Ajuste a duração conforme necessário
        digitalWrite(bomba, LOW);
      }
      xSemaphoreGive(semaforoBomba);
    }

    vTaskDelay(pdMS_TO_TICKS(1000));
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

  // Configuração Fuzzy
  fuzzy->addFuzzyInput(new FuzzyInput(1));
  fuzzy->addFuzzyOutput(new FuzzyOutput(1));

  fuzzy->getFuzzyInput(1)->addFuzzySet(umidadeSeca);
  fuzzy->getFuzzyInput(1)->addFuzzySet(umidadeModerada);
  fuzzy->getFuzzyInput(1)->addFuzzySet(umidadeUmida);

  fuzzy->getFuzzyOutput(1)->addFuzzySet(irrigarNenhuma);
  fuzzy->getFuzzyOutput(1)->addFuzzySet(irrigarPouca);
  fuzzy->getFuzzyOutput(1)->addFuzzySet(irrigarMuita);

  fuzzy->addFuzzyRule(new FuzzyRule(1, FuzzyRuleAntecedent().joinWithOR(new FuzzyRuleCondition().setInput(1, umidadeSeca), new FuzzyRuleCondition().setInput(1, umidadeModerada)), new FuzzyRuleConsequent().addOutput(1, irrigarPouca)));
  fuzzy->addFuzzyRule(new FuzzyRule(2, FuzzyRuleAntecedent().joinWithOR(new FuzzyRuleCondition().setInput(1, umidadeUmida)), new FuzzyRuleConsequent().addOutput(1, irrigarNenhuma)));

  semaforoBomba = xSemaphoreCreateMutex();

  xTaskCreatePinnedToCore(connectWiFiTask, "ConnectWiFiTask", 10000, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(mqttTask, "MqttTask", 10000, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(vasoTask, "VasoTask1", 10000, (void *)&solo1, 1, NULL, 1);
  xTaskCreatePinnedToCore(vasoTask, "VasoTask2", 10000, (void *)&solo2, 1, NULL, 1);
  xTaskCreatePinnedToCore(vasoTask, "VasoTask3", 10000, (void *)&solo3, 1, NULL, 1);
  xTaskCreatePinnedToCore(vasoTask, "VasoTask4", 10000, (void *)&solo4, 1, NULL, 1);
  
}

void loop() {
  vTaskSuspend(NULL);
}
