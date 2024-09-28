//Fila - 02
TaskHandle_t myTaskHandle = NULL;
TaskHandle_t myTaskHandle2 = NULL;
QueueHandle_t queue;

void Task1(void *arg) {
    queue = xQueueCreate(15, sizeof(int)); 
    if (queue == 0) {
     printf("Failed to create queue= %p\n", queue);
    }
    while(1){
     for(int i = 1; i<30; i=i+2){
      xQueueSend(queue, &i, portMAX_DELAY);
    }
      vTaskDelay(1000/ portTICK_RATE_MS);
    }
    
}

void Task2(void *arg) {
    int element;
    while(1){
     if( xQueueReceive(queue, &element, portMAX_DELAY)) {
        Serial.print(element);
        Serial.print("|");
        if (element >=29) {
          Serial.println();
        }
    
      vTaskDelay(1000/ portTICK_RATE_MS);
     }
    }

}

void setup(void)
{
  Serial.begin(115200);
  Serial.println("Fila");  
   xTaskCreate(Task1, "Task1", 4096, NULL, 1, &myTaskHandle);
   xTaskCreatePinnedToCore(Task2, "Task2", 4096, NULL, 1, &myTaskHandle2, 1);
 }

 void loop(){

 }
