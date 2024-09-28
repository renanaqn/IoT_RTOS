//Semáforo

/* LED pins */
byte redPin = 2; // Pino do Led Vermelho
byte yellowPin = 16; // Pino do Led Amarelo
byte greenPin = 18; // Pino do Led Verde
byte ledPin = 21; // Pino do Led de Pedestre
int flag = 0; // flag para ser usada dentro da interrupção

// Pino do botão de interrupção
byte interruptPin = 22;

volatile bool isPaused = false; // Flag para pausar a operação do semáforo de trânsito
BaseType_t xHigherPriorityTaskWoken = pdFALSE;

SemaphoreHandle_t xCountingSemaphore; // Semáforo contador
SemaphoreHandle_t SemasTransito; // Semáforo binário 


// Callback da função de interrupção 
void ISRcallback() {
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  /* un-block the interrupt processing task now */
  xSemaphoreGiveFromISR( xCountingSemaphore, &xHigherPriorityTaskWoken );
  // Uma vez que a interrupção é acionada o estado de isPaused vai para true para travar 
  // o funcionamento do semáforo de trânsito
  isPaused = true;

}

// Função para operar a interrupção
void ISRprocessing(void * parameter) {
  for (;;) {
    xSemaphoreTake( xCountingSemaphore, portMAX_DELAY );
    Serial.println("Interrupção em Andamento!");
    // Pisca o led para representar a interrupção
    digitalWrite(ledPin,1);
    delay(1000);
    digitalWrite(ledPin,0);
    delay(1000);
    flag++; 
    if (flag == 3) {
      // Flag que serve para sincronizar com o semáforo contador a fim de liberar o semáforo binário para 
      // a operação do semáforo de trânsito voltar a funcionar
      flag = 0;
      isPaused = false;
      Serial.println("Voltando com o semáforo de trânsito");
      xSemaphoreGiveFromISR(SemasTransito, &xHigherPriorityTaskWoken);    
    }
  }
  //vTaskDelete( NULL );
}

// Tarefa para o controle do semáforo de trânsito
void TransitoLuz(void * parameter) {
  for (;;) {
    if (xSemaphoreTake(SemasTransito, portMAX_DELAY)) {
      // Led Vermelho
      digitalWrite(redPin, HIGH);
      digitalWrite(yellowPin, LOW);
      digitalWrite(greenPin, LOW);
      vTaskDelay(2000 / portTICK_PERIOD_MS); 
      if (isPaused==true) {
        continue; // Espera até o semáforo binário ser dado novamente
      }
      // Led Verde
      digitalWrite(redPin, LOW);
      digitalWrite(yellowPin, LOW);
      digitalWrite(greenPin, HIGH);
      vTaskDelay(1000 / portTICK_PERIOD_MS); 
      if (isPaused==true) {
        continue; // Espera até o semáforo binário ser dado novamente
      }

      // Led Amarelo
      digitalWrite(redPin, LOW);
      digitalWrite(yellowPin, HIGH);
      digitalWrite(greenPin, LOW);
      vTaskDelay(500 / portTICK_PERIOD_MS); 
      if (isPaused==true) {
        continue; // Espera até o semáforo binário ser dado novamente
      }
      
    }
    xSemaphoreGive(SemasTransito);
  }  
  //vTaskDelete(NULL);
}


void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  pinMode(redPin, OUTPUT);
  pinMode(yellowPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(interruptPin, INPUT_PULLUP);

  Serial.println();
  Serial.println("---Prática 09 Semáforo de Trânsito---");
  Serial.println();

  // Assimila a interrupção ao botão 
  attachInterrupt(digitalPinToInterrupt(interruptPin), ISRcallback, RISING);

  // Semáforo contador que pode acumular até 3 eventos
  xCountingSemaphore = xSemaphoreCreateCounting(2, 0);
  SemasTransito = xSemaphoreCreateBinary();
  xSemaphoreGive(SemasTransito);
  
  /* this task will process the interrupt event 
     which is forwarded by interrupt callback function */
  xTaskCreate(
    ISRprocessing,           /* Task function. */
    "ISRprocessing",        /* name of task. */
    1000,                    /* Stack size of task */
    NULL,                     /* parameter of the task */
    2,                        /* priority of the task */
    NULL); 

  /* Task to control the traffic light */
  xTaskCreate(
    TransitoLuz,           /* Task function. */
    "TransitoLuz",        /* name of task. */
    1000,                      /* Stack size of task */
    NULL,                      /* parameter of the task */
    1,                         /* priority of the task */
    NULL);
}

void loop() {
  // No need for any code in the loop, as FreeRTOS tasks handle the functionality.
}
