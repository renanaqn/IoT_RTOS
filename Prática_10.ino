// Bombeamento

// Definição de pinos para os LEDs simulando os motores
const int motor1Pin = 16;
const int motor2Pin = 2;

// Pino do potenciômetro para simular o nível do reservatório
const int potPin = 34;

// Pino do botão para acionar o sistema
const int buttonPin = 22;
volatile byte state = LOW;

// Variáveis para controle do tempo
// REVER SOFTWARE TIMER?
const TickType_t intervalo10Dias = pdMS_TO_TICKS(10000); // 10 dias em milissegundos
const TickType_t intervalo5Min = pdMS_TO_TICKS(300); // 5 minutos em milissegundos

// variavel para fazer a leitura do valor do potenciomento simulando o nivel do reservatório
// onde foi considerado que 2048 é a metade do reservatório
// pois a leitura do ADC do ESP é de 0 à 4095
float Nivel = analogRead(potPin);
static SemaphoreHandle_t mutex;

// Tarefa para verificar se está quase vazio com tendência para acionar o motor 1
// e depois de um tempo, aciona o motor 2
void QVazio1(void *pvParameters) {
  Nivel = analogRead(potPin);
  while (true) {
    if (Nivel<2048 && state == true) {
      xSemaphoreTake(mutex, portMAX_DELAY);
      Serial.println("Caso os motores já estejam acionados, permanecem como está, caso não:");
      Serial.println("Ativando Motor 1º, mas jaja ligo o 2º");
      digitalWrite(motor1Pin, HIGH); // Liga o LED simulando o motor 1
      Serial.print("Nivel atual: ");
      Serial.print("Nivel atual: ");
      Serial.println(Nivel);
      vTaskDelay(2500 / portTICK_PERIOD_MS);; // Espera 5 min
      digitalWrite(motor2Pin, HIGH); // Liga o LED simulando o motor 2
      vTaskDelay(5000 / portTICK_PERIOD_MS); // Espera 10 Dias 
      Nivel = analogRead(potPin);
      Serial.println(Nivel);
      xSemaphoreGive(mutex);
    }
    //vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}
// Tarefa para verificar se está quase vazio com tendência para acionar o motor 2
// e depois de um tempo, aciona o motor 1
void QVazio2(void *pvParameters) {
  Nivel = analogRead(potPin);
  while (true) {
    if (Nivel<2048 && state == true) {
      xSemaphoreTake(mutex, portMAX_DELAY);
      Serial.println("Caso os motores já estejam acionados, permanecem como está, caso não:");
      Serial.println("Ativando Motor 2º, mas jaja ligo o 1º");
      digitalWrite(motor2Pin, HIGH); // Liga o LED simulando o motor 2
      Serial.print("Nivel atual: ");
      Serial.println(Nivel);
      vTaskDelay(2500 / portTICK_PERIOD_MS);; // Espera 5 min
      digitalWrite(motor1Pin, HIGH); // Liga o LED simulando o motor 1
      Serial.print("Nivel atual: ");
      Serial.println(Nivel);
      vTaskDelay(5000 / portTICK_PERIOD_MS); // Espera 10 Dias 
      Nivel = analogRead(potPin);
      xSemaphoreGive(mutex);
    }
    //vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}
// Tarefa para verificar se está quase cheio com tendência para acionar o motor 1
void QCheio1(void *pvParameters) {
  Nivel = analogRead(potPin);
  while (true) {
    if (Nivel>=2048 && state == true) {
      xSemaphoreTake(mutex, portMAX_DELAY);
      Serial.print("Nivel atual: ");
      Serial.println(Nivel);
      Serial.println("Caso os motor já esteja acionado, permanece como está, caso não:");
      Serial.println("Ativando só o Motor 1");
      digitalWrite(motor1Pin, HIGH); // Liga o LED simulando o motor 1
      digitalWrite(motor2Pin, LOW); // Desliga o LED simulando o motor 2 (só para garantir)
      vTaskDelay(5000 / portTICK_PERIOD_MS); // Espera 10 Dias 
      Nivel = analogRead(potPin);
      Serial.print("Nivel atual: ");
      Serial.println(Nivel);
      xSemaphoreGive(mutex);
    }
    //vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

// Tarefa para verificar se está quase cheio com tendência para acionar o motor 2
void QCheio2(void *pvParameters) {
  Nivel = analogRead(potPin);
  while (true) {
    if (Nivel>=2048 && state == true) {
      xSemaphoreTake(mutex, portMAX_DELAY);
      Serial.print("Nivel atual: ");
      Serial.println(Nivel);
      Serial.println("Caso os motor já esteja acionado, permanece como está, caso não:");
      Serial.println("Ativando só o Motor 2");
      digitalWrite(motor2Pin, HIGH); // Liga o LED simulando o motor 2
      digitalWrite(motor1Pin, LOW); // Desliga o LED simulando o motor 1 (só para garantir)
      vTaskDelay(5000 / portTICK_PERIOD_MS); // Espera 10 Dias 
      Nivel = analogRead(potPin);
      Serial.print("Nivel atual: ");
      Serial.println(Nivel);
      xSemaphoreGive(mutex);
    }
    //vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void setup() {
  Serial.begin(115200);

  // Wait a moment to start (so we don't miss Serial output)
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  Serial.println();
  Serial.println("---Prática 10 Bombeamento---");
  Serial.println();
  
  pinMode(motor1Pin, OUTPUT);
  pinMode(motor2Pin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);

  //mutex para o controle de qual tarefa será utilizada
  mutex = xSemaphoreCreateMutex();

  // While para travar o sistema esperando o acionamento do botão, para então operar como se deve
  while(state==false){  
    Serial.println("Esperando botao para iniciar");
    if (digitalRead(buttonPin)==0){
      state = true;
      Serial.println("Botao foi pressionado");
      xSemaphoreGive(mutex);
    }
  } 

  // Criação das tarefas com mesmo nível de prioridade e tamanho
  xTaskCreate(QVazio1, "QVazio 1", 1024, NULL, 1, NULL);
  xTaskCreate(QVazio2, "QVazio 1", 1024, NULL, 1, NULL);
  xTaskCreate(QCheio1, "QCheio 1", 1024, NULL, 1, NULL);
  xTaskCreate(QCheio2, "QCheio 2", 1024, NULL, 1, NULL);
}

void loop() {
  // Vazio
}
