#include <ObirDhcp.h>        //bilioteka ktora umozliwia pobranie IP z DHCP - platforma dla ebsim'a
#include <ObirEthernet.h>    //biblioteka niezbedna dla klasy 'ObirEthernetUDP'
#include <ObirEthernetUdp.h> //biblioteka z klasa 'ObirEthernetUDP'


#include "coap.h" //biblioteka coap zawierajaca rozne typy zmiennych

#define UDP_SERVER_PORT 1234 //port z ktoego korzystamy w tym projekcie
#define PACKET_BUFFER_LEN 60 //dlugosc pakietu z danymi dla/z UDP

#define URIPATH_MAX_SIZE 255

byte MAC[] = {0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x01}; //MAC adres karty sieciowej, to powinno byc unikatowe - proforma dla ebsim'a

uint8_t packetBuffer[PACKET_BUFFER_LEN]; //tablica o dlugosci pakietu

ObirEthernetUDP Udp; //obiekt klasy 'ObirEthernetUDP'

int localport = UDP_SERVER_PORT; //numer portu na jakim bedziemy nasluchiwac

void PrintHeader(CoapHeader &header) //funkcja wypisujaca parametry obslugiwanych wiadomosci
{
  Serial.println(F("----HEADER----"));
  Serial.print(F("Ver: "));
  Serial.println(header.ver, DEC);
  Serial.print(F("Type: "));
  Serial.println(header.type, DEC);
  Serial.print(F("Token Length: "));
  Serial.println(header.tokenLen, DEC);
  Serial.print(F("Code: "));
  Serial.print(header.coapClass, DEC);
  Serial.print(F("."));
  Serial.println(header.coapCode, DEC);
  Serial.print(F("MessageID: "));
  Serial.println(header.mid, DEC);
  Serial.println(F("--------------"));
}

void setup()
{
  //Przywitanie z uzytkownikiem - potwierdzajace poprawne uruchomienie sie systemu
  Serial.begin(115200);
  Serial.print(F("OBIR PROJEKT INIT..."));
  Serial.print(F(__FILE__));
  Serial.print(F(", "));
  Serial.print(F(__DATE__));
  Serial.print(F(", "));
  Serial.print(F(__TIME__));
  Serial.println(F("]"));

  ObirEthernet.begin(MAC); //inicjacja karty sieciowej - preforma dla ebsim'a

  Serial.print(F("My IP address: ")); //wyswietlenie informacji potwierdzajacej na jakim IP dzialamy
  for (byte thisByte = 0; thisByte < 4; thisByte++)
  {
    Serial.print(ObirEthernet.localIP()[thisByte], DEC);
    Serial.print(F("."));
  }
  Serial.println();

  Udp.begin(localport); //Uruchomienie nasluchiwania na datagaramy UDP
}

