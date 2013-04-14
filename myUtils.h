#ifndef _UTILITY_H
#define _UTILITY_H
#include <string>

using namespace std;


#define functionDebug 1
#define infoDebug 1

void throwException(string msg);
bool isIdEqual(string id1, string id2);
bool isInInterval(string ID, string fromID, string toID);
string getLocalHashID(string nodeIP);
void functionEntryLog(const char msg[]);
void generalInfoLog(const char msg[]);

#endif
