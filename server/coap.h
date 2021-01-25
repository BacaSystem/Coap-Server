/*
Biblioteka Coap na potrzeby przedmiotu OBIR
*/
#pragma once
#include <stdint.h>

#define PACKET_SIZE 60

class CoapHeader{
  public:
    uint8_t ver;
    uint8_t type;
    uint8_t tokenLen;
    uint8_t coapClass;
    uint8_t coapCode;
    uint16_t mid;

    CoapHeader(){}
    CoapHeader(uint8_t ver, uint8_t type, uint8_t tokenLen, uint8_t coapClass, uint8_t coapCode, uint16_t mid)
    {
        this->ver = ver;
        this->type = type;
        this->tokenLen = tokenLen;
        this->coapClass = coapClass;
        this->coapCode = coapCode;
        this->mid = mid;
    }
};

class CoapMessage{

    CoapHeader header;
    uint8_t packet[PACKET_SIZE];
    unsigned int actualSize = 0;


    public:
    //CoapMessage() { }
    //CoapMessage(CoapHeader& h) { SetHeader(h); }
    CoapMessage(CoapHeader& h, uint8_t* token)
    { //SetHeader(h); SetToken(token);
        header = h;

        packet[0] = (1 << 6) | (header.type << 4) | (header.tokenLen);
        packet[1] = (header.coapClass << 5) | (header.coapCode);
        packet[2] = (0xFF00 & header.mid) >> 8;
        packet[3] = (0x00FF) & header.mid;

        actualSize = 4;

        for (int i = 0; i < header.tokenLen; i++)
            packet[actualSize + i] = token[i];
        actualSize += header.tokenLen;
    }
    //CoapMessage(CoapHeader& h, uint8_t* token, uint8_t* payload, uint8_t payloadLen);

    void SetContentFormat(uint8_t cf)
    {
        packet[actualSize++] = 0b11000001; //(0xF0 & 12) << 4 | (0x0F & 1); - To dziadostwo nie chce dziaslac
        packet[actualSize++] = (0xFF & cf);
    }

    void SetPayload(uint8_t* payload, uint8_t payloadLen)
    {
        packet[actualSize++] = 0xFF; //11111111

        for (int i = 0; i < payloadLen; i++)
        {
            packet[actualSize + i] = payload[i];
        }
        actualSize += payloadLen;
        packet[actualSize] = '\0';
    }

    void SetPayload(String message)
    {
        uint8_t payloadContent[PACKET_SIZE - actualSize + 1];
        message.toCharArray(payloadContent, message.length() + 1);
        SetPayload(payloadContent, message.length());
    }

    void Send(ObirEthernetUDP& udp)
    {
        udp.beginPacket(udp.remoteIP(), udp.remotePort());
        udp.write(packet, actualSize);
        udp.endPacket();
        actualSize = 0;
    }

    int GetPacketLen()
    {
        return actualSize;
    }
};
