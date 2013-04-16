

#include "Chord.h"
#include "Defs.h"
#include <utility>
#include <stdio.h>
#include "myUtils.h"
#include "message.h"

Chord::Chord(){
	
	this->localNode = NULL;	
}

Chord::Chord(string localID,string localIP,int numSuccessor,int clientSocket,int serverSocket) {

	this->localNode = NULL;
	this->localNode = new Node;
	this->localNode->setNodeID(localID);
	this->localNode->setNodeIP(localIP);
	this->successors.setLocalID(localID);
	this->successors.setMaxNumSuccssor(numSuccessor);
	this->clientSocket = clientSocket;
	this->serverSocket = serverSocket;
	
}

Chord::~Chord() {
	delete this->localNode;
}

void Chord::createHelp()
{
	
	//The map and finger table are already instantiated so no work here

}



//searches the local table for the highest predecessor of id
string Chord::closestPrecedingNode(string id)
{
	
	string localNodeID = this->localNode->getNodeID();

	//Need a reverse iterator
  std::map<int,pair<string,string> >::reverse_iterator finger_table_iterator;

	for(finger_table_iterator=this->fingerTable.rbegin(); finger_table_iterator != this->fingerTable.rend(); ++finger_table_iterator)
	{
				//check fingerTable(i) if between localID,id
				//
				// pair has id,ip
				pair<string,string> curr_id_ip = finger_table_iterator->second;
			  string curr_finger_id = curr_id_ip.first;
				string curr_finger_ip = curr_id_ip.second;

				fprintf(stderr,"%s\n",curr_id_ip.first.c_str());

			if(strcmp(localNodeID.c_str(),id.c_str()) < 0 && strcmp(curr_finger_id.c_str(),id.c_str()) > 0)
			{
				return curr_finger_ip; 	
			}
			else if(strcmp(localNodeID.c_str(),id.c_str()) > 0 && strcmp(curr_finger_id.c_str(),id.c_str()) < 0)
			{
				return curr_finger_ip;	
			}				
	}
	
	return this->localNode->getNodeIP();
	
}


string Chord::findSuccessor(string IP)
{
		
		//TO DO:
		//Get id from IP
		//
		string id; // = ;

		//TO DO:
		//Check for cross over with zero
		//
		
		string localNodeID = this->localNode->getNodeID();

		string successorID = this->successors.getFirstSuccessor()->getNodeID();
	
	
		//Lies between the current node and successor node 
		//
		if(strcmp(localNodeID.c_str(),id.c_str()) < 0 && strcmp(successorID.c_str(),id.c_str()) > 0)
		{
			return this->successors.getFirstSuccessor()->getNodeIP(); 	
		}
		else if(strcmp(localNodeID.c_str(),id.c_str()) > 0 && strcmp(successorID.c_str(),id.c_str()) < 0)
		{
			return this->localNode->getNodeIP();	
		}
		
		else
		{
			string closestPrecedingNodeIP = this->closestPrecedingNode(id);
			/* TO DO:
			 * Call the successor of the closestPrecedingNodeIP
			 *
			 *return closestPrecedingNode->findSuccessor(id);
			 */
		}
		
}


void Chord::create(){
		
	//is Node already connected	
	if(this->localNode == NULL)
	{
		throwException(ERR_NODE_NOT_EXISTS);
	}

	//has NodeIP been set?
	if(this->localNode->getNodeIP().length() == 0)
	{	
		throwException(ERR_NODE_IP_NOT_SET);
	}

	//if necessary generate NodeId out of IP
	if(this->localNode->getNodeID().length() == 0)
	{
		this->localNode->setNodeID(getLocalHashID(this->localNode->getNodeIP()));
	}
	
	this->createHelp();
}

void Chord::joinHelp(string IP)
{
        /*	
		//create local repository for successors
		if (NUMBER_OF_SUCCESSORS >=1)
		{
			
		}
		else
		{
			throwException(WRONG_NUM_SUCCESSORS);
		}
        */

		//check if local node exists
		this->predecessor = NULL;

		//Get successor
        string succID;

        // Send the bootstrap node a request to find my successor
        sendRequestToServer(FIND_SUCCESSOR, IP);

        // wait for the response message
        char* maxMessage = new char[MAX_MSG_SIZE];
        struct sockaddr_in senderProcAddrUDP;

        // To store the address of the process from whom a message is received
        memset((char*)&senderProcAddrUDP, 0, sizeof(senderProcAddrUDP));
        socklen_t senderLenUDP = sizeof(senderProcAddrUDP);

        int recvRet = 0;

        recvRet = recvfrom(serverSocket, maxMessage, MAX_MSG_SIZE,
                  0, (struct sockaddr*) &senderProcAddrUDP, &senderLenUDP);

        string succIP = inet_ntoa(senderProcAddrUDP.sin_addr);

        if(recvRet > 0){
            handleResponseFromServer(maxMessage, recvRet, succID);
    		buildFingerTable(succIP, succID);

            // Build the successor Node and store inside the successor list
            Node* mySucc = buildSuccessorNode(succIP, succID);
            successors.storeSuccessor(mySucc);
        }
        else{
            fprintf(stderr, "Error while receiving response for findSuccessor request message in Join\n");
        }

}

