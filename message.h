#ifndef _MESSAGE_H
#define _MESSAGE_H

#include <stdlib.h>

typedef struct{

    uint32_t type; // 0 for client message and 1 for server RPC message
    char senderID[20]; // Id of the sender
    uint32_t numParameters;
    int* paramLenArray;
    char* parameters; // First parameter is the command name(or function name), second
                      // parameter is IP address and then any other function parameters
}command;


#endif
