
#include <SPI.h>
#include <Ethernet.h>
#include "DHT.h"

#define DHTPIN A1 // pino que estamos conectado
#define DHTTYPE DHT11 // DHT 11

DHT dht(DHTPIN, DHTTYPE);

//Definicoes de IP, mascara de rede e gateway
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192,168,0,110);          //Define o endereco IP
IPAddress gateway(192,168,0,1);     //Define o gateway
IPAddress subnet(255, 255, 255, 0); //Define a máscara de rede
 
//Inicializa o servidor web na porta 80
EthernetServer server(80);

void setup()
{
  //Inicializa a interface de rede
  Ethernet.begin(mac, ip, gateway, subnet);
  server.begin();

  Serial.begin(9600);
  dht.begin();
}
 
void loop() {
  float u = dht.readHumidity();
  float t = dht.readTemperature();
  // testa se retorno é valido, caso contrário algo está errado.
  if (isnan(t) || isnan(u)) 
  {
    Serial.println("Failed to read from DHT");
  } 
  else
  {
    Serial.print("Umidade: ");
    Serial.print(u);
    Serial.print(" %    ");
    Serial.print("Temperatura: ");
    Serial.print(t);
    Serial.println(" *C");
    delay (2000);
  }
 
  //Aguarda conexao do browser
  EthernetClient client = server.available();
  if (client) {
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == 'n' && currentLineIsBlank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");
          client.println("Refresh: 5"); //Recarrega a pagina a cada 2seg
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          //Configura o texto e imprime o titulo no browser
          client.print("<font color=#FF0000><b><u>");
          client.print("Envio de informacoes pela rede utilizando Arduino");
          client.print("</u></b></font>");
          client.println("<br />");
          client.println("<br />");
          //Mostra as informacoes lidas pelo sensor
          client.print("Umidade : ");
          client.print("<b>");
          client.print(u);
          client.print(" %");
          client.println("</b>");
          client.println("<br />");
          client.print("Temperatura : ");
          client.print("<b>");
          client.print(t);
          client.print(" *C");
          client.println("</b></html>");
          break;
        }
        if (c == 'n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } 
        else if (c != 'r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    }
}