void loop()
{
  int packetSize = Udp.parsePacket();
  if (packetSize > 0)
  {                                                      //instrkcja warunkowa sprawdzajaca jaka jest dligosc pakietu,
                                                         //jesli jest <=0 to nic nie otrzymalismy
    int len = Udp.read(packetBuffer, PACKET_BUFFER_LEN); //odczytujemy pakiet(maksymalnie do liczby bajtów = PACKET_BUFER_LEN)

    Serial.println(F("----------------MESSAGE--------------"));
    //Serial.println((char*)packetBuffer);

    uint8_t ver = (0xC0 & packetBuffer[0]) >> 6;
    uint8_t type = (0x30 & packetBuffer[0]) >> 4; //1=NON, 0 =CON
    uint8_t _token_len = (0x0F & packetBuffer[0]) >> 0;
    uint8_t _class = ((packetBuffer[1] >> 5) & 0x07);
    uint8_t _code = ((packetBuffer[1] >> 0) & 0x1F);
    uint16_t _mid = (packetBuffer[2] << 8) | (packetBuffer[3]); //Message ID - ma 2 bajty

    uint8_t _token[_token_len]; //latwiej niz tworzyc 8 opcji roznych dlugosci int

    CoapHeader header(ver, type, _token_len, _class, _code, _mid); //obiekt klasy CoapHeader

    PrintHeader(header); //wywolanie funkcji wypisujacej parametry wiadomosci

    if (_token_len > 0)
    {
      Serial.println(F("Token: "));
      for (int i = 0; i < _token_len; ++i)
      {
        _token[i] = packetBuffer[i + 4];
        /*przepisujemy bajty*/
        Serial.print(_token[i], HEX);
        Serial.print(F(" "));
      }
      Serial.println();
    }

    /////////////////////////

    //bool payloadFound = false;

    int marker = 4 + _token_len; // 4bajty naglowka + dlugosc tokena

    int payloadMarker = -1;      //Znacznik polozenia payloadu (w bajtach)

    /*Ustawiony na -1, zeby wykryc blad*/

    int32_t delta; //Moze miec 4 bit + 2 bajty = 20 bit, ale uint24 nie istnieje
    uint8_t optionLen;
    uint8_t optionNum = 0;

    /*W celu oddzielenia odczytywania wiadomosci od wysylania odpowiedzi, 
          kazda obslugiwana opcja musi tez miec status, lub swoja wartoscia
          poczatkowa status ten odwzorowac*/
    //uint8_t eTag[ETAG_MAX_SIZE]; //patrz: dokumentacja
    //enum ETagStatus _eTagStatus = NO_ETAG;

    uint16_t contentFormat = 0xFFFF;
    uint16_t acceptFormat = 0xFFFF;
    uint8_t _uriPath[URIPATH_MAX_SIZE]; //Tablica na znaki w postaci liczb
    String uriPath = "";                //wl. URI; zaczyna od "NULL" zeby mozna bylo sprawdzic, czy URI w ogole byl obecny




    Serial.println(F("Options: "));

    while (packetBuffer[marker] != '\0') //Dopoki nie znajdzie sie payload albo nie skonczy ramka
    {
      delta = (packetBuffer[marker] & 0xF0) >> 4;   //Maska na pierwsze 4 bity
      optionLen = (packetBuffer[marker++] & 0x0F); //Maska na kolejne 4 bity
            //marker++;                               //przesuniecie markera na nastepny bajt

      /*kiedy delta albo optionLength < 12, to jest brak rozszerzen; 
              Bajt markera oznacza wtedy wartosc opcji */

      if (delta == 13)
      {
        //Jest 1 bajt rozszerzenia:
        delta += packetBuffer[marker++];
        //++marker;
      }
      else if (delta == 14)
      {
        //Sa 2 bajty rozszerzenia
        //TODO: to moze nie byc konieczne, zalezy czy musimy obslugiwac opcje z duzym numerem
        delta = 269 + 256 * packetBuffer[marker++]; //pierwszy bajt ma wieksza wage
        //++marker;
        delta += packetBuffer[marker++]; //dodajemy wartosc kolejnego bajtu
        //++marker;
      }

      else if (delta == 15){ //Trafiono na marker payloadu
        payloadMarker = marker;
      }

      //Czytanie payloadu potem (po petli while), dla ulatwienia

      if (delta != 15) //Jezeli nie bylo markera payloadu, mozna obsluzyc opcje bez przejmowania sie bledami
      {

        //Dlugosc opcji zdobywamy w analogiczny sposob do delty.
        if (optionLen == 13)
        {
          optionLen += packetBuffer[marker++];
         // ++marker;
        }
        else if (optionLen == 14)
        {
          optionLen = 269 + 256 * packetBuffer[marker++];
          //++marker;
          optionLen += packetBuffer[marker++];
         // ++marker;
        }

        optionNum += delta; //Numer opcji to nr poprzedniej+delta

        /*Marker powinien w tym momencie wskazywac
                na zawartosc opcji, tzn optionValue.
                Pole to jest wielkosci optionLength. */

        //Opcje do obsluzenia podczas odbierania:

        Serial.print(F("\nDelta: "));
        Serial.print(delta, DEC);
        Serial.print(F(", Option number: "));
        Serial.print(optionNum, DEC);
        Serial.print(F(", Option Length: "));
        Serial.println(optionLen, DEC);

        if (optionNum == 11)
        //URI-path
        {
          
          String tmp = "";
          Serial.println(F("Option URI-Path: "));
          for (int i = 0; i < optionLen; ++i)
          {
            _uriPath[i] = packetBuffer[marker++];
            tmp += (char) _uriPath[i];
            //++marker;
          }
          //ArrayToString(tmp, _uriPath, optionLength);

          uriPath += '/';
          uriPath += tmp;  //URI moze skladac sie z kilku segmentow
          Serial.print(F("URI-Path: ")); Serial.println(uriPath);
        }

        //Content-Format
        else if(optionNum == 12)
        {
          uint16_t contentType; //Tu bedzie przechowana zawartosc opcji

          Serial.println("Option: Content-Format");

         if (optionLen == 0)
          {
            Serial.println(F("Content-type option length is zero: assuming text/plain"));
            //Copper robil tak zamiast wysylac zero. Stad takie zalozenie (nie do konca zgodne z RFC)
            contentType = 0;
          }
          if (optionLen == 1)
          {
            contentType = packetBuffer[marker++];
            //++marker;
          }
          if (optionLen == 2)
          {
            contentType = packetBuffer[marker];
            contentType << 8; //Eksperymentalne; nie mialem na czym tego przetestowac
            ++marker;
            contentType = contentType | packetBuffer[marker];
            ++marker;
          }
          if (contentType == 0)
            Serial.println(F("plain text"));
          else if (contentType == 40)
            Serial.println(F("application/link-format"));
          else
            Serial.println(F("Given format not supported"));
          //reszta formatow raczej nas nie obchodzi

          contentFormat = contentType;
        }
        

        else if (optionNum == 17)
        //Accept (czyli jaka reprezentacje woli klient)
        {
          uint16_t contentType; //Tu bedzie przechowana zawartosc opcji

          Serial.println(F("Option: Accept"));


          if (optionLen == 0)
          {
            Serial.println(F("Content-type option length is zero: assuming text/plain"));
            //Copper robil tak zamiast wysylac zero. Stad takie zalozenie (nie do konca zgodne z RFC)
            contentType = 0;
          }

          if (optionLen == 1)
          {
            contentType = packetBuffer[marker++];
            //++marker;
          }
          if (optionLen == 2)
          {
            contentType = packetBuffer[marker];
            contentType << 8; //Eksperymentalne; nie mialem na czym tego przetestowac
            ++marker;
            contentType = contentType | packetBuffer[marker];
            ++marker;
          }

          if (contentType == 0)
            Serial.println(F("plain text\n"));
          else if (contentType == 40)
            Serial.println(F("application/link-format\n"));
          else
            Serial.println(F("Given format not supported"));
          //reszta formatow raczej nas nie obchodzi

          //Zaleznie od opcji, wartosc jest wpisana do odp. zmiennej
          acceptFormat = contentType;
        }

        else //Jezeli jest nieobslugiwana opcja, i tak trzeba przesunac marker
        {
          marker += optionLen;
          Serial.println(F("Unsupported option"));
        }
      }
    }

    if(uriPath == "/.well-known/core")
    {
         CoapHeader h(1, 1, header.tokenLen, 2, 5, header.mid);
         CoapMessage m(h, _token);
         m.SetContentFormat(40);
         String var = "</metryka>;if=metryka";
         m.SetPayload(var);
         m.Send(Udp);
    }

      Serial.println("\n-------PAYLOAD---------");

      uint8_t payloadLen = len - payloadMarker;
      uint8_t payload[payloadLen];

      if(payloadMarker > 0)
      {
        Serial.print(F("Payload: 0x"));
        for(int i = 0; i<payloadLen; i++)
        {
          payload[i] = packetBuffer[payloadMarker+i];
          Serial.print(payload[i], HEX);
        }
        Serial.print(F(" = "));
            for(int i=0; i < payloadLen; ++i)
              Serial.print((char)payload[i]);
        Serial.println();
      }
      else 
        Serial.println("No payload found");
      
    }
  
}
