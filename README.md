# 📡 Monitoramento SNMP e Controle de Ar-Condicionado para Data Center (Arduino)

Projeto de **monitoramento de temperatura e umidade via SNMP** e **controle automático/manual de ar-condicionado via infravermelho**, utilizando **Arduino Uno**, **Ethernet Shield W5100** e **sensor DHT**.  
Inclui **servidor web embarcado**, **agente SNMP v1** e lógica de automação baseada em limites térmicos.

***

## 📋 Visão Geral

Este projeto foi desenvolvido para uso em **Data Centers, CPDs ou salas técnicas**, permitindo:

*   Monitoramento remoto de **temperatura** e **umidade** via **SNMP**
*   Integração com ferramentas como **Zabbix**, **Nagios** e **LibreNMS**
*   Controle do **ar-condicionado via IR**, com:
    *   Modo **automático**
    *   Modo **manual via navegador**
*   Interface web simples embarcada no próprio Arduino
*   Controle progressivo de temperatura para evitar choques térmicos

***

## 🧑‍💻 Autor

*   **Autor:** Denilly Carvalho do Carmo
*   **Versão:** 3.5
*   **Data:** Outubro de 2019

***

## 🧰 Hardware Utilizado

| Componente       | Descrição                              |
| ---------------- | -------------------------------------- |
| Arduino          | Arduino Uno R3                         |
| Microcontrolador | ATmega328P                             |
| Ethernet         | Ethernet Shield W5100                  |
| Sensor           | DHT11 (ou DHT22, opcional)             |
| LED IR           | LED infravermelho no pino **D3 (PWM)** |
| Receptor IR      | KY-022 (para captura dos códigos)      |
| Resistor         | 100 Ω em série com o LED IR            |

### 🔌 Pinagem Principal

*   **D3** → LED Infravermelho (IR)
*   **A1** → Sensor DHT (dados)
*   **SPI (Ethernet)**:
    *   Uno: 10 (SS), 11 (MOSI), 12 (MISO), 13 (SCK)

***

## 📚 Bibliotecas Utilizadas

*   `Ethernet`
*   `SPI`
*   `Agentuino` (SNMP v1)
*   `DHT`
*   `EasyWebServer`
*   `Streaming`

> ⚠️ Certifique-se de instalar todas as bibliotecas antes de compilar o sketch.

***

## 🌡️ Monitoramento via SNMP

O Arduino atua como um **agente SNMP v1**, expondo OIDs padrão (RFC1213) e OIDs privados para temperatura e umidade.

### 📌 OIDs Implementados

| OID                       | Descrição        |
| ------------------------- | ---------------- |
| `1.3.6.1.2.1.1.1.0`       | sysDescr         |
| `1.3.6.1.2.1.1.3.0`       | sysUpTime        |
| `1.3.6.1.2.1.1.4.0`       | sysContact       |
| `1.3.6.1.2.1.1.5.0`       | sysName          |
| `1.3.6.1.2.1.1.6.0`       | sysLocation      |
| `1.3.6.1.2.1.1.7.0`       | sysServices      |
| `1.3.6.1.4.1.36582.3.1.0` | Temperatura (°C) |
| `1.3.6.1.4.1.36582.3.2.0` | Umidade (%)      |

### 🧪 Exemplo de Teste SNMP (Windows)

```bash
snmpget -v:1 -t:10 -c:public 192.168.0.101 1.3.6.1.4.1.36582.3.1.0
```

***

## 🌐 Servidor Web Embarcado

O projeto disponibiliza uma **interface web HTTP (porta 80)** para controle do ar-condicionado.

### 🔗 Rotas Disponíveis

| URL                   | Função                         |
| --------------------- | ------------------------------ |
| `/`                   | Página principal (status)      |
| `/setauto`            | Ativa modo automático          |
| `/naoauto`            | Desativa modo automático       |
| `/ligar`              | Liga o ar-condicionado         |
| `/desligar`           | Desliga o ar-condicionado      |
| `/temp17` a `/temp24` | Ajusta temperatura manualmente |

### 🖥️ Funcionalidades da Interface

