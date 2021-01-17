#include <ObirDhcp.h>                            //bilioteka ktora umozliwia pobranie IP z DHCP - platforma dla ebsim'a
#include <ObirEthernet.h>                        //biblioteka niezbedna dla klasy 'ObirEthernetUDP'
#include <ObirEthernetUdp.h>                     //biblioteka z klasa 'ObirEthernetUDP'

#include "coap.h"                                //biblioteka coap zawierajaca rozne typy zmiennych

#define UDP_SERVER_PORT 1234                     //port z ktoego korzystamy w tym projekcie
#define PACKET_BUFFER_LEN 50                     //dlugosc pakietu z danymi dla/z UDP

byte MAC[]={0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x01}; //MAC adres karty sieciowej, to powinno byc unikatowe - proforma dla ebsim'a 

uint8_t packetBuffer [PACKET_BUFFER_LEN];        //tablica o dlugosci pakietu

ObirEthernetUDP Udp;                             //obiekt klasy 'ObirEthernetUDP'

int localport = UDP_SERVER_PORT;                 //numer portu na jakim bedziemy nasluchiwac

void PrintHeader(CoapHeader& header)             //funkcja wypisujaca parametry obslugiwanych wiadomosci
{
   Serial.println(F("----HEADER----"));
   Serial.print(F("Ver: "));Serial.println(header.ver, DEC);
   Serial.print(F("Type: "));Serial.println(header.type, DEC);
   Serial.print(F("Token Length: "));Serial.println(header.tokenLen, DEC);
   Serial.print(F("Code: "));Serial.print(header.coapClass, DEC);Serial.print(F("."));Serial.println(header.coapCode, DEC);
   Serial.print(F("MessageID: "));Serial.println(header.mid, DEC);
   Serial.println(F("--------------"));
}

void setup() {
  //Przywitanie z uzytkownikiem - potwierdzajace poprawne uruchomienie sie systemu
  Serial.begin(115200);
  Serial.print(F("OBIR PROJEKT INIT..."));Serial.print(F(__FILE__));
  Serial.print(F(", "));Serial.print(F(__DATE__));Serial.print(F(", "));Serial.print(F(__TIME__));Serial.println(F("]")); 

  ObirEthernet.begin(MAC);                          //inicjacja karty sieciowej - preforma dla ebsim'a 

  Serial.print(F("My IP address: "));               //wyswietlenie informacji potwierdzajacej na jakim IP dzialamy
    for (byte thisByte = 0; thisByte < 4; thisByte++) {
        Serial.print(ObirEthernet.localIP()[thisByte], DEC);Serial.print(F("."));
    }
    Serial.println();
    
    Udp.begin(localport);                            //Uruchomienie nasluchiwania na datagaramy UDP
}

void loop() {
  int packetSize = Udp.parsePacket();                
  if(packetSize>0){                                       //instrkcja warunkowa sprawdzajaca jaka jest dligosc pakietu, 
                                                          //jesli jest <=0 to nic nie otrzymalismy
    int len = Udp.read(packetBuffer, PACKET_BUFFER_LEN);  //odczytujemy pakiet(maksymalnie do liczby bajtów = PACKET_BUFER_LEN)

    Serial.println(F("----------------MESSAGE--------------"));
    //Serial.println((char*)packetBuffer);

    uint8_t ver = (0xC0 & packetBuffer[0])>>6;
    uint8_t type = (0x30 & packetBuffer[0])>>4;                    //1=NON, 0 =CON
    uint8_t _token_len = (0xF & packetBuffer[0]);
    uint8_t _class = ((packetBuffer[1]>>5)&0x07);
    uint8_t _code = ((packetBuffer[1]>>0)&0x1F);
    uint16_t _mid = (packetBuffer[2]<<8)|(packetBuffer[3]);        //Message ID - ma 2 bajty
    
    uint8_t _token[_token_len];                                    //latwiej niz tworzyc 8 opcji roznych dlugosci int
    
    CoapHeader header(ver, type, _token_len, _class, _code, _mid); //obiekt klasy CoapHeader
    
    PrintHeader(header);                                           //wywolanie funkcji wypisujacej parametry wiadomosci

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

   
  }
}
