/*
Biblioteka Coap na potrzeby przedmiotu OBIR
*/
#pragma once
#include <stdint.h>

typedef enum {
    CON = 0,
    NON = 1,
    ACK = 2,
    RST = 3

} Coap_Type;

typedef enum {
    REQUEST = 0,
    SUCCESS = 2,
    CLIENT_ERROR = 4,
    SERVER_ERROR = 5
} Coap_Class;

typedef enum {
    EMPTY = 0,
    GET = 1,
    POST = 2,
    PUT = 3,
    DELETE = 4
} Coap_Code;

class CoapHeader{
  public:
    uint8_t ver;
    Coap_Type type;
    uint8_t tokenLen;
    Coap_Class coapClass;
    Coap_Code coapCode;
    uint16_t mid;

    CoapHeader(uint8_t ver, uint8_t type, uint8_t tokenLen, uint8_t coapClass, uint8_t coapCode, uint16_t mid);

};

class CoapOption{
    public:
        uint8_t number;
        uint8_t length;
        uint8_t* value;
};

class CoapMessage{
    CoapHeader header;
    uint8_t* token;
    CoapOption options [6];

    void AddOption();

};
