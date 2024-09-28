//RTOS Alocação de Memória - Envio de dados pela serial
  char c;
  //char buf[50];
  char *buf = (char*)pvPortMalloc(50 * sizeof(char)); //Malloc
  int idx=0;
  static volatile uint8_t flag=0;
  int tam;

void receber(void *arg){
  while(1){    
    //Serial.print("Heap depois do malloc (bytes): "); 
    //Serial.println(xPortGetFreeHeapSize());
    while (Serial.available()>0){//verifica se há algo na serial
      c = Serial.read();//ler o caracter
      if (idx < 50 - 1) {
          buf[idx] = c;//insere o caracter no vetor
          idx++; //incrimenta o indice
      }
      if (c == '\n') {//quando for dado o enterna porta serial
          buf[idx - 1] = '\0'; //inseri o /0 ao final da string
          tam = idx; // ver o tamanho do dado recebido
          idx = 0; //zero o indice
          flag=1; //faz a flag igual a 1
          //imprimi o valor na task
          Serial.print("Enviando: ");
          for(int i=0;i<tam;i++){
            Serial.print(buf[i]);
          }
          Serial.println( );
          //Serial.println(flag);
          //Serial.println(tam); 
        
      }
      vPortFree(buf);            
    }
    
  }
}

void enviar(void *arg) {

  while (1) {
    if (flag == 1){ //verifica se a flag foi acionada
     Serial.print("Recebendo: "); //imprimia mesma coisa recebida na outra task
       for(int i=0;i<tam;i++){
         Serial.print(buf[i]);
       }
       flag=0;//zera a flag
       Serial.println();
    }  
  }
}

void setup() {
  // Configure Serial
  Serial.begin(115200);
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  //Serial.print("Heap antes do malloc (bytes): ");
  //Serial.println(xPortGetFreeHeapSize());

  // Start task 1
  xTaskCreatePinnedToCore(enviar,
                          "enviar",
                          1024,
                          NULL,
                          1,
                          NULL,
                          1);

  xTaskCreatePinnedToCore(receber,
                          "Receber",
                          1024,
                          NULL,
                          1,
                          NULL,
                          1);
}

void loop() {

}