*   Exibe:
    *   Temperatura atual
    *   Umidade atual
    *   Último comando IR enviado
*   Permite:
    *   Alternar entre modo automático e manual
    *   Enviar comandos IR diretamente

***

## 🤖 Modo Automático de Controle

No **modo automático**, o sistema:

*   Liga o ar-condicionado se:
    *   Temperatura > **25 °C**
*   Ajusta a temperatura gradualmente:
    *   Faixa ideal: **19 °C a 22 °C**
    *   Ajustes progressivos (17 °C a 24 °C)

⏱️ Isso evita oscilações bruscas e sobrecarga do equipamento.

***

## 📡 Controle Infravermelho (IR)

*   Os códigos IR são armazenados em **PROGMEM**
*   Cada comando contém **199 pulsos**
*   Frequência aproximada: **38 kHz**
*   Compatível com controles clonados de ar-condicionado

> Para capturar novos códigos IR, utilize o sketch de captura mencionado nos comentários do código (`capturar_ir_v2.ino`).

***

## 🌐 Configuração de Rede

*   **Padrão:** DHCP
*   **Fallback:** Configuração manual via **Serial Monitor**
    *   IP
    *   Gateway
    *   Máscara de sub-rede

***

## 🚀 Como Usar

1.  Monte o hardware conforme a pinagem
2.  Instale as bibliotecas necessárias
3.  Compile e grave o sketch no Arduino
4.  Conecte o cabo de rede
5.  Descubra o IP via Serial Monitor ou DHCP
6.  Acesse:
    *   Navegador → `http://IP_DO_ARDUINO`
    *   SNMP Manager → OIDs definidos

***

## 🔒 Observações Importantes

*   SNMP v1 **não possui criptografia**
*   Recomendado uso em **rede interna**
*   Ethernet W5100 possui limitações de sockets TCP
*   O projeto foi otimizado para **baixo uso de RAM**

***

## 📖 Referências

*   Arduino – Referência Oficial (PT-BR):  
    <https://www.arduino.cc/reference/pt/>
*   FilipeFlop – Tutoriais e Componentes Arduino:  
    <https://www.filipeflop.com>
*   Agentuino – Biblioteca SNMP para Arduino:  
    <https://github.com/1sw/Agentuino>
*   Agentuino + Zabbix – Implementação SNMP no Arduino:  
    <http://arduinoprojexp.blogspot.com/2014/11/agentuino-zabbix-parte-1-implementacao.html>
*   Monitoramento de Temperatura e Umidade de Data Center utilizando Arduino e Zabbix (Slides):  
    <https://pt.slideshare.net/lailtonmontenegro/monitoramento-de-temperatura-e-umidade-de-data-center-utilizando-o-arduino-e-o-sistema-zabbix>
*   Controle de Ar-Condicionado com Arduino e LED Infravermelho:  
    <http://labdegaragem.com/profiles/blogs/controlando-ar-condicionado-utilizando-arduino-e-led>
*   Ar-Condicionado Controlado por Arduino via Infravermelho (Instructables):  
    <https://www.instructables.com/id/Ar-Condicionado-controlado-por-Arduino-via-infrave/>
*   Clonando Qualquer Controle Remoto com Arduino:  
    <https://arduinolivre.wordpress.com/2012/07/31/clonando-qualquer-controle-remoto/>
*   Guia Completo do Controle Remoto IR e Receptor IR para Arduino:  
    <https://blog.eletrogate.com/guia-completo-do-controle-remoto-ir-receptor-ir-para-arduino/>
*   Arduino – Uso de PROGMEM para otimização de memória:  
    <https://www.arduino.cc/reference/pt/language/variables/utilities/progmem/>
*   Artigos sobre Controle de Ar-Condicionado e Automação Térmica:  
    <https://www.analysir.com/blog/tag/air-conditioner/>
*   EasyWebServer – Servidor Web Embarcado para Arduino:  
    <https://github.com/llelundberg/EasyWebServer>

***

## ✅ Status do Projeto

✔️ Funcional  
🛠️ Mantido como referência técnica e educacional  
📦 Ideal para integração com sistemas de monitoramento

***
