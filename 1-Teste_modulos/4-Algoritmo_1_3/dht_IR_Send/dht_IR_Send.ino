/*
* Projeto de monitoramento via SNMP e controle da temperatura e umidade de Data Center
* Autor: Denilly Carvalho do Carmo
* Versão: 0.1 Julho de 2019
* Microcontrolador: Arduino Uno R3
* Componentes: 
  - LED emissor IR conectado por padrão ao pino 3 (PWM) do Arduino;
  - Resistor de 100 Ω conectado em série ao cátodo (negativo) do LED IR;
  - Receptor IR KY-022 para decodificação do controle remoto;
  - Sensor de temperatura e umidade DHT 11 (Pinos: 1 - VCC / 2 - DADOS / 4 - GND); e
  - Ethernet Shield W5100.
    Observação: a placa Arduino se comunica com a shield usando o barramento SPI (SCK - Clock, MOSI - Master Output/Slave Input, 
    MISO - Master Input/Slave Output e SS - Slave Select) para realizar uma comunicação full duplex ocupando os pinos digitais 
    11, 12 e 13 no Uno e nos pinos 50, 51 e 52 no Mega. Em ambos, o pino 10 é usado como SS (Slave Select) para Ethernet Controller e o 4 para SD Card.
* Referências:
  - https://www.filipeflop.com
  - https://github.com/1sw/Agentuino
  - http://arduinoprojexp.blogspot.com/2014/11/agentuino-zabbix-parte-1-implementacao.html
  - https://pt.slideshare.net/lailtonmontenegro/monitoramento-de-temperatura-e-umidade-de-data-center-utilizando-o-arduino-e-o-sistema-zabbix
  - http://labdegaragem.com/profiles/blogs/controlando-ar-condicionado-utilizando-arduino-e-led
  - https://www.instructables.com/id/Ar-Condicionado-controlado-por-Arduino-via-infrave/
  - https://arduinolivre.wordpress.com/2012/07/31/clonando-qualquer-controle-remoto/
*/

// 1 - Importando Bibliotecas
#include <DHT.h> // sensor de temperatura e umidade

// 2 - Define variaveis e parametros das bibliotecas e do programa
#define DHTPIN A1 // Sensor DHT conectado ao pino analógico A1
#define DHTTYPE DHT11 // Sensor DHT 11
int IRledPin = 3; // LED Infravermelho conectado ao pino digital 3
int enviouIrOn = 0; // Variável para informar envio de sinal IR para ligar o Ar Condicionado
int enviouIrTemp = 0; // Variável para informar envio de sinal IR para ajustar a temperatura do Ar Condicionado
unsigned long tempoAnteriorUm = 0; // Variavel para contagem de tempo dos intervalos de verificacao de funcionamento do Ar Condicionado 
unsigned long tempoIntervaloUm = 300000; // Constante de tempo de intervalo para leitura da temperatura e umidade, em milisegundos (300000 ms = 5 min)
unsigned long tempoAnteriorDois = 0; // Variavel para contagem de tempo dos intervalos de controle da temperatura do Ar Condicionado 
unsigned long tempoIntervaloDois = 900000; // Constante de tempo de intervalo para leitura e controle da temperatura, em milisegundos (900000 ms = 15 min)

DHT dht(DHTPIN, DHTTYPE); // Define os parametros da biblioteca DHT.h

// 3 - Executa parametros de inicialização
void setup(){
  Serial.begin(9600); // Inicializa a serial
  pinMode(IRledPin, OUTPUT); // Define modo do pino 3 como saída
  dht.begin(); // Inicializa a biblioteca DHT.h
}

