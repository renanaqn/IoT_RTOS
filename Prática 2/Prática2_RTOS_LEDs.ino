#define botao1 23
#define botao2 22
#define led1 4
#define led2 2

//código sem interruptção: 
/*
void setup() {
  // put your setup code here, to run once:
  pinMode(botao1, INPUT);
  pinMode(botao2, INPUT);
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  Serial.begin(115200);
  Serial.println("Hello, ESP32!");
}

void loop() {
if (digitalRead(botao1)==0){
  digitalWrite(led1, !digitalRead(led1));
  Serial.println("led 1 mudei de estado ");
}

delay(200);

if(digitalRead(botao2)==0){
  digitalWrite(led2, !digitalRead(led2));
  Serial.println("led 2 mudei de estado ");
}

delay(200);
}
*/


//código com interruptção:
///*
void setup() {
  // put your setup code here, to run once:
  pinMode(botao1, INPUT);
  pinMode(botao2, INPUT);
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  Serial.begin(115200);
  Serial.println("Hello, ESP32!");
  attachInterrupt(digitalPinToInterrupt(botao1),acionarLED1, FALLING);
  attachInterrupt(digitalPinToInterrupt(botao2),acionarLED2, FALLING);
}

void loop() {
  while(true){
    Serial.println("Programa dentro do loop while");
    Serial.println("tempo: "+ String(millis()));
    delay(50);
  }
}

void acionarLED1 (){
  digitalWrite(led1, !digitalRead(led1));
}

void acionarLED2 (){
  digitalWrite(led2, !digitalRead(led2));
}
//*/
