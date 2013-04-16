#ifndef _UTILITY_H
#define _UTILITY_H
#include <string>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include "Node.h"

using namespace std;


#define functionDebug 1
#define infoDebug 1

void throwException(string msg);
bool isIdEqual(string id1, string id2);
bool isInInterval(string ID, string fromID, string toID);
string getLocalHashID(string nodeIP);
void functionEntryLog(const char msg[]);
void generalInfoLog(const char msg[]);
Node* buildSuccessorNode(string IP, string ID);
string	addPowerOfTwo(int powerOfTwo,string ID); 

#endif
