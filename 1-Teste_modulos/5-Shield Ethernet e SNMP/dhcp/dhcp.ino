
#include <SPI.h>
#include <Ethernet.h>

static byte mac[] = { 0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02 };
static byte ip[] = { 0, 0, 0, 0 };
static byte gateway[] = { 0, 0, 0, 0 };
static byte subnet[] = { 0, 0, 0, 0 };
bool configIp = 0;

void setup() {
  Serial.begin(9600);
  // iniciando conexão Ethernet com DHCP
  Serial.println("Obtendo IP por DHCP...");
  if (Ethernet.begin(mac) == 0) { // caso o DHCP não esteja disponível
    Serial.println("DHCP indisponível!");
    delay(1000);
    while (!Serial) {
    ; // aguarda a conexão serial para configurar manualmente os endereços IP, Gateway e Máscara
    }
    Serial.println("Informe os enderecos IP: (IP,Gateway,Mascara)");
    while (configIp == 0) {
      if (Serial.available() > 0) { // em conjunto com o while, aguarda inserção de informação na Serial
        ip[0] = Serial.parseInt(); // procura o próximo inteiro válido no buffer de recebimento serial e grava em cada elemento dos arrays
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
      if (ip[0] != 0) { // sai do loop while
        configIp = 1;
      }
    }
    Ethernet.begin(mac, ip, gateway, subnet); // inicia a conexão Ethernet sem DHCP
  }
  // imprime endereço IP
  Serial.print("IP: ");
  Serial.println(Ethernet.localIP());
  Serial.print("Gateway: ");
  for(int i = 0; i < 4; i++){
    Serial.print(gateway[i]);
    if (i < 3){
      Serial.print(".");
    }
  }
  Serial.println();
  Serial.print("Mascara: ");
  for(int i = 0; i < 4; i++){
    Serial.print(subnet[i]);
    if (i < 3){
      Serial.print(".");
    }
  }
  Serial.println();
  delay(2000);
}

void loop() {

}
