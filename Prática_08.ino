//Pinagem
int leds[]={2, 16, 18, 22};
int bts[]={23, 19, 17, 4};
int q1 [24];
int q2 [24];

String labels[]={"Azul", "Amarelo", "Verde", "Vermelho"};
QueueHandle_t queue;
static SemaphoreHandle_t mutex;


void TaskGer(void *parameters) {
  // Loop forever
  Serial.println("Entrei na TaskGer ");
  while (1) {
    if (xSemaphoreTake(mutex, 0) == pdTRUE) {
      for (int i = 0; i < 24; i++) {
        q1[i]=random(4);
        digitalWrite(leds[q1[i]],1);
        delay(500);
        digitalWrite(leds[q1[i]],0);
        delay(500);
        xQueueSend(queue, &q1[i], portMAX_DELAY);
        xSemaphoreGive(mutex);       
      }
    }
    vTaskDelay(1000/ portTICK_RATE_MS);
  }
}

void TaskRecebe(void *parameters) {
  // Loop forever
  Serial.println("Entrei na TaskRecebe");
  while (1) {
    if (xSemaphoreTake(mutex, 0) == pdTRUE) {
      for(int j = 0; j<24; j++){
        xQueueReceive(queue, &q2[j], portMAX_DELAY);
        digitalWrite(leds[q2[j]],1);
        delay(500);
        digitalWrite(leds[q2[j]],0);
        delay(500);
        xSemaphoreGive(mutex);    
     } 
    }
    vTaskDelay(1000/ portTICK_RATE_MS);
  }
}



void setup() {
  queue = xQueueCreate(24, sizeof(int)); 
  if (queue == 0) {
    printf("Failed to create queue= %p\n", queue);
  }
  Serial.begin(115200);
  Serial.println("Simon");
  mutex = xSemaphoreCreateMutex();

  for (int i=0; i<4 ; i++){
    pinMode(leds[i], OUTPUT);
    pinMode(bts[i], INPUT_PULLUP);
  }  
  xTaskCreatePinnedToCore(TaskGer, "TaskGer", 1024, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(TaskRecebe, "TaskRecebe", 1024, NULL, 1, NULL, 1);
  //xTaskCreatePinnedToCore(Task, "Task", 1024, NULL, 1, NULL, 1);
}

void loop() { 
}
