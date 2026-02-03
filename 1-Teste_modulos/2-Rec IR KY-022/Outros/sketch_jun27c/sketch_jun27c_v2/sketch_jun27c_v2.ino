// Programa : Teste Controle Remoto IR  
// Autor : Arduino e Cia  
  
#include <IRremote.h>

int RECV_PIN = 11;  
int armazenavalor = 0;  
int pinoledvermelho = 5;  
int pinoledverde = 7;
boolean estadoledverde = 0;
boolean estadoledvermelho = 0;

IRrecv irrecv(RECV_PIN);
decode_results results;  
  
void setup()  
{  
  pinMode(pinoledvermelho, OUTPUT);   
  pinMode(pinoledverde, OUTPUT);  
  Serial.begin(9600);  
  irrecv.enableIRIn(); // Inicializa o receptor IR  
  digitalWrite(pinoledverde, estadoledverde);
  digitalWrite(pinoledvermelho, estadoledvermelho);
}  
   
void loop()  
{  
 if (irrecv.decode(&results))
 {
  Serial.print("Protocolo: ");
  Serial.println(results.decode_type, DEC);
  Serial.print("Valor lido : ");
  Serial.println(results.value, HEX);
  armazenavalor = (results.value);
  switch (armazenavalor)
  {
   case 0xE0E046B9:
    estadoledverde = !estadoledverde;
    digitalWrite(pinoledverde, estadoledverde);  //Acende/Apaga o led verde
    break;

   case 0xE0E0A659:
    estadoledvermelho = !estadoledvermelho;
    digitalWrite(pinoledvermelho, estadoledvermelho);  //Acende/Apaga o led vermelho
    break;

   case 0xE0E016E9:
    if (estadoledverde == 0 && estadoledvermelho == 0)
    {
     estadoledverde = !estadoledverde;
     estadoledvermelho = !estadoledvermelho;
     digitalWrite(pinoledverde, estadoledverde);  //Acende/Apaga o led verde
     digitalWrite(pinoledvermelho, estadoledvermelho);  //Acende/Apaga o led vermelho
    }
    else if (estadoledverde == 1 && estadoledvermelho == 0)
    {
     estadoledvermelho = !estadoledvermelho;
     digitalWrite(pinoledverde, estadoledverde);  //Acende/Apaga o led verde
     digitalWrite(pinoledvermelho, estadoledvermelho);  //Acende/Apaga o led vermelho 
    }
    else if (estadoledverde == 0 && estadoledvermelho == 1)
    {
     estadoledverde = !estadoledverde;
     digitalWrite(pinoledverde, estadoledverde);  //Acende/Apaga o led verde
     digitalWrite(pinoledvermelho, estadoledvermelho);  //Acende/Apaga o led vermelho 
    }
    else if (estadoledverde == 1 && estadoledvermelho == 1)
    {
     estadoledverde = !estadoledverde;
     estadoledvermelho = !estadoledvermelho;
     digitalWrite(pinoledverde, estadoledverde);  //Acende/Apaga o led verde
     digitalWrite(pinoledvermelho, estadoledvermelho);  //Acende/Apaga o led vermelho 
    }
    break;
  }
  irrecv.resume(); //Le o próximo valor
 }
}
