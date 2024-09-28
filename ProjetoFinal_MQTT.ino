//Projeto Final - IoT - Com MQTT v1

#include <DHTesp.h>
#include <WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

/************************* WiFi Access Point ***************************/
#define WLAN_SSID "Wokwi-GUEST"
#define WLAN_PASS ""

/************************* Adafruit.io Setup ***************************/
#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  XX                   // use 8883 for SSL
#define AIO_USERNAME    "XX"
#define AIO_KEY         "XX"
#define AIO_FEED_1      "/feeds/rtos.temperature"
#define AIO_FEED_2      "/feeds/rtos.humidity"
#define AIO_FEED_3      "/feeds/rtos.level"

// Wifi and MQTT

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_USERNAME, AIO_KEY);

// Publishers
Adafruit_MQTT_Publish tempPublish = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME AIO_FEED_1);
Adafruit_MQTT_Publish humiPublish = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME AIO_FEED_2);
Adafruit_MQTT_Publish nivelPublish = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME AIO_FEED_3);

// Settings
static const uint16_t timer_divider = 80; // Count at 1 MHz
static const uint64_t timer_max_count = 1000000;

// Pins
static const int dht_pin = 14;
static const int pot_pin = 12;
static const int btn_pin = 2;

struct sensor {
  int N_Lei;
  float Lei_Temp;
  float Lei_Umi;
  float Lei_Nivel;
};
 
TaskHandle_t myTaskHandle = NULL;
TaskHandle_t myTaskHandle2 = NULL;
QueueHandle_t queue;

EventGroupHandle_t evt;

DHTesp dhtSensor;


#define USUA_EV (1<<0)
#define RELO_EV (1<<1)

static hw_timer_t *timer = NULL;

static volatile uint16_t temp;
static volatile uint16_t umi;
static volatile uint16_t nivel;

static SemaphoreHandle_t mutex;

void wifiSetup() {
  // Connect to WiFi access point.
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS, 6);
  while (WiFi.status() != WL_CONNECTED) {
    vTaskDelay(500 / portTICK_PERIOD_MS);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi Connected!");
}

void mqttSetup() {
  int8_t ret;
  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }
  Serial.print("Connecting to MQTT... ");
  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying MQTT connection in 10 seconds...");
    mqtt.disconnect();
    vTaskDelay(10000 / portTICK_PERIOD_MS);
    retries--;
    if (retries == 0) {
      // basically die and wait for WDT to reset me
      while (1);
    }
  }
  Serial.println("MQTT Connected!");
}

//*****************************************************************************
// Interrupt Service Routines (ISRs)

// This function executes when timer reaches max (and resets)
void IRAM_ATTR onTimer() {
  xEventGroupSetBits(evt, RELO_EV);
}

void ISRCallback() {
  xEventGroupSetBits(evt, USUA_EV);
}

//*****************************************************************************
// Tasks


void Coleta (void *pvParameters) {
  queue = xQueueCreate(5, sizeof(struct sensor)); 
  if (queue == 0) {
    printf("Failed to create queue= %p\n", queue);
  }
  struct sensor mySensor;
  Serial.println("Fila criada para anotar os dados!");
  while (1) {
    //xSemaphoreTake(mutex, portMAX_DELAY);
      for(int i = 0; i<5; i++){
        TempAndHumidity data = dhtSensor.getTempAndHumidity();
        mySensor.Lei_Temp = data.temperature;
        mySensor.Lei_Umi = data.humidity;
        mySensor.Lei_Nivel = analogRead(pot_pin);
        Serial.println(analogRead(pot_pin));
        Serial.println("Coleta realizada!");
        xQueueSend(queue, &mySensor, portMAX_DELAY);
        vTaskDelay(500 / portTICK_PERIOD_MS);
      }
     //xSemaphoreGive(mutex);   
  }
    vTaskDelay(2000 / portTICK_PERIOD_MS);
}

