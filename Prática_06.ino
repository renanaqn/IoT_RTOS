//Mutex Separador

#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

#define led 2
// Globals
static int shared_var = 0;
static SemaphoreHandle_t mutex;
int i;
int t = 50;
char c;
char a[50];
char b[50];
char c2[50];
char d[50];
int idx=0;
static volatile uint8_t flag=0;
int tam;

//*****************************************************************************
// Tasks


void TaskA(void *parameters) {
  
  Serial.println("Entrei na TaskA ");
  while (1) {
    if (xSemaphoreTake(mutex, 0) == pdTRUE) {
      while (Serial.available()>0){//verifica se há algo na serial
        c = Serial.read();//ler o caracter
        if (idx < 50 - 1) {
            a[idx] = c;//insere o caracter no vetor
            idx++; //incrementa o indice
        }
        if (c == '\n') {//quando for dado o enter na porta serial
            a[idx - 1] = '\0'; //insere o /0 ao final da string
            tam = idx; // ver o tamanho do dado recebido
            idx = 0; //zera o indice
            Serial.print("Enviando: ");
            for(int i=0;i<tam;i++){
              Serial.print(a[i]);
            }
            Serial.println( );
           
        }     
         xSemaphoreGive(mutex);       
      }
      
    }
  }
}

//task para separar
void TaskB(void *parameters) {
  Serial.println("Entrei na TaskB ");
  while(1){
    if (xSemaphoreTake(mutex, 0) == pdTRUE) {
        for(i=0;i<50;i++){
          if(a[i]>=65 && a[i]<=90){ //separando as Maiúsculas
            b[i]=a[i];
          }
      
          if(a[i]>=97 && a[i]<=122){ //separando as minúsculas
            c2[i]=a[i];
          } 
    
          if(a[i]>=48 && a[i]<=57){ //separando os números
            d[i]=a[i];
          }

        }
      xSemaphoreGive(mutex);
    }
  }
}

//task para ordenar
void TaskC(void *parameters) {
  Serial.println("Entrei na TaskC ");
  while(1){
    if (xSemaphoreTake(mutex, 0) == pdTRUE) {
      int temp = 0;
      int temp2 = 0;             
      int temp3 = 0;
      for (int i = 0; i < t; i++) {     
          for (int j = i+1; j < t; j++) {     
              if(b[i] > b[j]) {    
                  temp = b[i];    
                  b[i] = b[j];    
                  b[j] = temp;    
              }
              if(c2[i] > c2[j]) {    
                  temp2 = c2[i];    
                  c2[i] = c2[j];    
                  c2[j] = temp2;    
              }
              if(d[i] > d[j]) {    
                  temp3 = d[i];    
                  d[i] = d[j];    
                  d[j] = temp3;    
              }     
          }     
      }
      xSemaphoreGive(mutex);
    }
  }
}

//task para imprimir
void TaskD(void *parameters) {
  Serial.println("Entrei na TaskD ");
  while(1){
    if (xSemaphoreTake(mutex, 0) == pdTRUE) {
       Serial.print("Recebendo: "); //imprimia mesma coisa recebida na outra task
        for(int i=0;i<tam;i++){
         Serial.print(a[i]);
        }
       Serial.println( );
        for(int i=0;i<tam;i++){
          Serial.print(b[i]);
        }
       Serial.println( );
       for(int i=0;i<tam;i++){
         Serial.print(c2[i]);
       }
       Serial.println( );
       for(int i=0;i<tam;i++){
         Serial.print(d[i]);
       }
       Serial.println( );

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
  Serial.println("---FreeRTOS Separador e Ordenador---");

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

// Start task 3
  xTaskCreatePinnedToCore(TaskC,
                          "TaskC",
                          1024,
                          NULL,
                          1,
                          NULL,
                          app_cpu); 

// Start task 4
  xTaskCreatePinnedToCore(TaskD,
                          "TaskD",
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
