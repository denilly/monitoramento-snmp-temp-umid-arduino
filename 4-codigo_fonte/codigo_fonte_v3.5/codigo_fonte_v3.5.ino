/*
* Projeto de monitoramento via SNMP e controle da temperatura e umidade de Data Center
* Autor: Denilly Carvalho do Carmo
* Versão: 3.5 Outubro de 2019
* Placa: Arduino Uno R3
* Microcontrolador: ATmega328P
* Componentes:
  - LED emissor IR conectado ao pino 3 (PWM) do Arduino;
  - Resistor de 100 Ω conectado em série ao cátodo (negativo) do LED IR;
  - Receptor IR KY-022 para decodificação do controle remoto;
  - Sensor de temperatura e umidade DHT (Pinos: 1 - VCC / 2 - DADOS / 4 - GND); e
  - Ethernet Shield W5100.
    Observação: a placa Arduino se comunica com a shield usando o barramento SPI (SCK - Clock, MOSI - Master Output/Slave Input, 
    MISO - Master Input/Slave Output e SS - Slave Select) para realizar uma comunicação full duplex ocupando os pinos digitais 
    11, 12 e 13 no Uno e nos pinos 50, 51 e 52 no Mega. Em ambos, o pino 10 é usado como SS (Slave Select) para Ethernet Controller e o 4 para SD Card.
* Exemplo de comando de teste do SNMP no Windows => snmpget -v:1 -t:10 -c:public -r:192.168.0.101 -o:1.3.6.1.4.1.36582.3.1.0
* Referências:
  - https://www.arduino.cc/reference/pt/
  - https://www.filipeflop.com
  - https://github.com/1sw/Agentuino
  - http://arduinoprojexp.blogspot.com/2014/11/agentuino-zabbix-parte-1-implementacao.html
  - https://pt.slideshare.net/lailtonmontenegro/monitoramento-de-temperatura-e-umidade-de-data-center-utilizando-o-arduino-e-o-sistema-zabbix
  - http://labdegaragem.com/profiles/blogs/controlando-ar-condicionado-utilizando-arduino-e-led
  - https://www.instructables.com/id/Ar-Condicionado-controlado-por-Arduino-via-infrave/
  - https://arduinolivre.wordpress.com/2012/07/31/clonando-qualquer-controle-remoto/
  - https://blog.eletrogate.com/guia-completo-do-controle-remoto-ir-receptor-ir-para-arduino/
  - https://www.arduino.cc/reference/pt/language/variables/utilities/progmem/
  - https://www.analysir.com/blog/tag/air-conditioner/
  - https://github.com/llelundberg/EasyWebServer
*/
//
// 1 - Importando Bibliotecas
#include <Streaming.h>     // Biblioteca Streaming usada em conjunto com a Ethernet
#include <Ethernet.h>      // Bliblioteca da Shield Ethernet
#include <SPI.h>           // Barramento SPI para comunicação serial do Arduino com a Ethernet Shield
#include <Agentuino.h>     // Biblioteca de criação da MIB e do agente SNMP
#include <DHT.h>           // Biblioteca do sensor de temperatura e umidade
#include <EasyWebServer.h> // Biblioteca do servidor web
//
// 2 - Define variaveis e parametros das bibliotecas e do programa
#define DHTPIN A1             // Sensor DHT conectado ao pino analógico A1
#define DHTTYPE DHT11         // Sensor DHT 11
//#define DHTTYPE DHT22         // Sensor DHT 22
DHT dht(DHTPIN, DHTTYPE);     // Grava os parametros da biblioteca DHT.h
int u = 0;                    // Cria a variável para a umidade com valor inicial igual a 0
int t = 0;                    // Cria a variável para a umidade com valor inicial igual a 0
int IRledPin = 3;             // LED Infravermelho conectado ao pino digital 3
boolean enviouIrOn = 0;       // Variável 0 e 1 (FALSE/TRUE) para informar envio de sinal IR para ligar o Ar Condicionado
int enviouIrTemp = 0;         // Variável para informar envio de sinal IR para ajustar a temperatura do Ar Condicionado
char *cmdEnviado[] = {"N/A"}; // Variável de caracteres para informar na página web o último comando enviado
boolean enviaIrAuto = 1;      // Variável 0 e 1 (FALSE/TRUE) para habilitar ou desabilitar o modo automático de controle do Ar Condicionado
boolean enviadoIr = 0;        // Variável 0 e 1 (FALSE/TRUE) para carregar a página web de sucesso no envio do comando de controle do Ar Condicionado
unsigned long tempoAnteriorUm = 0;         // Variável para contagem de tempo dos intervalos de verificação de funcionamento do Ar Condicionado 
unsigned long tempoIntervaloUm = 300000;   // Constante de tempo de intervalo para leitura da temperatura e umidade, em milisegundos (300000 ms = 5 min)
unsigned long tempoAnteriorDois = 0;       // Variável para contagem de tempo dos intervalos de controle da temperatura do Ar Condicionado 
unsigned long tempoIntervaloDois = 900000; // Constante de tempo de intervalo para leitura e controle da temperatura, em milisegundos (900000 ms = 15 min)
//unsigned long tempoAnteriorTres = 0;       // Variável para contagem de tempo dos intervalos de obtenção da temperatura e umidade
//unsigned long tempoIntervaloTres = 60000;  // Constante de tempo de intervalo para leitura da temperatura e umidade, em milisegundos (60000 ms = 1 min)
//
// define os arrays com os valores capturados de cada função do controle remoto e grava na memória flash (PROGMEM) para redução no armazenamento de dados na RAM
// entre chaves estão os valores capturados pelo receptor IR (usar o sketch em "...\Ctrl_Temp_UR\2-capturar_ir\capturar_ir_v2\capturar_ir_v2.ino" em conjunto com o módulo receptor IR KY-022 para capturar os valores)
unsigned const int liga[199] PROGMEM              = {4448, 4376, 576, 1608, 580, 540, 556, 1636, 556, 1632, 556, 540, 560, 540, 556, 1632, 556, 544, 556, 544, 556, 1632, 560, 544, 604, 488, 560, 1628, 560, 1628, 556, 540, 560, 1636, 608, 492, 556, 540, 560, 1632, 560, 1628, 608, 1580, 556, 1632, 608, 1580, 608, 1580, 560, 1636, 556, 1632, 608, 488, 560, 540, 556, 520, 628, 492, 608, 488, 556, 544, 556, 544, 608, 488, 560, 1636, 604, 492, 556, 540, 560, 540, 608, 492, 556, 540, 612, 1580, 556, 1632, 556, 540, 556, 1636, 608, 1580, 556, 1632, 556, 1632, 608, 1580, 524, 5264, 4444, 4372, 576, 1612, 576, 540, 608, 1584, 608, 1580, 556, 540, 608, 492, 556, 1632, 556, 544, 608, 492, 608, 1584, 556, 540, 560, 540, 556, 1632, 556, 1632, 556, 540, 612, 1584, 556, 544, 556, 540, 608, 1584, 560, 1628, 608, 1584, 556, 1632, 556, 1632, 556, 1632, 560, 1636, 608, 1580, 556, 544, 604, 492, 608, 488, 560, 540, 608, 492, 556, 540, 608, 492, 608, 492, 556, 1636, 608, 488, 608, 492, 556, 540, 560, 540, 604, 496, 608, 1580, 560, 1628, 560, 540, 556, 1636, 556, 1632, 556, 1632, 608, 1580, 556, 1636, 524}; // botao de ligar
unsigned const int desliga[199] PROGMEM           = {4472, 4368, 576, 1612, 576, 540, 556, 1636, 556, 1632, 556, 540, 560, 520, 576, 1632, 556, 544, 556, 544, 556, 1636, 556, 540, 560, 540, 556, 1628, 560, 1632, 556, 540, 556, 1636, 560, 540, 556, 1632, 556, 1632, 556, 1632, 556, 1632, 556, 540, 560, 1632, 560, 1632, 556, 1636, 556, 544, 556, 540, 556, 544, 556, 540, 556, 1632, 556, 540, 556, 544, 556, 1640, 556, 1628, 560, 1628, 560, 540, 556, 540, 556, 544, 556, 540, 556, 544, 556, 544, 556, 540, 560, 540, 556, 1636, 556, 1632, 556, 1612, 576, 1612, 576, 1632, 524, 5264, 4444, 4388, 560, 1628, 556, 544, 556, 1636, 556, 1628, 560, 540, 556, 544, 556, 1628, 560, 524, 576, 540, 560, 1632, 556, 544, 556, 540, 556, 1632, 556, 1632, 556, 544, 556, 1636, 556, 544, 556, 1632, 556, 1632, 556, 1632, 556, 1632, 556, 540, 556, 1636, 556, 1632, 560, 1636, 556, 540, 560, 540, 556, 540, 556, 544, 556, 1628, 560, 540, 556, 544, 556, 1636, 560, 1628, 560, 1628, 560, 540, 556, 540, 560, 540, 556, 540, 560, 540, 556, 544, 560, 540, 556, 540, 560, 1632, 556, 1632, 556, 1632, 556, 1612, 576, 1636, 524}; // botao de desligar
unsigned const int tempDezessete[199] PROGMEM     = {4408, 4412, 612, 1580, 608, 488, 612, 1580, 640, 1548, 612, 488, 612, 484, 612, 1576, 612, 492, 636, 464, 608, 1584, 608, 488, 612, 488, 608, 1580, 612, 1576, 612, 484, 612, 1584, 612, 488, 612, 488, 636, 1556, 608, 1580, 608, 1580, 608, 1580, 612, 1576, 612, 1576, 612, 1584, 612, 1576, 608, 488, 640, 460, 612, 488, 608, 488, 612, 484, 612, 488, 612, 488, 612, 488, 608, 488, 612, 488, 612, 484, 612, 488, 608, 488, 612, 488, 612, 1580, 612, 1572, 612, 1580, 608, 1580, 608, 1580, 608, 1580, 612, 1576, 612, 1576, 528, 5256, 4476, 4340, 608, 1580, 608, 488, 612, 1580, 612, 1576, 612, 488, 608, 488, 612, 1576, 612, 488, 636, 464, 612, 1580, 608, 488, 612, 488, 608, 1580, 608, 1576, 612, 488, 612, 1580, 612, 492, 608, 488, 608, 1584, 608, 1580, 636, 1548, 612, 1580, 608, 1580, 608, 1580, 612, 1584, 608, 1576, 612, 488, 612, 484, 612, 488, 608, 488, 612, 484, 612, 492, 636, 460, 612, 488, 612, 484, 612, 488, 612, 484, 612, 488, 612, 484, 612, 488, 612, 1576, 612, 1576, 612, 1576, 612, 1576, 612, 1576, 612, 1576, 612, 1576, 612, 1580, 528}; // botao de ajustar temperatura em 17 graus
unsigned const int tempDezoito[199] PROGMEM       = {4504, 4320, 632, 1576, 612, 488, 612, 1580, 608, 1580, 612, 484, 612, 488, 608, 1576, 612, 488, 612, 488, 612, 1580, 612, 488, 612, 484, 612, 1576, 612, 1576, 612, 488, 612, 1580, 612, 488, 612, 484, 616, 1556, 632, 1576, 612, 1556, 632, 1576, 612, 1576, 612, 1576, 612, 1580, 612, 1576, 612, 488, 612, 484, 612, 488, 612, 488, 608, 468, 632, 488, 612, 488, 608, 488, 612, 484, 612, 1576, 612, 488, 608, 488, 612, 468, 632, 488, 608, 1584, 612, 1576, 612, 1576, 612, 484, 612, 1580, 612, 1576, 612, 1576, 612, 1576, 524, 5264, 4500, 4332, 636, 1532, 656, 460, 612, 1580, 612, 1576, 612, 488, 612, 484, 612, 1576, 612, 488, 612, 488, 612, 1580, 608, 488, 612, 488, 612, 1572, 612, 1576, 612, 488, 612, 1580, 612, 488, 612, 488, 608, 1580, 612, 1556, 632, 1556, 632, 1576, 612, 1576, 612, 1576, 612, 1584, 608, 1560, 656, 440, 632, 464, 632, 488, 612, 484, 612, 488, 608, 492, 608, 488, 612, 488, 612, 484, 612, 1576, 612, 488, 612, 484, 612, 484, 612, 488, 612, 1564, 628, 1556, 632, 1576, 612, 488, 636, 1556, 612, 1576, 612, 1572, 612, 1580, 528}; // botao de ajustar temperatura em 18 graus
unsigned const int tempDezenove[199] PROGMEM      = {4444, 4368, 580, 1608, 576, 540, 560, 1632, 560, 1628, 560, 540, 556, 540, 560, 1628, 556, 544, 556, 544, 560, 1632, 560, 536, 560, 540, 560, 1608, 576, 1632, 556, 540, 556, 1636, 560, 540, 560, 540, 556, 1636, 556, 1632, 556, 1632, 556, 1628, 560, 1628, 560, 1632, 556, 1636, 556, 1632, 556, 544, 556, 540, 556, 540, 560, 540, 556, 540, 560, 540, 560, 540, 560, 540, 556, 1636, 556, 1628, 560, 540, 556, 540, 560, 540, 556, 544, 556, 1636, 560, 1628, 556, 540, 560, 520, 576, 1632, 556, 1632, 556, 1632, 556, 1632, 528, 5256, 4448, 4364, 580, 1628, 560, 540, 556, 1636, 556, 1628, 560, 540, 556, 540, 560, 1628, 560, 540, 556, 544, 556, 1636, 556, 540, 560, 540, 556, 1632, 556, 1628, 560, 540, 556, 1616, 580, 540, 560, 540, 556, 1636, 556, 1628, 560, 1628, 560, 1628, 560, 1628, 560, 1632, 556, 1636, 556, 1632, 560, 540, 556, 540, 556, 540, 560, 540, 556, 540, 560, 540, 560, 540, 556, 540, 560, 1632, 560, 1628, 560, 520, 576, 540, 560, 540, 556, 544, 556, 1636, 556, 1632, 556, 540, 560, 540, 560, 1628, 556, 1632, 556, 1632, 556, 1632, 532}; // botao de ajustar temperatura em 19 graus
unsigned const int tempVinte[199] PROGMEM         = {4468, 4364, 580, 1632, 556, 540, 560, 1632, 560, 1628, 556, 540, 560, 540, 556, 1628, 560, 540, 560, 540, 560, 1632, 560, 540, 556, 540, 560, 1608, 576, 1632, 556, 540, 560, 1636, 556, 544, 556, 540, 556, 1636, 556, 1632, 556, 1636, 552, 1632, 556, 1608, 580, 1632, 556, 1636, 560, 1628, 560, 540, 556, 540, 556, 540, 560, 540, 556, 540, 560, 540, 560, 520, 580, 540, 556, 1632, 560, 540, 556, 540, 560, 540, 556, 540, 560, 540, 560, 1628, 560, 1628, 560, 540, 556, 1636, 556, 1632, 556, 1628, 560, 1628, 560, 1628, 532, 5276, 4424, 4388, 560, 1628, 560, 536, 560, 1632, 556, 1612, 580, 536, 560, 540, 556, 1628, 560, 540, 560, 540, 560, 1632, 560, 540, 556, 540, 560, 1628, 556, 1632, 556, 540, 560, 1632, 560, 540, 560, 540, 556, 1612, 580, 1628, 560, 1628, 560, 1628, 560, 1628, 556, 1632, 560, 1612, 580, 1608, 580, 540, 556, 540, 560, 536, 560, 540, 560, 536, 560, 540, 560, 540, 556, 540, 560, 1632, 560, 536, 560, 520, 580, 536, 560, 540, 556, 544, 556, 1632, 556, 1632, 556, 540, 560, 1632, 560, 1628, 556, 1632, 556, 1632, 556, 1632, 532}; // botao de ajustar temperatura em 20 graus
unsigned const int tempVinteUm[199] PROGMEM       = {4388, 4412, 580, 1632, 556, 540, 556, 1636, 556, 1632, 556, 540, 560, 540, 556, 1628, 560, 540, 556, 544, 556, 1636, 556, 540, 560, 540, 556, 1632, 556, 1608, 580, 540, 556, 1636, 560, 540, 560, 540, 556, 1636, 556, 1628, 560, 1628, 560, 1628, 560, 1628, 556, 1636, 556, 1636, 556, 1632, 556, 520, 580, 540, 556, 544, 552, 540, 560, 540, 556, 524, 576, 544, 556, 1632, 556, 1628, 560, 540, 556, 540, 556, 544, 556, 540, 556, 544, 556, 1632, 560, 540, 556, 540, 556, 1632, 556, 1612, 576, 1632, 556, 1628, 560, 1628, 528, 5236, 4444, 4368, 580, 1628, 560, 536, 560, 1636, 556, 1628, 560, 540, 556, 540, 560, 1628, 556, 544, 556, 544, 556, 1636, 556, 540, 560, 540, 556, 1612, 576, 1628, 560, 540, 556, 1616, 580, 540, 560, 540, 556, 1632, 560, 1628, 560, 1608, 580, 1628, 560, 1628, 560, 1632, 556, 1636, 560, 1608, 576, 544, 556, 540, 556, 540, 560, 540, 556, 520, 580, 520, 580, 540, 556, 1632, 556, 1632, 556, 540, 556, 544, 556, 540, 556, 544, 556, 520, 580, 1632, 556, 544, 556, 540, 556, 1632, 556, 1632, 556, 1632, 556, 1612, 576, 1632, 524}; // botao de ajustar temperatura em 21 graus
unsigned const int tempVinteDois[199] PROGMEM     = {4448, 4388, 560, 1628, 560, 520, 576, 1612, 580, 1628, 560, 540, 556, 540, 556, 1632, 556, 544, 608, 492, 556, 1632, 560, 540, 556, 540, 556, 1632, 556, 1632, 556, 540, 556, 1640, 556, 540, 560, 540, 604, 1584, 560, 1628, 560, 1628, 560, 1628, 556, 1632, 556, 1632, 556, 1636, 556, 1632, 556, 540, 560, 536, 560, 540, 556, 540, 560, 540, 556, 544, 556, 520, 580, 1628, 556, 1632, 556, 1632, 556, 540, 556, 540, 560, 536, 560, 540, 560, 1636, 556, 540, 556, 540, 556, 544, 556, 1632, 560, 1628, 556, 1632, 556, 1632, 524, 5260, 4444, 4388, 556, 1612, 576, 540, 560, 1632, 556, 1632, 556, 540, 556, 540, 560, 1628, 560, 540, 560, 540, 556, 1636, 556, 540, 560, 520, 576, 1608, 580, 1628, 556, 540, 560, 1636, 556, 544, 556, 540, 556, 1636, 556, 1628, 560, 1608, 580, 1628, 556, 1632, 556, 1632, 556, 1636, 560, 1628, 556, 540, 556, 544, 556, 540, 556, 540, 608, 468, 580, 540, 560, 544, 556, 1628, 560, 1628, 560, 1628, 556, 540, 560, 540, 556, 540, 560, 540, 556, 1636, 560, 520, 576, 540, 556, 544, 556, 1632, 560, 1628, 560, 1628, 556, 1636, 524}; // botao de ajustar temperatura em 22 graus
unsigned const int tempVinteTres[199] PROGMEM     = {4440, 4384, 556, 1632, 608, 492, 556, 1632, 560, 1628, 608, 488, 608, 492, 608, 1576, 608, 492, 608, 492, 560, 1632, 556, 540, 560, 540, 556, 1628, 560, 1628, 608, 492, 556, 1636, 556, 544, 608, 488, 556, 1636, 556, 1632, 556, 1632, 604, 1580, 612, 1576, 556, 1632, 560, 1636, 556, 1632, 604, 492, 556, 540, 560, 540, 556, 540, 608, 492, 556, 540, 608, 492, 560, 1628, 560, 536, 560, 1632, 556, 544, 556, 540, 608, 488, 608, 492, 560, 1632, 560, 540, 556, 1636, 608, 488, 556, 1636, 556, 1632, 608, 1576, 560, 1628, 528, 5272, 4424, 4388, 604, 1580, 612, 488, 556, 1632, 608, 1580, 612, 488, 608, 488, 608, 1580, 556, 544, 556, 544, 556, 1636, 556, 540, 608, 492, 556, 1632, 556, 1628, 608, 492, 608, 1584, 608, 492, 608, 488, 556, 1636, 556, 1632, 556, 1628, 560, 1628, 560, 1628, 556, 1632, 608, 1588, 604, 1584, 604, 492, 556, 540, 608, 492, 608, 488, 608, 488, 560, 540, 556, 544, 560, 1628, 556, 540, 560, 1632, 556, 544, 556, 540, 556, 540, 608, 492, 608, 1584, 608, 492, 556, 1636, 608, 488, 556, 1636, 556, 1628, 608, 1580, 608, 1584, 524}; // botao de ajustar temperatura em 23 graus
unsigned const int tempVinteQuatro[199] PROGMEM   = {4416, 4416, 580, 1628, 560, 540, 556, 1636, 556, 1612, 576, 540, 560, 540, 556, 1632, 556, 544, 556, 544, 556, 1616, 580, 536, 560, 540, 556, 1612, 580, 1608, 580, 540, 556, 1636, 560, 540, 556, 544, 556, 1636, 556, 1632, 556, 1632, 556, 1632, 556, 1632, 556, 1632, 560, 1616, 576, 1632, 556, 540, 560, 540, 556, 540, 560, 540, 556, 540, 560, 540, 560, 540, 560, 1628, 556, 544, 556, 540, 556, 540, 560, 540, 556, 540, 560, 544, 556, 1636, 556, 540, 556, 1636, 560, 1628, 560, 1628, 556, 1632, 556, 1632, 556, 1632, 528, 5280, 4424, 4368, 580, 1628, 560, 540, 556, 1636, 556, 1632, 556, 540, 560, 540, 556, 1632, 556, 544, 556, 544, 560, 1632, 556, 540, 560, 540, 556, 1632, 556, 1632, 556, 540, 560, 1616, 576, 544, 556, 540, 560, 1632, 560, 1628, 560, 1628, 560, 1608, 580, 1632, 556, 1632, 560, 1636, 556, 1608, 580, 540, 560, 540, 556, 540, 560, 520, 580, 540, 556, 540, 560, 540, 560, 1632, 556, 540, 556, 540, 560, 540, 556, 544, 556, 540, 556, 544, 560, 1636, 556, 540, 560, 1632, 560, 1628, 556, 1612, 580, 1628, 560, 1608, 580, 1632, 524}; // botao de ajustar temperatura em 24 graus
unsigned int pulse; // variável usada na funcao cmdAr() para armazenar temporariamente cada valor dos arrays 
//
static byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED }; // variáveis do tipo byte usadas pela bilbioteca Ethernet.h
static byte ip[] = { 0, 0, 0, 0 };
static byte gateway[] = { 0, 0, 0, 0 };
static byte subnet[] = { 0, 0, 0, 0 };
boolean configIp = 0; // variável para informar se a configuração manual de IP foi realizada
//
EthernetServer server(80);          // seleciona a porta para o servidor web
//EthernetServer telnetserver(23); // seleciona a porta para o servidor telnet
//
// RFC1213-MIB OIDs
// .iso (.1)
// .iso.org (.1.3)
// .iso.org.dod (.1.3.6)
// .iso.org.dod.internet (.1.3.6.1)
// .iso.org.dod.internet.mgmt (.1.3.6.1.2)
// .iso.org.dod.internet.mgmt.mib-2 (.1.3.6.1.2.1)
// .iso.org.dod.internet.mgmt.mib-2.system (.1.3.6.1.2.1.1)
// .iso.org.dod.internet.mgmt.mib-2.system.sysDescr (.1.3.6.1.2.1.1.1)
const static char sysDescr[] PROGMEM      = "1.3.6.1.2.1.1.1.0";  // read-only  (DisplayString)
// .iso.org.dod.internet.mgmt.mib-2.system.sysObjectID (.1.3.6.1.2.1.1.2)
//static char sysObjectID[] PROGMEM   = "1.3.6.1.2.1.1.2.0";  // read-only  (ObjectIdentifier)
// .iso.org.dod.internet.mgmt.mib-2.system.sysUpTime (.1.3.6.1.2.1.1.3)
const static char sysUpTime[] PROGMEM     = "1.3.6.1.2.1.1.3.0";  // read-only  (TimeTicks)
// .iso.org.dod.internet.mgmt.mib-2.system.sysContact (.1.3.6.1.2.1.1.4)
const static char sysContact[] PROGMEM    = "1.3.6.1.2.1.1.4.0";  // read-write (DisplayString)
// .iso.org.dod.internet.mgmt.mib-2.system.sysName (.1.3.6.1.2.1.1.5)
const static char sysName[] PROGMEM       = "1.3.6.1.2.1.1.5.0";  // read-write (DisplayString)
// .iso.org.dod.internet.mgmt.mib-2.system.sysLocation (.1.3.6.1.2.1.1.6)
const static char sysLocation[] PROGMEM   = "1.3.6.1.2.1.1.6.0";  // read-write (DisplayString)
// .iso.org.dod.internet.mgmt.mib-2.system.sysServices (.1.3.6.1.2.1.1.7)
const static char sysServices[] PROGMEM   = "1.3.6.1.2.1.1.7.0";  // read-only  (Integer)
//
// Arduino defined OIDs
// .iso.org.dod.internet.private (.1.3.6.1.4)
// .iso.org.dod.internet.private.enterprises (.1.3.6.1.4.1)
// .iso.org.dod.internet.private.enterprises.arduino (.1.3.6.1.4.1.36582)
//
// OIDs de Temperatura e Umidade
const static char sysTemp[24] PROGMEM      = "1.3.6.1.4.1.36582.3.1.0";
const static char sysUmid[24] PROGMEM      = "1.3.6.1.4.1.36582.3.2.0";
//
// RFC1213 local values
static char locDescr[]              = "Agentuino, Agente SNMP V1.";  // read-only (static)
//static char locObjectID[]         = "1.3.6.1.3.2009.0";            // read-only (static)
static uint32_t locUpTime           = 0;                             // read-only (static)
static char locContact[24]          = "mail@dominio.com";        // should be stored/read from EEPROM - read/write (not done for simplicity)
static char locName[24]             = "Agentuino";             // should be stored/read from EEPROM - read/write (not done for simplicity)
static char locLocation[24]         = "Municipio, Estado";          // should be stored/read from EEPROM - read/write (not done for simplicity)
static int32_t locServices          = 6;                             // read-only (static)
int locTemp                         = 0;                             // cria variável para guardar os valores de temperatura na MIB SNMP
int locUmid                         = 0;                             // cria variável para guardar os valores de umidade na MIB SNMP
//
uint32_t prevMillis = millis(); // variável para o contador locUpTime
char oid[SNMP_MAX_OID_LEN];
SNMP_API_STAT_CODES api_status;
SNMP_ERR_CODES status;
//
// 3 - Executa os parâmetros de inicialização do Arduino
void setup(){
  Serial.begin(9600);        // Inicializa a serial
  pinMode(IRledPin, OUTPUT); // Define modo do pino 3 como saída
  dht.begin();               // Inicializa a biblioteca DHT.h
  netConfig();               // inicia conexão Ethernet
  server.begin();            // inicializa o servidor web
  //telnetserver.begin();      // inicializa o servidor telnet
  Serial.println ();
  Serial.println (F("Servidor Telnet Ok...")); // imprime mensagem na Serial
  Serial.println (F("Servidor Web Ok..."));
//
  api_status = Agentuino.begin(); // inicializa o agente SNMP
  if ( api_status == SNMP_API_STAT_SUCCESS ) {
    Agentuino.onPduReceive(pduReceived);
    delay(10);
    Serial.println (F("Agente SNMP Ok..."));
    return;
  } /*else {
    delay(10);
    Serial.print (F("Agente SNMP Falhou..."));
  }*/
}
//
// 4 - Executa a função loop
void loop(){
  // 4.1 - obtém a umidade e temperatura para o agente SNMP e para o programa
  u = dht.readHumidity();    // grava o valor atual da umidade na variável u
  t = dht.readTemperature(); // grava o valor atual da temperatura na variável t
  Agentuino.listen();        // escuta solicitações SNMP recebidas
  locTemp = t;               // grava o valor de t para o agente SNMP
  locUmid = u;               // grava o valor de u para o agente SNMP
//
  // sysUpTime - tempo (em centésimos de segundo) de atividade do Arduino
  if ( millis() - prevMillis > 1000 ) {
    prevMillis += 1000; // incrementa millisegundos anteriores
    locUpTime += 100;   // incrementa contador de tempo no SNMP
  }
  // 4.2 - escuta conexões telnet na porta 23 (se habilitado) e HTTP na porta 80
  //EthernetClient telnetclient = telnetserver.available();
  server.begin();            // reinicia o servidor web para liberação dos sockets TCP
  EthernetClient client = server.available();
  if (client){ //caso o cliente esteja conectado
    //telnetclient.println(F("Cliente web conectado"));
    EasyWebServer w(client);                    // Lê e analisa a solicitação HTTP
    w.serveUrl("/",rootPage);                   // hospeda a página principal
    w.serveUrl("/naoauto",naoAutoPage);         // hospeda a página que define o modo manual de controle do Ar Condicionado
    w.serveUrl("/setauto",setAutoPage);         // hospeda a página que define o modo automático de controle do Ar Condicionado
    w.serveUrl("/ligar",ligarArPage);           // esta e as próximas linhas hospedam as páginas que executam os respectivos comandos de controle do Ar Condicionado
    w.serveUrl("/desligar",desligarArPage);
    w.serveUrl("/temp17",temp17Page);
    w.serveUrl("/temp18",temp18Page);
    w.serveUrl("/temp19",temp19Page);
    w.serveUrl("/temp20",temp20Page);
    w.serveUrl("/temp21",temp21Page);
    w.serveUrl("/temp22",temp22Page);
    w.serveUrl("/temp23",temp23Page);
    w.serveUrl("/temp24",temp24Page);
    delay(1); // dar tempo ao navegador para receber os dados
    client.stop(); // fecha a conexao
    //telnetclient.println();
    //telnetclient.println(F("Cliente web desconetado"));
  }
  // 4.3 - verifica se o ar condicionado está ligado
  if ((millis() - tempoAnteriorUm) > tempoIntervaloUm && enviouIrOn == 0 && enviaIrAuto == 1){ // verifica se o tempo decorrido é maior que 5 min (300000 ms), se o sinal de ligar o Ar Condicionado foi enviado e se foi desabilitado o modo automático
    if (t > 25){ // verifica se a temperatura obtida é maior que 25 graus
      //telnetclient.println(F("Ligando Ar...")); // envia msg via telnet
      cmdAr(0); // executa a função de envio do sinal IR com o parâmetro (0) de ligar
      enviouIrOn = 1; // define 1 na variável para informar à próxima condição que o sinal de ligar foi enviado
      cmdEnviado[0] = "On"; // grava na variável o comando enviado para disponibilizar na página web
      //telnetclient.println(F("Monitorando temperatura...")); // envia msg via telnet
    }
    else {
      //telnetclient.println(F("Ar condicionado: LIGADO")); // envia msg via telnet
    }
    tempoAnteriorUm = millis(); // atualiza o tempo em que o loop está sendo executado. Esta instrução é necessária para que a operacao matemática na primeira condição e na seguinte façam sentido zerando os contadores de 5 e de 10 minutos
  }
  if ((enviouIrOn == 1) && ((millis() - tempoAnteriorUm) > 600000)){ // verifica se o sinal de ligar foi enviado e inicia o contador de 10 min (600000 ms) aguardando a temperatura ambiente normalizar
    enviouIrOn = 0; // reinicia a informacao de envio de sinal
  }
//
  // 4.4 - controla a potência do ar condicionado conforme a temperatura ambiente detectada
  if ((millis() - tempoAnteriorDois) > tempoIntervaloDois && enviouIrTemp == 0 && enviaIrAuto == 1){ // verifica se o tempo decorrido é maior que 15 min (900000 ms), se o sinal de ajustar o ar condicionado foi enviado e se o modo automático está habilitado
    //telnetclient.print("Verificando Temperatura: ");
    if (t >= 19 && t <= 22){ // temperatura normal
      //telnetclient.println(F("NORMAL"));
    }
    else if (t < 19){ // temperatura baixa
      //telnetclient.println(F("BAIXA"));
      cmdAr(6);
      enviouIrTemp = 1;
      cmdEnviado[0] = "21ºC";
      //telnetclient.println(F("21ºC enviado"));
    }
    else if (t > 22){ // temperatura alta
      //telnetclient.println(F("ALTA"));
      cmdAr(5);
      enviouIrTemp = 1;
      cmdEnviado[0] = "20ºC";
      //telnetclient.println(F("20ºC enviado"));
    }
    tempoAnteriorDois = millis();
  }
  if ((enviouIrTemp == 1) && ((millis() - tempoAnteriorDois) > 600000)){ // verifica se o primeiro sinal de ajuste já foi enviado, aguarda ultrapassar o contador de 10 min (600000 ms) para a temperatura ambiente normalizar e envia novo comando caso necessário
    if (t < 19){
      cmdAr(7);
      enviouIrTemp = 2;
      cmdEnviado[0] = "22ºC";
      //telnetclient.println(F("22ºC enviado"));
    }
    else if (t > 22){
      cmdAr(4);
      enviouIrTemp = 2;
      cmdEnviado[0] = "19ºC";
      //telnetclient.println(F("19ºC enviado"));
    }
    tempoAnteriorDois = millis();
  }
  if ((enviouIrTemp == 2) && ((millis() - tempoAnteriorDois) > 600000)){ // verifica se o segundo sinal de ajuste já foi enviado, aguarda ultrapassar o contador de 10 min (600000 ms) para a temperatura ambiente normalizar e envia novo comando caso necessário
    if (t < 19){
      cmdAr(8);
      enviouIrTemp = 3;
      cmdEnviado[0] = "23ºC";
      //telnetclient.println(F("23ºC enviado"));
    }
    else if (t > 22){
      cmdAr(3);
      enviouIrTemp = 4;
      cmdEnviado[0] = "18ºC";
      //telnetclient.println(F("18ºC enviado"));
    }
    tempoAnteriorDois = millis();
  }
  if ((enviouIrTemp == 3) && ((millis() - tempoAnteriorDois) > 600000)){ // verifica se o sinal de 23ºC já foi enviado, aguarda ultrapassar o contador de 10 min (600000 ms) para a temperatura ambiente normalizar e envia novo comando caso necessário
    if (t < 19){
      cmdAr(9);
      cmdEnviado[0] = "24ºC";
      //telnetclient.println(F("24ºC enviado"));
      tempoAnteriorDois = millis();
    }
    else if (t > 22){
        enviouIrTemp = 0; // retorna para a primeira condição caso a temperatura no ambiente fique elevada
    }
  }
  if ((enviouIrTemp == 4) && ((millis() - tempoAnteriorDois) > 600000)){ // verifica se o sinal de 18ºC já foi enviado, aguarda ultrapassar o contador de 10 min (600000 ms) para a temperatura ambiente normalizar e envia novo comando caso necessário  
    if (t > 22){
      cmdAr(2);
      cmdEnviado[0] = "17ºC";
      //telnetclient.println(F("17ºC enviado"));
      tempoAnteriorDois = millis();
    }
    else if (t < 19){
        enviouIrTemp = 0; // retorna para a primeira condição caso a temperatura no ambiente fique baixa
      }
  }
//
  // 4.5 - obtém a umidade e temperatura de 1 em 1 minuto (60000 ms) e imprime na serial
  /*if ((millis() - tempoAnteriorTres) > tempoIntervaloTres){
    //telnetclient.print(F("Umidade: "));
    //telnetclient.print(u); // imprime a umidade no telnet
    //telnetclient.print(F("%  "));
    //telnetclient.print(F("Temperatura: "));
    //telnetclient.print(t); // imprime a temperatura no telnet
    //telnetclient.println(F("ºC"));
    tempoAnteriorTres = millis();
  }*/
}
//
// 5 - Funções utilizadas no programa
// 5.1 - Inicializa a Shield Ethernet por DHCP ou IP fixo
void netConfig(){
  Serial.println(F("Obtendo IP..."));
  if (Ethernet.begin(mac) == 0){ // caso o DHCP não esteja disponível
    Serial.println(F("DHCP Falhou"));
    delay(1000);
    while (!Serial){
    ; // aguarda a conexão serial para configurar manualmente os endereços IP, Gateway e Máscara
    }//*
    Serial.println(F("Informe IP, Gateway e Mascara:"));
    while (configIp == 0){
      if (Serial.available() > 0) { // em conjunto com o while, aguarda inserção de informação na Serial
        ip[0] = Serial.parseInt();  // procura o próximo inteiro válido no buffer de recebimento serial e grava em cada elemento dos arrays abaixo (ip, gateway e subnet)
        ip[1] = Serial.parseInt();
        ip[2] = Serial.parseInt();
        ip[3] = Serial.parseInt();
        gateway[0] = Serial.parseInt();
        gateway[1] = Serial.parseInt();
        gateway[2] = Serial.parseInt();
        gateway[3] = Serial.parseInt();
        subnet[0] = Serial.parseInt();
        subnet[1] = Serial.parseInt();
        subnet[2] = Serial.parseInt();
        subnet[3] = Serial.parseInt();
      }
      if (ip[0] != 0){ // sai do loop while
        configIp = 1;
      }
    }
    Ethernet.begin(mac, ip, gateway, subnet); // inicia a conexão Ethernet sem DHCP
    /*// imprime os valores de IP definidos na Serial
    Serial.print(F("IP: "));
    Serial.println(Ethernet.localIP()); // função da biblioteca Ethernet para informar o IP atribuído
    Serial.print(F("Gateway: "));
    // como as funções Ethernet.gatewayIP e Ethernet.subnetMask da biblioteca Ethernet não funcionam adequadamente
    // para impressão dos valores quando os IPs são configurados manualmente, foi implementado a função de controle 
    // "for" para imprimir na Serial os valores configurados
    for(int i = 0; i < 4; i++){
      Serial.print(gateway[i]);
      if (i < 3){
        Serial.print(F("."));
      }
    }
    Serial.println();
    Serial.print(F("Mascara: "));
    for(int i = 0; i < 4; i++){
      Serial.print(subnet[i]);
      if (i < 3){
        Serial.print(F("."));
      }
    }
    Serial.println();*/
  }
  Serial.print(F("IP: "));
  Serial.println(Ethernet.localIP());
}
//
// 5.2 - Cria o PDU SNMP
void pduReceived(){
  SNMP_PDU pdu;
  api_status = Agentuino.requestPdu(&pdu);
  //
  if ((pdu.type == SNMP_PDU_GET || pdu.type == SNMP_PDU_GET_NEXT || pdu.type == SNMP_PDU_SET)
    && pdu.error == SNMP_ERR_NO_ERROR && api_status == SNMP_API_STAT_SUCCESS ) {
    //
    pdu.OID.toString(oid);
    // Implementation SNMP GET NEXT
    if ( pdu.type == SNMP_PDU_GET_NEXT ) {
      char tmpOIDfs[SNMP_MAX_OID_LEN];
      if ( strcmp_P( oid, sysDescr ) == 0 ) {  
        strcpy_P ( oid, sysUpTime ); 
        strcpy_P ( tmpOIDfs, sysUpTime );        
        pdu.OID.fromString(tmpOIDfs);  
      } else if ( strcmp_P(oid, sysUpTime ) == 0 ) {  
        strcpy_P ( oid, sysContact );  
        strcpy_P ( tmpOIDfs, sysContact );        
        pdu.OID.fromString(tmpOIDfs);          
      } else if ( strcmp_P(oid, sysContact ) == 0 ) {  
        strcpy_P ( oid, sysName );  
        strcpy_P ( tmpOIDfs, sysName );        
        pdu.OID.fromString(tmpOIDfs);                  
      } else if ( strcmp_P(oid, sysName ) == 0 ) {  
        strcpy_P ( oid, sysLocation );  
        strcpy_P ( tmpOIDfs, sysLocation );        
        pdu.OID.fromString(tmpOIDfs);                  
      } else if ( strcmp_P(oid, sysLocation ) == 0 ) {  
        strcpy_P ( oid, sysServices );  
        strcpy_P ( tmpOIDfs, sysServices );        
        pdu.OID.fromString(tmpOIDfs);                  
      } else if ( strcmp_P(oid, sysServices ) == 0 ) {  
        strcpy_P ( oid, "1.0" );  
      } else {
          int ilen = strlen(oid);
          if ( strncmp_P(oid, sysDescr, ilen ) == 0 ) {
            strcpy_P ( oid, sysDescr ); 
            strcpy_P ( tmpOIDfs, sysDescr );        
            pdu.OID.fromString(tmpOIDfs); 
          } else if ( strncmp_P(oid, sysUpTime, ilen ) == 0 ) {
            strcpy_P ( oid, sysUpTime ); 
            strcpy_P ( tmpOIDfs, sysUpTime );        
            pdu.OID.fromString(tmpOIDfs); 
          } else if ( strncmp_P(oid, sysContact, ilen ) == 0 ) {
            strcpy_P ( oid, sysContact ); 
            strcpy_P ( tmpOIDfs, sysContact );        
            pdu.OID.fromString(tmpOIDfs); 
          } else if ( strncmp_P(oid, sysName, ilen ) == 0 ) {
            strcpy_P ( oid, sysName ); 
            strcpy_P ( tmpOIDfs, sysName );        
            pdu.OID.fromString(tmpOIDfs);   
          } else if ( strncmp_P(oid, sysLocation, ilen ) == 0 ) {
            strcpy_P ( oid, sysLocation ); 
            strcpy_P ( tmpOIDfs, sysLocation );        
            pdu.OID.fromString(tmpOIDfs);    
            } else if ( strncmp_P(oid, sysServices, ilen ) == 0 ) {
            strcpy_P ( oid, sysServices ); 
            strcpy_P ( tmpOIDfs, sysServices );        
            pdu.OID.fromString(tmpOIDfs);  
            }
      }
    }
    // End of implementation SNMP GET NEXT / WALK
    if ( strcmp_P(oid, sysDescr ) == 0 ) {
      // handle sysDescr (set/get) requests
      if ( pdu.type == SNMP_PDU_SET ) {
        // response packet from set-request - object is read-only
        pdu.type = SNMP_PDU_RESPONSE;
        pdu.error = SNMP_ERR_READ_ONLY;
      } else {
        // response packet from get-request - locDescr
        status = pdu.VALUE.encode(SNMP_SYNTAX_OCTETS, locDescr);
        pdu.type = SNMP_PDU_RESPONSE;
        pdu.error = status;
      }
      //
    } else if ( strcmp_P(oid, sysUpTime ) == 0 ) {
      // handle sysName (set/get) requests
      if ( pdu.type == SNMP_PDU_SET ) {
        // response packet from set-request - object is read-only
        pdu.type = SNMP_PDU_RESPONSE;
        pdu.error = SNMP_ERR_READ_ONLY;
      } else {
        // response packet from get-request - locUpTime
        status = pdu.VALUE.encode(SNMP_SYNTAX_TIME_TICKS, locUpTime);       
        pdu.type = SNMP_PDU_RESPONSE;
        pdu.error = status;
      }
      //
    } else if ( strcmp_P(oid, sysName ) == 0 ) {
      // handle sysName (set/get) requests
      if ( pdu.type == SNMP_PDU_SET ) {
        // response packet from set-request - object is read/write
        status = pdu.VALUE.decode(locName, strlen(locName)); 
        pdu.type = SNMP_PDU_RESPONSE;
        pdu.error = status;
      } else {
        // response packet from get-request - locName
        status = pdu.VALUE.encode(SNMP_SYNTAX_OCTETS, locName);
        pdu.type = SNMP_PDU_RESPONSE;
        pdu.error = status;
      }
      //
    } else if ( strcmp_P(oid, sysContact ) == 0 ) {
      // handle sysContact (set/get) requests
      if ( pdu.type == SNMP_PDU_SET ) {
        // response packet from set-request - object is read/write
        status = pdu.VALUE.decode(locContact, strlen(locContact)); 
        pdu.type = SNMP_PDU_RESPONSE;
        pdu.error = status;
      } else {
        // response packet from get-request - locContact
        status = pdu.VALUE.encode(SNMP_SYNTAX_OCTETS, locContact);
        pdu.type = SNMP_PDU_RESPONSE;
        pdu.error = status;
      }
      //
    } else if ( strcmp_P(oid, sysLocation ) == 0 ) {
      // handle sysLocation (set/get) requests
      if ( pdu.type == SNMP_PDU_SET ) {
        // response packet from set-request - object is read/write
        status = pdu.VALUE.decode(locLocation, strlen(locLocation)); 
        pdu.type = SNMP_PDU_RESPONSE;
        pdu.error = status;
      } else {
        // response packet from get-request - locLocation
        status = pdu.VALUE.encode(SNMP_SYNTAX_OCTETS, locLocation);
        pdu.type = SNMP_PDU_RESPONSE;
        pdu.error = status;
      }
      //
    } else if ( strcmp_P(oid, sysServices) == 0 ) {
      // handle sysServices (set/get) requests
      if ( pdu.type == SNMP_PDU_SET ) {
        // response packet from set-request - object is read-only
        pdu.type = SNMP_PDU_RESPONSE;
        pdu.error = SNMP_ERR_READ_ONLY;
      } else {
        // response packet from get-request - locServices
        status = pdu.VALUE.encode(SNMP_SYNTAX_INT, locServices);
        pdu.type = SNMP_PDU_RESPONSE;
        pdu.error = status;
      }
      //
    } else if ( strcmp_P(oid, sysTemp) == 0 ) {
      // handle sysLocation (set/get) requests
      if ( pdu.type == SNMP_PDU_SET ) {
        // response packet from set-request - object is read/write
        status = pdu.VALUE.decode(locTemp, strlen(locTemp)); 
        pdu.type = SNMP_PDU_RESPONSE;
        pdu.error = status;
      } else {
        // response packet from get-request - locLocation
        status = pdu.VALUE.encode(SNMP_SYNTAX_INT, locTemp);
        pdu.type = SNMP_PDU_RESPONSE;
        pdu.error = status;
      }
      //
    } else if ( strcmp_P(oid, sysUmid) == 0 ) {
      // handle sysLocation (set/get) requests
      if ( pdu.type == SNMP_PDU_SET ) {
        // response packet from set-request - object is read/write
        status = pdu.VALUE.decode(locUmid, strlen(locUmid)); 
        pdu.type = SNMP_PDU_RESPONSE;
        pdu.error = status;
      } else {
        // response packet from get-request - locLocation
        status = pdu.VALUE.encode(SNMP_SYNTAX_INT, locUmid);
        pdu.type = SNMP_PDU_RESPONSE;
        pdu.error = status;
      }
      //
    } else {
      // oid does not exist
      // response packet - object not found
      pdu.type = SNMP_PDU_RESPONSE;
      pdu.error = SNMP_ERR_NO_SUCH_NAME;
    }
    //
    Agentuino.responsePdu(&pdu);
  }
  //
  Agentuino.freePdu(&pdu);
  //
}
//
//5.3 Funções da biblioteca EasyWebServer que "imprimem" as linhas do código HTML da página
void rootPage(EasyWebServer &w){ // página raiz "/"
  // código HTML para a biblioteca EasyWebServer
  w.client.println(F("<!DOCTYPE HTML>"
    "<html><center><img src='http://...'/></br>"
    "<head><title>Controle de Ar Condicionado</title>"
    "<meta name='viewport' content='width=device-width, initial-scale=1'></head>"
    "<script>function envia(link) {location.href = link;}</script><body>"
    "<font size='2'>Serviço de Tecnologia da Informação</font></br>"
    "<p><h2>Controle de Ar Condicionado</h2></p>"
    "<p style='line-height: 24px'><b>Local: CPD/STI</b><br>")); // informar a localização ex: Local: CPD/STI
  w.client.print(F("Temperatura: <b>"));
  w.client.print(t);
  w.client.print(F("ºC</b> &nbsp; "
    "Umidade: <b>"));
  w.client.print(u);
  w.client.println(F("%</b><br>Comando enviado: "));
  w.client.print(cmdEnviado[0]);
  w.client.print("</p>");
  if (enviaIrAuto == 1){
    w.client.println(F("<p>Modo Automático: <b>ATIVADO</b> &nbsp; "
      "<a href='/naoauto'><button>Desativar</button></a>"
      "</body></center></html>"));
  }
  else if (enviaIrAuto == 0){
    if (enviadoIr == 0){
      w.client.println(F("<p>Modo Automático: <b>DESATIVADO</b> &nbsp; "
        "<a href='/setauto'><button>Ativar</button></a>"
        "<p style='line-height: 24px'>Comandos:</br>"
        "<a href='/ligar'><button>On</button></a> &nbsp; "
        "<a href='/desligar'><button>Off</button></a>"
        "<form method='post' action='resposta.asp'><select name='escolha' size='1'>"
        "<option value='/temp17'>17ºC</option><option value='/temp18'>18ºC</option><option value='/temp19'>19ºC</option><option value='/temp20'>20ºC</option>"
        "<option value='/temp21'>21ºC</option><option value='/temp22'>22ºC</option><option value='/temp23'>23ºC</option><option value='/temp24'>24ºC</option></select> &nbsp; "
        "<input type='button' name='Btenvia' value='Enviar' onclick='envia(this.form.escolha.value);'>"
        "</p></body></center></html>"));
    }
    else{
      enviadoIr = 0;
      w.client.println(F("<meta http-equiv='refresh' content='2; /'>"
        "<br/>"));
      w.client.print(F("<p>Sucesso!</p>"));
    }
  }
}
//
void naoAutoPage(EasyWebServer &w){ // página "/naoauto"
  enviaIrAuto = 0;
  enviouIrOn = 0;
  enviouIrTemp = 0;
  cmdEnviado[0] = "N/A";
  w.client.println("<meta http-equiv='refresh' content='0; /'>");
}
//
void setAutoPage(EasyWebServer &w){ // página "/setauto"
  enviaIrAuto = 1;
  tempoAnteriorUm = millis();
  tempoAnteriorDois = millis();
  cmdEnviado[0] = "N/A";
  w.client.println("<meta http-equiv='refresh' content='0; /'>");
}
//
// páginas que acionam as funções de envio dos sinais IR
void ligarArPage(EasyWebServer &w){
  cmdAr(0);
  enviadoIr = 1;
  cmdEnviado[0] = "On";
  w.client.println("<meta http-equiv='refresh' content='0; /'>");
}
//
void desligarArPage(EasyWebServer &w){
  cmdAr(1);
  enviadoIr = 1;
  cmdEnviado[0] = "Off";
  w.client.println("<meta http-equiv='refresh' content='0; /'>");
}
//
void temp17Page(EasyWebServer &w){
  cmdAr(2);
  enviadoIr = 1;
  cmdEnviado[0] = "17ºC";
  w.client.println("<meta http-equiv='refresh' content='0; /'>");
}
//
void temp18Page(EasyWebServer &w){
  cmdAr(3);
  enviadoIr = 1;
  cmdEnviado[0] = "18ºC";
  w.client.println("<meta http-equiv='refresh' content='0; /'>");
}
//
void temp19Page(EasyWebServer &w){
  cmdAr(4);
  enviadoIr = 1;
  cmdEnviado[0] = "19ºC";
  w.client.println("<meta http-equiv='refresh' content='0; /'>");
}
//
void temp20Page(EasyWebServer &w){
  cmdAr(5);
  enviadoIr = 1;
  cmdEnviado[0] = "20ºC";
  w.client.println("<meta http-equiv='refresh' content='0; /'>");
}
//
void temp21Page(EasyWebServer &w){
  cmdAr(6);
  enviadoIr = 1;
  cmdEnviado[0] = "21ºC";
  w.client.println("<meta http-equiv='refresh' content='0; /'>");
}
//
void temp22Page(EasyWebServer &w){
  cmdAr(7);
  enviadoIr = 1;
  cmdEnviado[0] = "22ºC";
  w.client.println("<meta http-equiv='refresh' content='0; /'>");
}
//
void temp23Page(EasyWebServer &w){
  cmdAr(8);
  enviadoIr = 1;
  cmdEnviado[0] = "23ºC";
  w.client.println("<meta http-equiv='refresh' content='0; /'>");
}
//
void temp24Page(EasyWebServer &w){
  cmdAr(9);
  enviadoIr = 1;
  cmdEnviado[0] = "24ºC";
  w.client.println("<meta http-equiv='refresh' content='0; /'>");
}
//
// 5.4 Funções para controlar o Ar Condicionado
// as funções de controle abaixo obtém os elementos do respectivo array gravado na memória flash do Arduino e executa os ciclos de pulsos e de delays
// pelo tempo (em microsegundos) determinado em cada elemento deste array e executado de forma repetida até até alcançar o 198º elemento
void cmdAr(byte irSinal){
  for(byte i = 0; i < 199; i++){ // a cada rotina do for é gravado na variavel i o próximo número começando do 0 até 198
    switch (irSinal){
      case 0:
        pulse = pgm_read_word_near(liga + i); // busca o elemento do array na memoria flash do Arduino e grava na variável pulse
        break;
      case 1:
        pulse = pgm_read_word_near(desliga + i);
        break;
      case 2:
        pulse = pgm_read_word_near(tempDezessete + i);
        break;
      case 3:
        pulse = pgm_read_word_near(tempDezoito + i);
        break;
      case 4:
        pulse = pgm_read_word_near(tempDezenove + i);
        break;
      case 5:
        pulse = pgm_read_word_near(tempVinte + i);
        break;
      case 6:
        pulse = pgm_read_word_near(tempVinteUm + i);
        break;
      case 7:
        pulse = pgm_read_word_near(tempVinteDois + i);
        break;
      case 8:
        pulse = pgm_read_word_near(tempVinteTres + i);
        break;
      case 9:
        pulse = pgm_read_word_near(tempVinteQuatro + i);
        break;
    }
    if (i % 2 == 0){  // condição usada para executar a função pulseIR com a variável pulse somente quando o determinado elemento
      pulseIR(pulse); // gravado na variável i for divisível por 2 (resto 0), ou seja, quando estiver na posição par do array
    }
    else{ // se não tiver resto zero, a posição é ímpar
      delayMicroseconds(pulse);
    }
  }
}
//
// a função abaixo é chamada durante a execução das funcoes de pulso para o efetivo envio dos sinais infravermelho
// o LED acende e apaga repetidamente em um intervalo de tempo medido em microsegundos (1 seg = 1.000.000 microseg) e o ar condicionado detecta e interpreta
// as piscadas do led infravermelho acionando os comandos respectivos
void pulseIR(long microsecs){
  cli(); // desativa quaisquer interrupções
  while (microsecs > 0){
    // Em 38 kHz, 1 Hz tem cerca de 13 microssegundos em nível lógico alto e 13 microssegundos em baixo
    digitalWrite(IRledPin, HIGH);  // liga o LED. A execução desta linha leva cerca de 3 microsegundos
    delayMicroseconds(10);         // aguarda por 10 microseconds
    digitalWrite(IRledPin, LOW);   // desliga o LED. A execução desta linha leva cerca de 3 microsegundos
    delayMicroseconds(10);         // aguarda por 10 microseconds
    microsecs -= 26; // contador regressivo. Como cada ciclo (1 Hz) acima leva 26 microsegundos para ser executado, o while é executado até a condição não ser mais atendida
  }
  sei(); // habilita interrupções
}