void Chord::join(string IP){

	//Check if IP is valid
	if(IP.length() == 0)
	{
		throwException(ERR_IP_INVALID);
	}

	if(this->localNode == NULL)
	{
		throwException(ERR_NODE_NOT_EXISTS);
	}
	
	this->joinHelp(IP);

		
		



}

void Chord::leave(){


}

void Chord::ping(){


}

void Chord::buildFingerTable(string IP, string ID){


}

void Chord::insert(string id,string fileContent){


}

void Chord::sendRequestToServer(int method, string rcvrIP){

    string commandName;
    char* msgBuffer = NULL;
    long messageLen = 0;

    switch(method){

        case 0:
            {
                // create the message to send
                commandName = "findSuccessor";
                command* findSuccMsg = new command;
                findSuccMsg->type = SERVER_REQ;
                memcpy(findSuccMsg->senderID, localNode->getNodeID().c_str(), 20);
                findSuccMsg->numParameters = 2;

                int* paramLen = new int[2];
                paramLen[0] = commandName.length();
                paramLen[1] = localNode->getNodeIP().length();

                char* params = new char[paramLen[0] + paramLen[1]];
                memcpy(params, commandName.c_str(), paramLen[0]);
                memcpy(params + paramLen[0], localNode->getNodeIP().c_str(), paramLen[1]);

                // Allocate a large buffer to serialize the parameters
                messageLen = sizeof(command) + paramLen[0] + paramLen[1] + sizeof(int)*2;
                msgBuffer = new char[messageLen];
                memcpy(msgBuffer, findSuccMsg, sizeof(command));
                memcpy(msgBuffer + sizeof(command), paramLen, sizeof(int) * 2);
                memcpy(msgBuffer + sizeof(command) + sizeof(int) *2, params, (paramLen[0] + paramLen[1]));

            }

    default:
        generalInfoLog("Error while sending. Unkonwn command request found\n");
    }

    // check if socket is open for the servers
    if(getServerSocket() == -1){
        // Need to open a socket
        if((serverSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1){
            printf("Error while opening socket to send the message\n");
            exit(1);
        }
    }

    struct sockaddr_in receiverAddr;

    memset((char*)&receiverAddr, 0, sizeof(receiverAddr));
    receiverAddr.sin_family = AF_INET;
    receiverAddr.sin_port = htons(SERVER_PORT);

    if(inet_aton(rcvrIP.c_str(), &receiverAddr.sin_addr) == 0){
        printf("INET_ATON Failed\n");
    }

    if(sendto(getServerSocket(), msgBuffer, messageLen, 0,
                (struct sockaddr*) &receiverAddr, sizeof(receiverAddr)) == -1){

        fprintf(stderr, "%s: Failed to send the message type %d to leader: %s",
                localNode->getNodeIP().c_str(), method, rcvrIP.c_str());
        fflush(stderr);
    }
    else{
        fprintf(stderr, "%s: successfully sent the message type %d to %s\n",
                localNode->getNodeIP().c_str(), method, rcvrIP.c_str());
        fflush(stderr);
    }

}


int Chord::getServerSocket(){

    return serverSocket;
}

int Chord::getClientSocket(){

    return clientSocket;
}

void Chord::setServerSocket(int servSock){

    serverSocket = servSock;
}

void Chord::setClientSocket(int cliSock){
    
    clientSocket = cliSock;
}

void Chord::handleResponseFromServer(char* msgRcvd, int rcvdNumBytes, string& response)
{
    command* rcvdMsg = new command;

    memcpy(rcvdMsg, msgRcvd, sizeof(command));

    int paramCount = rcvdMsg->numParameters;
    int* paramLenArr = new int[paramCount];
    memcpy(paramLenArr, rcvdMsg + sizeof(command), sizeof(int) * paramCount);

    int totalParamsSize = 0;

    for(int i = 0; i<paramCount; i++){
        totalParamsSize += paramLenArr[i];
    }

    char* parameters = new char[totalParamsSize];
    memcpy(parameters, rcvdMsg + sizeof(command) + sizeof(int)*paramCount, totalParamsSize);

    char* commandName = new char[paramLenArr[0] + 1];
    memcpy(commandName, parameters, paramLenArr[0]);

    // Check if response is for FIND_SUCCESSOR
    if(strcmp(commandName, "findSuccessor") == 0){
        response = rcvdMsg->senderID;
    }

}
