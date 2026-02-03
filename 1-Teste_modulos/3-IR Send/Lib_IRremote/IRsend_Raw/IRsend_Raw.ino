#include <IRremote.h> // controle remoto IR
IRsend irsend; // constante usada pela biblioteca IRremote


void setup() {

}

void loop() {
ligarAr();
delay(5000);

}

void ligarAr(){
  int khz = 38; // frenquencia usada pelo controle remoto (38 KHz)
  unsigned int irSignal[] = {4550,4400, 600,1600, 650,1600, 650,1600, 550,550, 600,500, 600,500, 650,450, 600,550, 600,1600, 650,1600, 600,1600, 600,550, 600,500, 600,500, 600,500, 600,550, 550,1650, 600,1600, 600,550, 550,1650, 600,500, 600,550, 550,550, 600,500, 600,500, 600,550, 550,1650, 600,500, 600,1650, 550,1650, 650,1600, 550,1650, 600}; // sinal decodificado do controle remoto original
  irsend.sendRaw(irSignal, sizeof(irSignal) / sizeof(irSignal[0]), khz); // envia o sinal via IR para o ar condicionado
}
