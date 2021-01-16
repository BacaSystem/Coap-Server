#include <ObirDhcp.h>
#include <ObirEthernet.h>
#include <ObirEthernetUdp.h>

#include "coap.h"

#define UDP_SERVER_PORT 1234
#define PACKET_BUFFER_LEN 50

byte MAC[]={0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x01}; //MAC adres karty sieciowej, to powinno byc unikatowe - proforma dla ebsim'a 

uint8_t packetBuffer [PACKET_BUFFER_LEN];

ObirEthernetUDP Udp;

int localport = UDP_SERVER_PORT;

void PrintHeader(CoapHeader& header)
{
   Serial.println(F("----HEADER----"));
   Serial.print(F("Ver: "));Serial.println(header.ver, DEC);
   Serial.print(F("Type: "));Serial.println(header.type, DEC);
   Serial.print(F("Token Length: "));Serial.println(header.tokenLen, DEC);
   Serial.print(F("Code: "));Serial.print(header.coapClass, DEC);Serial.print(F("."));Serial.println(header.coapCode, DEC);
   Serial.print(F("MessageID: "));Serial.print(header.mid, DEC);
   Serial.println(F("--------------"));
}

void setup() {
  Serial.begin(115200);
  Serial.print(F("OBIR PROJEKT INIT..."));Serial.print(F(__FILE__));
  Serial.print(F(", "));Serial.print(F(__DATE__));Serial.print(F(", "));Serial.print(F(__TIME__));Serial.println(F("]")); 

  ObirEthernet.begin(MAC);

  Serial.print(F("My IP address: "));
    for (byte thisByte = 0; thisByte < 4; thisByte++) {
        Serial.print(ObirEthernet.localIP()[thisByte], DEC);Serial.print(F("."));
    }
    Serial.println();

    //Uruchomienie nasluchiwania na datagaramy UDP
    Udp.begin(localport);
}

void loop() {
  int packetSize = Udp.parsePacket();
  
  if(packetSize>0){
    int len = Udp.read(packetBuffer, PACKET_BUFFER_LEN);

    Serial.println(F("----------------MESSAGE--------------"));
    Serial.println((char*)packetBuffer);

    uint8_t ver = (0xC0 & packetBuffer[0])>>6;
    Serial.print(F("Coap ver: ")); Serial.println(ver, DEC);

    uint8_t type = (0x30 & packetBuffer[0])>>4; // 1=NON, 0 =CON

    if(type == 0){
      Serial.println(F("Type: CON"));
    }
    if(type == 1){
      Serial.println(F("Type: NON"));
    }

    

    uint8_t _token_len = (0xF & packetBuffer[0]);
    uint8_t _class = ((packetBuffer[1]>>5)&0x07);
    uint8_t _code = ((packetBuffer[1]>>0)&0x1F);
    uint16_t _mid = (packetBuffer[2]<<8)|(packetBuffer[3]); //Message ID - ma 2 bajty
    
    uint8_t _token[_token_len]; //latwiej niz tworzyc 8 opcji roznych dlugosci int
   
    Serial.print(F("Token length: ")); Serial.println(_token_len, DEC);
          //Zczytanie tokena:
          if(_token_len > 0) 
          {
            Serial.println(F("Token: "));
            for(int i = 0; i < _token_len; ++i)
            {
              _token[i] = packetBuffer[i + 4];
              /*przepisujemy bajty*/
              Serial.print(_token[i], HEX);Serial.print(F(" "));
            }
            Serial.println();
          }
          
          Serial.print(F("Code: ")); Serial.print(_class, DEC); Serial.print(F(".0"));Serial.println(_code, DEC);
          Serial.print(F("Message ID: ")); Serial.println(_mid, DEC);
    
   CoapHeader header(ver, type, _token_len, _class, _code, _mid);

   PrintHeader(header);

   
  }
}
