//Projeto Final - IoT - Sem MQTT

#include <DHTesp.h>

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
static volatile uint16_t ler_nivel;

static SemaphoreHandle_t mutex;


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
  struct sensor dados; 
  while(1){
    if (uxQueueSpacesAvailable (queue) == 0) {
      xSemaphoreTake(mutex, portMAX_DELAY);
      Serial.println("--------- Entrei em Envio 2");
      for(int i = 0; i<5; i++){
        xQueueReceive(queue, &dados, portMAX_DELAY);

        Serial.println("------- Envio dos Dados ------- ");
        Serial.print("Temperatura: ");
        Serial.println(dados.Lei_Temp);

        Serial.print("Umidade: ");
        Serial.println(dados.Lei_Umi);

        Serial.print("Nivel: ");
        Serial.println(dados.Lei_Nivel);
    
        Serial.println("----------------");
        vTaskDelay(500/ portTICK_RATE_MS);
      }
      xSemaphoreGive(mutex);
      vTaskDelay(1000 / portTICK_PERIOD_MS);
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

  
  mutex = xSemaphoreCreateMutex();
  xSemaphoreGive(mutex);
  // Criação das tarefas com mesmo nível de prioridade e tamanho
  xTaskCreatePinnedToCore(Coleta, "Coleta", 1024, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(Cont_Temp, "Cont_Temp", 1024, NULL, 3, NULL, 1);
  xTaskCreatePinnedToCore(Cont_Nivel, "Cont_Nivel", 1024, NULL, 3, NULL, 1);
  xTaskCreatePinnedToCore(Envia, "Envia", 1024, NULL, 1, NULL, 0);


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