void Cont_Temp (void *pvParameters) {
  while (1) {
    TempAndHumidity data = dhtSensor.getTempAndHumidity();
    temp = data.temperature;
    if (temp>=60) {
      Serial.println("----------------------");
      Serial.print("A Temperatura está em: ");
      Serial.println(temp);
      Serial.println("Abaixando Temperatura");
      Serial.println("----------------------");
      vTaskDelay(10 / portTICK_PERIOD_MS);
    } else if (temp < 10) {
      Serial.println("----------------------");
      Serial.print("A Temperatura está em: ");
      Serial.println(temp);
      Serial.println("Aumentando Temperatura");
      Serial.println("----------------------");
      vTaskDelay(10 / portTICK_PERIOD_MS);
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void Cont_Nivel (void *pvParameters) {
  while (1) {
    nivel = analogRead(pot_pin);
    if (nivel < 1024){
      Serial.println("----------------------");
      Serial.print("O Nível está em: ");
      Serial.println(nivel);
      Serial.println("Abrindo válvula para aumentar o Nivel");
      Serial.println("----------------------");
      vTaskDelay(10 / portTICK_PERIOD_MS);
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void Envia (void *arg) {
  const EventBits_t xBitsToWaitFor  = (RELO_EV | USUA_EV);
  EventBits_t x;
  struct sensor dados; 
  while(1){
    //Serial.println("--- To em Envia");
    x = xEventGroupWaitBits(evt, xBitsToWaitFor, pdTRUE, pdTRUE, pdMS_TO_TICKS(1000));

    if ((x & RELO_EV) != 0 || (x & USUA_EV) != 0) {
      if (uxQueueSpacesAvailable (queue) == 0) {
        mqttSetup();
        xSemaphoreTake(mutex, portMAX_DELAY);
        for(int i = 0; i<5; i++){
          xQueueReceive(queue, &dados, portMAX_DELAY);
      
          Serial.println("------- Envio dos Dados ------- ");
          Serial.print("Temperatura: ");  
          Serial.println(dados.Lei_Temp);
          if (!(dados.Lei_Temp != dados.Lei_Temp) && i == 4) tempPublish.publish(dados.Lei_Temp);

          Serial.print("Umidade: ");
          Serial.println(dados.Lei_Umi);
          if (!(dados.Lei_Umi != dados.Lei_Umi) && i == 4) humiPublish.publish(dados.Lei_Umi);

          Serial.print("Nivel: ");
          Serial.println(dados.Lei_Nivel);
          if (!(dados.Lei_Nivel != dados.Lei_Nivel) && i == 4) nivelPublish.publish(dados.Lei_Nivel);
      
          Serial.println("----------------");
          vTaskDelay(500/ portTICK_RATE_MS);
        }
        xSemaphoreGive(mutex);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
      }
    }
     
  }
}




void setup() {
  Serial.begin(115200);

  evt = xEventGroupCreate();
  // Wait a moment to start (so we don't miss Serial output)
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  Serial.println();
  Serial.println("--- Projeto Final IoT ---");
  Serial.println();
  
  pinMode(btn_pin, INPUT_PULLUP);
  
  dhtSensor.setup(dht_pin, DHTesp::DHT22);

  wifiSetup();
  
  mutex = xSemaphoreCreateMutex();
  xSemaphoreGive(mutex);
  // Criação das tarefas com mesmo nível de prioridade e tamanho
  xTaskCreatePinnedToCore(Coleta, "Coleta", 1024, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(Cont_Temp, "Cont_Temp", 1024, NULL, 3, NULL, 1);
  xTaskCreatePinnedToCore(Cont_Nivel, "Cont_Nivel", 1024, NULL, 3, NULL, 1);
  xTaskCreatePinnedToCore(Envia, "Envia", 2048, NULL, 1, NULL, 0);


  // Create and start timer (num, divider, countUp)
  timer = timerBegin(0, timer_divider, true);

  // Provide ISR to timer (timer, function, edge)
  timerAttachInterrupt(timer, &onTimer, true);

  // At what count should ISR trigger (timer, count, autoreload)
  timerAlarmWrite(timer, timer_max_count, true);

  // Allow ISR to trigger
  timerAlarmEnable(timer);
}

void loop() {
  // Vazio
}
