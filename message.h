#ifndef _MESSAGE_H
#define _MESSAGE_H

#include <stdlib.h>

#define CLIENT_REQ 0
#define CLIENT_RESP 1
#define SERVER_REQ 2
#define SERVER_RES 3



typedef struct{

    uint32_t type; // 0 for client req message and 2/3 for server req/resp message
    char senderID[20];
    uint32_t numParameters; // Number of parameters for the command
    int* paramLenArray; // Length of each parameter
    char* parameters; // First parameter is the command name(or function name), second
                      // parameter is IP address and then any other function parameters.
                      // It might so happen that my message is forwarded to third node. So
                      // now that node knows my IP and can directly send the response message
                      // to me.
}command;

typedef struct{
    uint32_t type; // Always 1
    char senderID[20]; // ID of the server sending the response
    uint32_t result; // 0 for failure and 1 for success
}ClientResponse;

typedef struct{
	uint32_t type;
	int lengthFileName;
	int lengthCommandName;
	int lengthFileData;
	char* fileName;
	char* command;
	char* data;
}ClientRequest;


#endif
