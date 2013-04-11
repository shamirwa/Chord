#ifndef _SERVICE_H
#define _SERVICE_H

#include <stdio.h>
#include <string>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <stdlib.h>

using namespace std;

#define XINU 0
#define VM 1
#define debug 1
#define functionDebug 1
#define infoDebug 1


// Function to log the entry of a function
void functionEntryLog(char msg[]);

// Function to log the general information about the system
void generalInfoLog(char msg[]);



/* Function to get the hash code for the node IP
   Input: nodeIP - IP address of the node for which hash key needs
                    to be computed
   Returns: string - hashKey generated for the passed IP address

   Description: This hashKey is the unique identifier of a node
                in the chord network
*/
string getLocalHashID(string nodeIP);

#endif
