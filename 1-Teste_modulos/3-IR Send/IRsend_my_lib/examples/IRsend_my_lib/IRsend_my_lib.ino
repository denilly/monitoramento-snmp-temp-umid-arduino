#include <IRsend.h>

// Instancia um objeto chamado LED no pino 3
IRsend IRsend(3);

void setup(){
}

void loop()
{
 IRsend.liga();    // liga o aparelho
 delay(3000);      // aguarda 3 segundos
 IRsend.desliga(); // desliga o aparelho
 delay(3000);      // aguarda 3 segundos
 IRsend.aumenta(); // aumenta o volume
 delay(3000);      // aguarda 3 segundos
 IRsend.diminui(); // diminui o volume
 delay(3000);      // aguarda 3 segundos
}