// 4 - Executa a funcao loop
void loop() {
  // 4.1 - obtem a umidade e temperatura e aguarda 1 seg (1000 ms)
  getUmidade();
  getTemperatura();
  delay(2000);

  // 4.2 - verifica se o ar condicionado está ligado
  if ((millis() - tempoAnteriorUm) > tempoIntervaloUm && enviouIrOn == 0){ // verifica se o tempo decorrido é maior que 5 min (300000 ms) e se o sinal de ligar o ar condicionado foi enviado
    if (dht.readTemperature() > 25){ // verifica se a temperatura obtida é maior que 25 graus
      Serial.println("Ligando Ar Condicionado...");
      ligaAr(); // executa a funcao de ligar ar condicionado
      delay(3000);
      Serial.println("Ajustando...");
      TempPadrao(); // executa a funcao de ajuste do ar condifionado para uma temperatura de referência (ex: 20 graus)
      delay(1000);
      enviouIrOn = 1; // define 1 na variavel para informar à próxima condicao que o sinal de ligar foi enviado
    }
    else {
      Serial.println("Ar condicionado: LIGADO"); // imprime msg na serial
    }
    tempoAnteriorUm = millis(); // atualiza o tempo em que o loop está sendo executado. Esta instrucao é necessária para que a operacao matematica na primeira condicao e na seguinte facam sentido zerando o contador de 2 e de 4 minutos
  }
  if ((enviouIrOn == 1) && ((millis() - tempoAnteriorUm) > 600000)){ // verifica se o sinal de ligar foi enviado e inicia o contador de 10 min (600000 ms) para que a temperatura ambiente se normalize
    enviouIrOn = 0; // reinicia a informacao de envio de sinal
    Serial.println("Passaram-se 10 minutos... Retornando para o intervalo normal."); // imprime msg na serial
  }

  // 4.3 - controla a potência do ar condicionado conforme a temperatura ambiente detectada
  if ((millis() - tempoAnteriorDois) > tempoIntervaloDois && enviouIrTemp == 0){ // verifica se o tempo decorrido é maior que 15 min (900000 ms) e se o sinal de ajustar o ar condicionado foi enviado
    Serial.println("Verificando temperatura...");
    delay(1000);
    if (dht.readTemperature() < 19) {
      aumentaTempUm();
      enviouIrTemp = 1;
    }
    else if (dht.readTemperature() > 19 && dht.readTemperature() < 24) {
      // Temperatura normal
      Serial.println("Temperatura: NORMAL"); // imprime msg na serial
    }
    else if (dht.readTemperature() > 24) {
      diminuiTempUm();
      enviouIrTemp = 1;
    }
    tempoAnteriorDois = millis();
  }
  if ((enviouIrTemp == 1) && ((millis() - tempoAnteriorDois) > 300000)){ // verifica se o sinal de ligar foi enviado e inicia o contador de 5 min (300000 ms) para que a temperatura ambiente se normalize
    if (dht.readTemperature() < 19) {
      aumentaTempDois();
    }
    else if (dht.readTemperature() > 24) {
      diminuiTempDois();
    }
  Serial.println("Passaram-se 5 min... Retornando para o intervalo normal.");
  enviouIrTemp = 0;
  }
}

// 5 - Funcoes utilizadas no loop()
// 5.1 - obtem umidade e temperatura ambiente
float getUmidade(){
  float u = dht.readHumidity(); // obtem a umidade e grava na variavel u
  if (isnan(u)) // testa se retorno é valido, caso contrário algo está errado
  {
    Serial.println("Falha no sensor de umidade");
  } 
  else
  {
    Serial.print("Umidade: ");
    Serial.print(u); // imprime a umidade na serial
    Serial.print(" %    ");
  }
  return u;
}

float getTemperatura(){
  float t = dht.readTemperature(); // obtem a temperatura e grava na variavel t
  if (isnan(t)) // testa se retorno é valido, caso contrário algo está errado
  {
    Serial.println("Falha no sensor de temperatura");
  } 
  else
  {
    Serial.print("Temperatura: ");
    Serial.print(t); // imprime a temperatura na serial
    Serial.println(" *C");
  }
  return t;
}

// 5.2 Funções para controlar o Ar Condicionado
void ligaAr(){
  // comandos gerados na captura IR
  Serial.println("Comando ligar.");
}

void TempPadrao(){
  // comandos gerados na captura IR
  Serial.println("Comando Temp Padrao.");
}

void aumentaTempUm(){
  // comandos gerados na captura IR
  Serial.println("Comando aumenta UM.");
}

void aumentaTempDois(){
  // comandos gerados na captura IR
  Serial.println("Comando aumenta DOIS.");
}

void diminuiTempUm(){
  // comandos gerados na captura IR
  Serial.println("Comando diminui UM.");
}

void diminuiTempDois(){
  // comandos gerados na captura IR
  Serial.println("Comando diminui DOIS.");
}

// a função abaixo é chamada durante a execução dos comandos gerados na captura dos sinais infravermelho emitidos por cada tecla do controle remoto
// o LED acende e apaga repetidamente em um intervalo de tempo medido em microsegundos (1 seg = 1.000.000 microseg) e o ar condicionado detecta e interpreta as piscadas do led 
// infravermelho acionando os comandos respectivos
void pulseIR(long microsecs) {
  cli();  // Desativa quaisquer interrupções de fundo
  while (microsecs > 0) {
    // 38 kHz tem cerca de 13 microssegundos em nível lógico alto e 13 microssegundos em baixo
    digitalWrite(IRledPin, HIGH);  // liga o LED. A execução desta linha leva cerca de 3 microsegundos
    delayMicroseconds(10);         // aguarda por 10 microseconds
    digitalWrite(IRledPin, LOW);   // desliga o LED. A execução desta linha leva cerca de 3 microsegundos
    delayMicroseconds(10);         // aguarda por 10 microseconds

    microsecs -= 26; // como cada ciclo acima leva 26 microsegundos para ser executado, o while é executado até a condição não ser mais atendida
  }
  sei();  // this turns them back on
}
