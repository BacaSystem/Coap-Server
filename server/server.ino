#include <ObirDhcp.h>        //bilioteka ktora umozliwia pobranie IP z DHCP - platforma dla ebsim'a
#include <ObirEthernet.h>    //biblioteka niezbedna dla klasy 'ObirEthernetUDP'
#include <ObirEthernetUdp.h> //biblioteka z klasa 'ObirEthernetUDP'
#include <ObirFeatures.h>

#include "coap.h" //biblioteka coap zawierajaca rozne typy zmiennych
#include "resources.h"
Resources resources;

#define UDP_SERVER_PORT 1234 //port z ktoego korzystamy w tym projekcie
#define PACKET_SIZE 60 //dlugosc pakietu z danymi dla/z UDP

#define URIPATH_MAX_SIZE 255

byte MAC[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF}; //MAC adres karty sieciowej, to powinno byc unikatowe - proforma dla ebsim'a

uint8_t packetBuffer[PACKET_SIZE]; //tablica o dlugosci pakietu
uint16_t serverMid = 0x000A;

ObirEthernetUDP Udp; //obiekt klasy 'ObirEthernetUDP'


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

    Udp.begin(UDP_SERVER_PORT); //Uruchomienie nasluchiwania na datagaramy UDP
}

void loop()
{
    int packetSize = Udp.parsePacket();
    if (packetSize > 0)
    {                                                      //instrkcja warunkowa sprawdzajaca jaka jest dligosc pakietu, jesli jest <=0 to nic nie otrzymalismy
        int len = Udp.read(packetBuffer, PACKET_SIZE); //odczytujemy pakiet(maksymalnie do liczby bajtów = PACKET_BUFER_LEN)

        serverMid += 1;

        resources.Received(len);

        Serial.println(F("----------------MESSAGE--------------"));
        uint8_t ver = (0xC0 & packetBuffer[0]) >> 6;
        uint8_t type = (0x30 & packetBuffer[0]) >> 4; //1=NON, 0 =CON
        uint8_t tkl = (0x0F & packetBuffer[0]) >> 0;
        uint8_t codeClass = ((packetBuffer[1] >> 5) & 0x07);
        uint8_t codeDetail = ((packetBuffer[1] >> 0) & 0x1F);
        uint16_t mid = (packetBuffer[2] << 8) | (packetBuffer[3]); //Message ID - ma 2 bajty
        uint8_t token[tkl]; //latwiej niz tworzyc 8 opcji roznych dlugosci int
        CoapHeader header(ver, type, tkl, codeClass, codeDetail, mid); //obiekt klasy CoapHeader

        PrintHeader(header); //wywolanie funkcji wypisujacej parametry wiadomosci

        if (tkl > 0)
        {
            Serial.println(F("Token: "));
            for (int i = 0; i < tkl; ++i)
            {
            token[i] = packetBuffer[i + 4];
            /*przepisujemy bajty*/
            Serial.print(token[i], HEX);
            }
            Serial.println();
        }

        /////////////////////////

        int marker = 4 + tkl; // znacznik poczatku opcji

        int payloadMarker = -1; //Znacznik polozenia payloadu (w bajtach)

        /*Ustawiony na -1, zeby wykryc blad*/

        int32_t delta; //Moze miec 4 bit + 2 bajty = 20 bit, ale uint24 nie istnieje
        uint8_t optionLen;
        uint8_t optionNum = 0;
        uint16_t contentFormat = 0xFFFF;
        uint16_t acceptFormat = 0xFFFF;

        uint8_t _uriPath[URIPATH_MAX_SIZE]; //Tablica na znaki w postaci liczb
        String uriPath = "";                //wl. URI; zaczyna od "NULL" zeby mozna bylo sprawdzic, czy URI w ogole byl obecny

        Serial.println(F("Options: "));

        while (packetBuffer[marker] != '\0') //Dopoki nie skonczy ramka
        {
            delta = (packetBuffer[marker] & 0xF0) >> 4;  //Maska na pierwsze 4 bity
            optionLen = (packetBuffer[marker++] & 0x0F); //Maska na kolejne 4 bity
                                                       //marker++;                               //przesuniecie markera na nastepny bajt

            /*kiedy delta albo optionLength < 12, to jest brak rozszerzen;
                  Bajt markera oznacza wtedy wartosc opcji */

            if (delta == 13) //Jest 1 bajt ext.
                delta += packetBuffer[marker++];

            else if (delta == 14)
            {
                delta = 269 + 256 * packetBuffer[marker++]; //pierwszy bajt ma wieksza wage
                delta += packetBuffer[marker++]; //dodajemy wartosc kolejnego bajtu
            }

            else if (delta == 15)
                payloadMarker = marker;

            //Czytanie payloadu potem (po petli while), dla ulatwienia

            //Dlugosc opcji zdobywamy w analogiczny sposob do delty.
            if (optionLen == 13)
                optionLen += packetBuffer[marker++];

            else if (optionLen == 14)
            {
              optionLen = 269 + 256 * packetBuffer[marker++];
              optionLen += packetBuffer[marker++];
            }

            optionNum += delta; //Numer opcji to nr poprzedniej+delta

            /*Marker powinien w tym momencie wskazywac
                    na zawartosc opcji, tzn optionValue.
                    Pole to jest wielkosci optionLength. */

            //Opcje do obsluzenia podczas odbierania:
            switch (optionNum)
            {
            case 11:
                Serial.println(F("Option 11: 'URI-Path'"));
                uriPath += "/";
                for (int i = 0; i < optionLen; ++i)
                {
                  _uriPath[i] = packetBuffer[marker++];
                  uriPath += (char)_uriPath[i];
                }
                Serial.println(uriPath);
                break;
            case 12:
                Serial.println("Option: Content-Format");
                if (optionLen == 0)
                    contentFormat = 0;
                if (optionLen == 1)
                    contentFormat = packetBuffer[marker++];
                if (optionLen == 2)
                {
                    contentFormat = packetBuffer[marker++];
                    contentFormat << 8;
                    contentFormat = contentFormat | packetBuffer[marker++];
                }
                if (contentFormat == 0)
                    Serial.println(F("plain text"));
                else if (contentFormat == 40)
                    Serial.println(F("application/link-format"));
                else{
                    Serial.println(F("Given format not supported"));
                    contentFormat = 0xFFFF;
                }
                //reszta formatow raczej nas nie obchodzi
                break;
            case 17:
                Serial.println("Option: Accept ");
                if (optionLen == 0)
                    acceptFormat = 0;
                if (optionLen == 1)
                    acceptFormat = packetBuffer[marker++];
                if (optionLen == 2)
                {
                    acceptFormat = packetBuffer[marker++];
                    acceptFormat << 8;
                    acceptFormat = acceptFormat | packetBuffer[marker++];
                }
                Serial.println(acceptFormat);
                if (acceptFormat == 0)
                    Serial.println(F("plain text"));
                else if (acceptFormat == 40)
                    Serial.println(F("application/link-format"));
                else{
                    Serial.println(F("Given format not supported"));
                }
                break;
            }
        }

        Serial.println("\n-------PAYLOAD---------");
        if (payloadMarker > 0)
        {
            uint8_t payloadLen = len - payloadMarker;
            uint8_t payload[payloadLen];

            Serial.print(F("Payload: "));
            for (int i = 0; i < payloadLen; i++)
            {
                payload[i] = packetBuffer[payloadMarker + i];
                Serial.print((char)payload[i]);
            }
            Serial.println();
        }
        else
            Serial.println("No payload found");

        //REAGOWANIE

        if(type == 1) // -----> NON
        {
            if(codeClass == 0) // ----->request
            {
                if(codeDetail == 1) // ------>GET
                {
                    if (uriPath == "/.well-known/core")
                    {
                        CoapHeader h(1, 1, header.tokenLen, 2, 5, serverMid);
                        CoapMessage m(h, token);
                        m.SetContentFormat(40);
                        String var = "</ReceivedB>;</SendB>;</TotalB>";
                        m.SetPayload(var);
                        resources.Send(m.GetPacketLen());
                        m.Send(Udp);
                    }
                    int result = resources.GetResource(uriPath);
                    if (result != -1)
                    {
                        CoapHeader h(1, 1, header.tokenLen, 2, 5, serverMid);
                        CoapMessage m(h, token);

                        if(acceptFormat != 0xFFFF)
                            m.SetContentFormat(0);
                        m.SetPayload(String(result));
                        resources.Send(m.GetPacketLen());
                        m.Send(Udp);
                    }
                }
            }
        }
        if(type == 0) // ----->CON
        {
            if(codeClass == 0) // ----->request
            {
                if(codeDetail == 1) // ------>GET
                {
                    int result = resources.GetLongResource(uriPath);
                    if (result != -1)
                    {
                        CoapHeader h(1, 2, 0, 0, 0, header.mid);
                        //CoapHeader h(1, 1, header.tokenLen, 2, 5, header.mid+1);
                        CoapMessage m(h, NULL);
                        resources.Send(m.GetPacketLen());
                        m.Send(Udp);

                        delay(10000);
                        CoapHeader hres(1, 1, header.tokenLen, 2, 5, serverMid);
                        CoapMessage mres(hres, token);
                        mres.SetPayload(String(result));
                        resources.Send(mres.GetPacketLen());
                        mres.Send(Udp);
                    }
                }
                else if(codeDetail == 0)  // -----> PING
                {
                    CoapHeader h(1, 2, 0, 0, 0, header.mid);
                    CoapMessage m(h, NULL);
                    resources.Send(m.GetPacketLen());
                    m.Send(Udp);
                }
            }
        }
    }
}
