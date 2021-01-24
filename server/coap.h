/*
Biblioteka Coap na potrzeby przedmiotu OBIR
*/
#pragma once
#include <stdint.h>

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
    uint8_t packet[60];
    unsigned int packetLen = 0;

    public:

    //CoapMessage() { }
    CoapMessage(CoapHeader& h) { SetHeader(h); }
    CoapMessage(CoapHeader& h, uint8_t* token) { SetHeader(h); SetToken(token); }
    CoapMessage(CoapHeader& h, uint8_t* token, uint8_t* payload, uint8_t payloadLen) { SetHeader(h); SetToken(token); SetPayload(payload, payloadLen);}

    void SetHeader(CoapHeader& h)
    {
        header = h;

        packet[0] = (1 << 6) | (h.type << 4) | (h.tokenLen);
        packet[1] = (h.coapClass << 5) | (h.coapCode);
        packet[2] = (0xFF00 & h.mid) >> 8;
        packet[3] = (0x00FF) & h.mid;

        if(!packetLen)
            packetLen = 4;
    }

    void SetToken(uint8_t* token)
    {
        for (int i = 0; i < header.tokenLen; i++)
            packet[4+i] = token[i];
        packetLen += header.tokenLen;
    }

    void SetContentFormat(uint8_t cf)
    {
        packet[packetLen++] = 0b11000001;//(0xF0 & 12) | (0x0F & 1);
        packet[packetLen++] = (0xFF & cf);
    }

    void SetPayload(uint8_t* payload, uint8_t payloadLen)
    {
        packet[packetLen++] = 0xFF; //1111111

        for (int i = 0; i < payloadLen; i++)
        {
            packet[packetLen + i] = payload[i];
        }
        packetLen += payloadLen;
    }

    void SetPayload(String message)
    {
        uint8_t txt[30];
        message.toCharArray(txt, message.length() + 1);
        SetPayload(txt, message.length());
    }

    void Send(ObirEthernetUDP& udp)
    {
        udp.beginPacket(udp.remoteIP(), udp.remotePort());
        udp.write(packet, packetLen);
        udp.endPacket();

        packetLen = 0;
        
    }


    // uint8_t* token;
    // CoapOption options [6];

};
