#include <IRremote.h>

#define ligar 0x18010000 //Codigo do botao power da tv samsung
#define volume 0xE0E0E01F //Codigo do botao volume+ da tv samsung

IRsend irsend;

void setup()
{
}

void loop() {
  irsend.sendSAMSUNG(ligar, 32); 
  delay (5000); // Cria um atraso entre cada envio
}
