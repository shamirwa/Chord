#ifndef _DEFS_H_
#define _DEFS_H_

#include <string>
#include <string.h>

using namespace std;

#define ERR_NODE_NOT_EXISTS "Cannot create network; node is not present!\n"

#define NUMBER_OF_SUCCESSORS 5

#define FINGER_TABLE_SIZE 160

#define ID_SIZE 20

#define ERR_NODE_IP_NOT_SET "Node URL is not set yet!"

#define ERR_NODE_ID_NOT_SET "Node ID is not set yet!"

#define ERR_IP_INVALID "IP address is invalid!"

#define FILE_NOT_FOUND "Error: File not found"

#define WRONG_NUM_SUCCESSORS "NUMBER_OF_SUCCESSORS intialized with wrong value!"

#define ERR_NODE_NULL "Error: Node is null"

#define FILE_NOT_FOUND "Error: File not found"

#define FILE_ALREADY_STORED "Error: File is already stored in the node"

#define NUM_SUCCESSOR 1 // Number of successors to store in the list

#define ID_WITH_DIFF_LENGTH "ID's with different length cannot be compared"

#define CLIENT_PORT 2345

#define SERVER_PORT 1234

#define FIND_SUCCESSOR 0
#define GET_PREDECESSOR 1
#define NOTIFY_SUCCESSOR 2

#define MAX_MSG_SIZE 5000

#define STABILIZE_TIMEOUT 12000000      // 2 seconds

#endif
