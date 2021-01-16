#include <ObirDhcp.h>
#include <ObirEthernet.h>
#include <ObirEthernetUdp.h>


#define UDP_SERVER_PORT 1234
#define PACKET_BUFFER_LEN 50

byte MAC[]={0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x01}; //MAC adres karty sieciowej, to powinno byc unikatowe - proforma dla ebsim'a 

uint8_t packetBuffer [PACKET_BUFFER_LEN];

ObirEthernetUDP Udp;

int localport = UDP_SERVER_PORT;

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
  }
}
