#include "coap.h"

CoapHeader::CoapHeader(uint8_t ver, uint8_t type, uint8_t tokenLen, uint8_t coapClass, uint8_t coapCode, uint16_t mid)
{
    this->ver = ver;
    this->type = (Coap_Type) type;
    this->tokenLen = tokenLen;
    this->coapClass = (Coap_Class)coapClass;
    this->coapCode = (Coap_Code)coapCode;
    this->mid = mid;
}
