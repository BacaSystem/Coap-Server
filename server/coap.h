/*
Biblioteka Coap na potrzeby przedmiotu OBIR
*/
#pragma once

typedef enum {
    CON = 0,
    NON = 1,
    ACK = 2,
    RST = 3

} Coap_Type;

typedef enum {
    GET = 1,
    POST = 2,
    PUT = 3,
    RESET = 4
} Coap_Method;



