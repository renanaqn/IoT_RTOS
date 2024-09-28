//Mutex

#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

#define led 2
// Globals
static int shared_var = 0;
static SemaphoreHandle_t mutex;

char c;
char buf[50];
int idx=0;
static volatile uint8_t flag=0;
int tam;

//*****************************************************************************
// Tasks


void TaskA(void *parameters) {
  // Loop forever
  Serial.println("Entrei na TaskA ");
  while (1) {
    if (xSemaphoreTake(mutex, 0) == pdTRUE) {
      while (Serial.available()>0){//verifica se há algo na serial
        c = Serial.read();//ler o caracter
        if (idx < 50 - 1) {
            buf[idx] = c;//insere o caracter no vetor
            idx++; //incrementa o indice
        }
        if (c == '\n') {//quando for dado o enterna porta serial
            buf[idx - 1] = '\0'; //insere o /0 ao final da string
            tam = idx; // ver o tamanho do dado recebido
            idx = 0; //zera o indice
            Serial.print("Enviando: ");
            for(int i=0;i<tam;i++){
              Serial.print(buf[i]);
            }
            Serial.println( );
        }            
      }
      xSemaphoreGive(mutex);
    }
  }
}

void TaskB(void *parameters) {
  Serial.println("Entrei na TaskB ");
  while(1){
    if (xSemaphoreTake(mutex, 0) == pdTRUE) {
      if (buf[0] != '\0') {
        digitalWrite(led, 1);
        delay(300);
        digitalWrite(led, 0);
        delay(300);
        buf[0]='\0';
      }
      xSemaphoreGive(mutex);
    }
  }
}



//*****************************************************************************
// Main (runs as its own task with priority 1 on core 1)

void setup() {

  // Hack to kinda get randomness
  randomSeed(analogRead(0));
  pinMode(led, OUTPUT);

  // Configure Serial
  Serial.begin(115200);

  // Wait a moment to start (so we don't miss Serial output)
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  Serial.println();
  Serial.println("---FreeRTOS Race Condition Demo---");

  // Create mutex before starting tasks
  mutex = xSemaphoreCreateMutex();

  // Start task 1
  xTaskCreatePinnedToCore(TaskA,
                          "TaskA",
                          1024,
                          NULL,
                          1,
                          NULL,
                          app_cpu);

  // Start task 2
  xTaskCreatePinnedToCore(TaskB,
                          "TaskB",
                          1024,
                          NULL,
                          1,
                          NULL,
                          app_cpu);           

  // Delete "setup and loop" task
  vTaskDelete(NULL);
}

void loop() {
  // Execution should never get here
}
