//Fila 3
#define temp 32 
#define ldr 34  

struct sensor {
  int deviceId;
  int Lei_LDR;
  float Lei_Temp;
};
 
TaskHandle_t myTaskHandle = NULL;
TaskHandle_t myTaskHandle2 = NULL;
QueueHandle_t queue;


void Task1(void *arg) {
    queue = xQueueCreate(5, sizeof(struct sensor)); 
    if (queue == 0) {
     printf("Failed to create queue= %p\n", queue);
    }
    struct sensor mySensor;
    while(1){
     for(int i = 0; i<10; i++){
      int leitor_ldr = analogRead(ldr);
      int analogvalue = analogRead(temp);
      
      mySensor.deviceId = i;
      mySensor.Lei_LDR = leitor_ldr;
      mySensor.Lei_Temp = analogvalue;
      xQueueSend(queue, &mySensor, portMAX_DELAY);
     }
      
    }
    
}

void Task2(void *arg) {
    struct sensor element;
    while(1){
     for(int i = 0; i<10; i++){
      xQueueReceive(queue, &element, portMAX_DELAY);
  
      Serial.print("Device ID: ");
      Serial.println(element.deviceId);
  
      Serial.print("Leitura LDR: ");
      Serial.println(element.Lei_LDR);
  
      Serial.print("Temperatura: ");
      Serial.println(element.Lei_Temp);
  
      Serial.println("----------------");
      Serial.println(analogRead(ldr));
      vTaskDelay(500/ portTICK_RATE_MS);
     }
    }

}

void setup() {
 
  Serial.begin(115200);
  Serial.println("Fila");  
  pinMode (ldr, INPUT);
  //pinMode (temp, INPUT);
  
   xTaskCreate(Task1, "Task1", 4096, NULL, 1, &myTaskHandle);
   xTaskCreatePinnedToCore(Task2, "Task2", 4096, NULL, 1, &myTaskHandle2, 1);
 }
 
void loop() {
}
