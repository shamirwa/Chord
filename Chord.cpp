// TO DO:
// Make the thread socket different from the normal communication socket
// It might so happen a server is waiting for the reponse but it gets the message
// from some other server for stabilize phase

#include "Chord.h"
#include "Defs.h"
#include <utility>
#include <stdio.h>
#include "myUtils.h"
#include "message.h"

Chord::Chord(){

    this->localNode = NULL;	
}

Chord::Chord(string localID,string localIP,int numSuccessor,int clientSocket,
             int serverSocket, int stabilizeSocket){

    this->localNode = NULL;
    this->localNode = new Node;
    this->localNode->setNodeID(localID);
    this->localNode->setNodeIP(localIP);
    this->successors.setLocalID(localID);
    this->successors.setMaxNumSuccssor(numSuccessor);
    this->clientSocket = clientSocket;
    this->serverSocket = serverSocket;
    this->stabilizeSocket = stabilizeSocket;
    this->predecessor = NULL;

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
    functionEntryLog("CHORD: closestPrecedingNode");

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

        printf("%s\n",curr_id_ip.first.c_str());
        fflush(NULL);

        if(isInInterval(curr_finger_id, localNodeID, id)){
            return curr_finger_ip;
        }
    }

    return localNode->getNodeIP();

}


void Chord::findSuccessor(string senderIPFromMsg, char* message, long messageLen)
{
    functionEntryLog("findSuccessor");

    if(!(localNode || successors.getFirstSuccessor())){
        return;
    }

    string localNodeID = this->localNode->getNodeID();
    string localNodeIP = this->localNode->getNodeIP();
    string successorID = this->successors.getFirstSuccessor()->getNodeID();
    string successorIP = this->successors.getFirstSuccessor()->getNodeIP();
    string senderID = ((command*)message)->senderID;

    if(senderID.length() > 20){
        senderID.erase(20,1);
    }


    //Lies between the current node and successor node 
    //
    if(isInInterval(senderID, localNodeID, successorID))
    {
        // Send my successor id to the node with senderID
        // Extract the receiver IP from the message received
        // It might be the case that sender of this message is
        // not the original server
        if(debug){
            printf("Sending response with self successor id\n");
            fflush(NULL);
        }
        sendResponseToServer(FIND_SUCCESSOR, successorID, successorIP, senderIPFromMsg);

    }
    else
    {
        string closestPrecedingNodeIP = this->closestPrecedingNode(senderID);

        if(closestPrecedingNodeIP.compare(localNode->getNodeIP()) == 0){

            // I am the only node in the network
            // Send my ID  SEND RESPONSE TO SERVER
            if(debug){
                printf("Sending response with self id. No preceeding node\n");
                fflush(NULL);
            }

            sendResponseToServer(FIND_SUCCESSOR, localNodeID, localNodeIP, senderIPFromMsg);
        }
        else{
            if(debug){
                printf("Forwarding the message to the preceeding node\n");
                fflush(NULL);
            }
            // Forward this message to the preceding Node
            sendRequestToServer(FIND_SUCCESSOR, closestPrecedingNodeIP, "NULL", message, messageLen);
        }
    }
}


void Chord::create(){

    functionEntryLog("Chord: create");
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

    // This is the first node. So sets itself to be the first successor
    Node* firstSucc = new Node;
    firstSucc->setNodeID(localNode->getNodeID());
    firstSucc->setNodeIP(localNode->getNodeIP());
    successors.storeFirstSuccessor(firstSucc);

    this->createHelp();
}

void Chord::joinHelp(string IP)
{
    functionEntryLog("CHORD: joinHelp");

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
    sendRequestToServer(FIND_SUCCESSOR, IP, localNode->getNodeID());

    // wait for the response message
    char* maxMessage = new char[MAX_MSG_SIZE];
    struct sockaddr_in senderProcAddrUDP;

    // To store the address of the process from whom a message is received
    memset((char*)&senderProcAddrUDP, 0, sizeof(senderProcAddrUDP));
    socklen_t senderLenUDP = sizeof(senderProcAddrUDP);

    int recvRet = 0;

    if(debug){
        printf("Waiting for response message at socket: %d\n", serverSocket);
        fflush(NULL);
    }

    recvRet = recvfrom(serverSocket, maxMessage, MAX_MSG_SIZE,
            0, (struct sockaddr*) &senderProcAddrUDP, &senderLenUDP);

    if(debug){
        printf("Response message received\n");
        fflush(NULL);
    }
    
    string succIP;// = inet_ntoa(senderProcAddrUDP.sin_addr);

    if(recvRet > 0){
        handleResponseFromServer(maxMessage, succID, succIP);
        buildFingerTable(succIP, succID);

        if(debug){
            printf("Successfully built the finger table\n");
            fflush(NULL);
        }

        // Build the successor Node and store inside the successor list
        Node* mySucc = buildSuccessorNode(succIP, succID);
        successors.storeSuccessor(mySucc);
    }
    else{
        printf("Error while receiving response for findSuccessor request message in Join\n");
        fflush(NULL);
    }

}

void Chord::join(string IP)
{

    functionEntryLog("CHORD: join");

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

map<string,Entry> Chord::getAllEntries()
{
	return localNode->getAllEntries();
}

char* Chord::makeBufferToSend(long& msgLength)
{
	
				map<string,Entry> entriesToBeSent = this->getAllEntries();

				int sumKeyLengths = 0;
				int sumValueLengths = 0;

				LeaveMsg* leaveMsg = new LeaveMsg; 

				leaveMsg->type = LEAVE_ENTRIES_MSG;

				leaveMsg->nEntries = entriesToBeSent.size();

				leaveMsg->lengthKeys =(int*) malloc(sizeof(int)*(leaveMsg->nEntries));	
				leaveMsg->lengthValues=(int*) malloc(sizeof(int)*(leaveMsg->nEntries));

				leaveMsg->keys = (char**)malloc(sizeof(char)*(leaveMsg->nEntries));	

				leaveMsg->values = (char**)malloc(sizeof(char)*(leaveMsg->nEntries));	

				int iter = 0;
				for(map<string,Entry>::iterator myIter = entriesToBeSent.begin(); myIter!= entriesToBeSent.end();myIter++)
				{
								Entry curEntry = myIter->second;

								int lengthKey = curEntry.getFileKey().length();

								sumKeyLengths += lengthKey;

								int lengthValue = curEntry.getFileValue().length();

								sumValueLengths += lengthValue;

								leaveMsg->lengthKeys[iter] = lengthKey;

								leaveMsg->lengthValues[iter] = lengthValue;

								leaveMsg->keys[iter] = (char*)malloc(sizeof(char)*lengthKey);

								memcpy(leaveMsg->keys[iter],curEntry.getFileKey().c_str(),lengthKey);

								leaveMsg->values[iter] = (char*)malloc(sizeof(char)*lengthValue);

								memcpy(leaveMsg->values[iter],curEntry.getFileValue().c_str(),lengthValue);

								iter++;
				}

				//Make a big buffer and copy everything
				long sizeMaxBuff = sizeof(LeaveMsg) + 2*sizeof(int)*leaveMsg->nEntries + sizeof(char)*sumValueLengths + sizeof(char)*sumKeyLengths;

				msgLength = sizeMaxBuff;	

				char* maxBuff = (char*)(malloc(sizeof(char)*sizeMaxBuff));

				memcpy(maxBuff,leaveMsg,sizeof(leaveMsg));

				memcpy(maxBuff,leaveMsg->lengthKeys,sizeof(int)*leaveMsg->nEntries);

				memcpy(maxBuff,leaveMsg->lengthValues,sizeof(int)*leaveMsg->nEntries);

				memcpy(maxBuff,leaveMsg->keys,sizeof(char)*sumKeyLengths);

				memcpy(maxBuff,leaveMsg->values,sizeof(char)*sumValueLengths);
		
			delete leaveMsg->lengthValues;
			delete leaveMsg->lengthKeys;
			delete leaveMsg->keys;
			delete leaveMsg->values;

			return maxBuff;
}

void Chord::leave(){

				functionEntryLog("CHORD: leave");

				//This node is about to leave
				Node* predecessor = this->getPredecessor();
				Node* successor   = this->getFirstSuccessor();

				//transfer all its keys to the successor
				long msgLength;
				char* buffToSend = makeBufferToSend(msgLength);
				
				sendRequestToServer(LEAVE_MSG_FOR_SUCCESSOR,successor->getNodeIP(), successor->getNodeID(),buffToSend,msgLength);

				




}

void Chord::ping(){


}


void Chord::buildFingerTable(string succIP, string succID){

    functionEntryLog("CHORD: buildFingerTable");	

    //for logging
    int lowestWrittenIndex = -1;	
    int highestWrittenIndex = -1;

    for(int i=0;i<FINGER_TABLE_SIZE;i++)
    {

        string startOfInterval =	addPowerOfTwo(i,localNode->getNodeID());  

        sendRequestToServer(FIND_SUCCESSOR,succIP, startOfInterval);
        // Here I need to ask my successor node to find me successor of 
        // startOfInterval ID

        // wait for the response message
        char* maxMessage = new char[MAX_MSG_SIZE];
        struct sockaddr_in senderProcAddrUDP;

        // To store the address of the process from whom a message is received
        memset((char*)&senderProcAddrUDP, 0, sizeof(senderProcAddrUDP));
        socklen_t senderLenUDP = sizeof(senderProcAddrUDP);

        int recvRet = 0;

        recvRet = recvfrom(serverSocket, maxMessage, MAX_MSG_SIZE,
                0, (struct sockaddr*) &senderProcAddrUDP, &senderLenUDP);

        string senderIP;// = inet_ntoa(senderProcAddrUDP.sin_addr);

        string senderID;

        if(recvRet > 0){
            handleResponseFromServer(maxMessage, senderID, senderIP);	
        }
        else{
            printf("Error while receiving response for findSuccessor request message in buildFingerTable \n");
            fflush(NULL);
        }

        if(!isInInterval(startOfInterval,localNode->getNodeID(),senderID))
        {
            break;
        }

        if(lowestWrittenIndex == -1){
            lowestWrittenIndex = i;
        }
        highestWrittenIndex = i;

        if(fingerTable.find(i) == fingerTable.end())
        {
            fingerTable[i] = make_pair(senderID,senderIP);
        }
        else if(isInInterval(senderID,localNode->getNodeID(),
                    fingerTable[i].first))	
        {
            fingerTable[i].first = senderIP;
            fingerTable[i].second = senderID;
        }


    }

    cout<<"Added reference to finger table entries "<<
        lowestWrittenIndex<<" "<<highestWrittenIndex<<endl;


}

void Chord::insert(string id,string fileContent){


}

void Chord::sendRequestToServer(int method, string rcvrIP, string idToSend, 
                                char* message, long msgLen){

    functionEntryLog("CHORD: sendRequestToServer");

    string commandName;
    char* msgBuffer = NULL;
    long messageLen = 0;
    bool useServerSock = false, useStabSock = false;
    int sendSock = -1;

    switch(method){

        case 0:
            {
                generalInfoLog("In case 0");
                
                // create the message to send
                commandName = "findSuccessor";

                if(!message){
                   msgBuffer = getMessageToSend(SERVER_REQ, commandName, idToSend, 
                                                localNode->getNodeIP(), messageLen);

                   if(debug){
                       printf("Message Len: %ld\n", messageLen);
                       printMessageDetails(msgBuffer);
                       fflush(NULL);
                   }
                }
                else{
                    msgBuffer = message;
                    messageLen = msgLen;
                }

                useServerSock = true;
                useStabSock = false;

                break;
            }

        case 1:
            {
                generalInfoLog("In case 1");

                // create the message to send
                commandName = "getPredecessor";
                msgBuffer = getMessageToSend(SERVER_REQ, commandName, idToSend, 
                                             localNode->getNodeIP(), messageLen);

                useStabSock = true;
                useServerSock = false;
                break;
            }

        case 2:
            {
                generalInfoLog("In case 2");

                // create the message to send
                commandName = "notifySucc";
                msgBuffer = getMessageToSend(SERVER_REQ, commandName, idToSend, 
                                             localNode->getNodeIP(), messageLen);
                
                useServerSock = false;
                useStabSock = true;

                break;
            }

        default:
            generalInfoLog("Error while sending. Unkonwn command request found\n");
    }

    // check if socket is open for the servers
    if(useServerSock){
        if(getServerSocket() == -1){
            // Need to open a socket
            if((serverSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1){
                printf("Error while opening socket to send the message\n");
                exit(1);
            }
        }

        sendSock = getServerSocket();
    }
    else if(useStabSock){
        if(stabilizeSocket == -1){
            // Need to reopen the socket
            if((stabilizeSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1){
                printf("Error while opening socket to send the message\n");
                exit(1);
            }
        }

        sendSock = stabilizeSocket;
    }

    struct sockaddr_in receiverAddr;

    memset((char*)&receiverAddr, 0, sizeof(receiverAddr));
    receiverAddr.sin_family = AF_INET;

    if(useServerSock){
        receiverAddr.sin_port = htons(SERVER_PORT);
    }
    else if(useStabSock){
        receiverAddr.sin_port = htons(STABILIZE_PORT);
    }
    else{
        if(debug){
            printf("Some error happened while sending\n");
            fflush(NULL);
        }
    }

    if(inet_aton(rcvrIP.c_str(), &receiverAddr.sin_addr) == 0){
        printf("INET_ATON Failed\n");
        fflush(NULL);
    }

    if(sendto(sendSock, msgBuffer, messageLen, 0,
                (struct sockaddr*) &receiverAddr, sizeof(receiverAddr)) == -1){

        printf("%s: Failed to send the message type %d to leader: %s",
                localNode->getNodeIP().c_str(), SERVER_REQ, rcvrIP.c_str());
        fflush(NULL);
    }
    else{
        printf("%s: successfully sent the message type %d to %s\n",
                localNode->getNodeIP().c_str(), SERVER_REQ, rcvrIP.c_str());
        fflush(NULL);
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

void Chord::handleResponseFromServer(char* msgRcvd, string& responseID, string& responseIP)
{
    functionEntryLog("CHORD: handleResponseFromServer");

    command* rcvdMsg = new command;

    memcpy(rcvdMsg, msgRcvd, sizeof(command));

    int paramCount = rcvdMsg->numParameters;
    int* paramLenArr = new int[paramCount];
    memcpy(paramLenArr, msgRcvd + sizeof(command), sizeof(int) * paramCount);

    int totalParamsSize = 0;

    for(int i = 0; i<paramCount; i++){
        totalParamsSize += paramLenArr[i];
    }

    char* parameters = new char[totalParamsSize];
    memcpy(parameters, msgRcvd + sizeof(command) + sizeof(int)*paramCount, totalParamsSize);

    char* commandName = new char[paramLenArr[0] + 1];
    memcpy(commandName, parameters, paramLenArr[0]);
    commandName[paramLenArr[0]] = '\0';

    char* respIP = new char[paramLenArr[1] + 1];
    memcpy(respIP, parameters + paramLenArr[0], paramLenArr[1]);
    respIP[paramLenArr[1]] = '\0';

    // Check if response is for FIND_SUCCESSOR
    if(strcmp(commandName, "findSuccessor") == 0){
        generalInfoLog("Received response for findSuccessor");

        if(debug){
            printf("Response id rcvd: %s\n", rcvdMsg->senderID);
            printf("Response ip rcvd: %s\n", respIP);
            fflush(NULL);
        }

        responseID = rcvdMsg->senderID;
        responseID[20] = '\0';
        responseIP = respIP;
    }
    else if(strcmp(commandName,"getPredecessor") == 0){
        generalInfoLog("Received response for getPredecessor");
        
        responseID = rcvdMsg->senderID;
        responseID[20] = '\0';
        responseIP = respIP;
    }
    else{
        printMessageDetails(msgRcvd);
        printf("Command name: %s\n", commandName);
        printf("Resp IP: %s\n", respIP);
        generalInfoLog("Invalid response received");
    }

}

void Chord::handleRequestFromServer(char* msgRcvd, long messageLen)
{
    functionEntryLog("CHORD: handleRequestFromServer");

    command* rcvdMsg = new command;

    memcpy(rcvdMsg, msgRcvd, sizeof(command));

    int paramCount = rcvdMsg->numParameters;
    int* paramLenArr = new int[paramCount];
    memcpy(paramLenArr, msgRcvd + sizeof(command), sizeof(int) * paramCount);

    int totalParamsSize = 0;

    for(int i = 0; i<paramCount; i++){
        totalParamsSize += paramLenArr[i];
    }

    char* parameters = new char[totalParamsSize];
    memcpy(parameters, msgRcvd + sizeof(command) + sizeof(int)*paramCount, totalParamsSize);

    char* commandName = new char[paramLenArr[0] + 1];
    memcpy(commandName, parameters, paramLenArr[0]);
    commandName[paramLenArr[0]] = '\0';

    char* sendIP = new char[paramLenArr[1] + 1];
    memcpy(sendIP, parameters + paramLenArr[0], paramLenArr[1]);
    sendIP[paramLenArr[1]] = '\0';

    if(debug){
        printMessageDetails(msgRcvd);
        fflush(NULL);
    }

    string senderIP = sendIP;

    if(debug){
        printf("Received message from %s\n", sendIP);
        fflush(NULL);
    }

    // Check if response is for FIND_SUCCESSOR
    if(strcmp(commandName, "findSuccessor") == 0){

        generalInfoLog("Received request for findSuccessor");
        findSuccessor(senderIP, msgRcvd, messageLen);	
    }
    else if(strcmp(commandName, "getPredecessor") == 0){

        generalInfoLog("Received request for getPredecessor");

        if(getPredecessor()){
            sendResponseToServer(GET_PREDECESSOR, getPredecessor()->getNodeID(), 
                    getPredecessor()->getNodeIP(), senderIP); 
        }
        else{
            sendResponseToServer(GET_PREDECESSOR, "NULL", "NULL", senderIP);
        }
    }
    else if(strcmp(commandName, "notifySucc") == 0){
        generalInfoLog("Received request for notify");

        // This is received since stabilize is running on one of the node
        string senderID = rcvdMsg->senderID;

        if(senderID.length() > 20){
            senderID.erase(20, 1);
        }

        printf("Sender ID received msg %s\n", rcvdMsg->senderID);
        printf("Sender ID received msg %s\n", senderID.c_str());
        printf("Length is %ld\n", senderID.length());
        fflush(NULL);

        if(!getPredecessor() || 
                isInInterval(senderID, getPredecessor()->getNodeID(), localNode->getNodeID()))
        {
           printf("updating my predecessor as %s *  %s\n", senderID.c_str(), senderIP.c_str());
           setPredecessor(senderIP, senderID); 
        }

    }
    else{
        generalInfoLog("Unknown request received\n");
    }
    

}

char* Chord::getMessageToSend(int msgType, string cmnd, string idToSend, string ipToSend,
                        long& msgLength)
{
    functionEntryLog("CHORD: getMessageToSend");

    if(debug){
        printf("Sending Message details\n");
        printf("Type: %d\n", msgType);
        printf("SenderID: %s\n", idToSend.c_str());
        printf("Length of ID: %ld\n", idToSend.length());
        printf("Command: %s\n", cmnd.c_str());
        printf("IP to send: %s\n", ipToSend.c_str());
        fflush(NULL);
    }

    // create the message to send
    command* Msg = new command;
    Msg->type = msgType;
    memcpy(Msg->senderID, idToSend.c_str(), idToSend.length());
    Msg->numParameters = 2;

    printf("SenderID in msg: %s\n", Msg->senderID);

    int* paramLen = new int[2];
    paramLen[0] = cmnd.length();
    paramLen[1] = ipToSend.length();

    if(debug){
        printf("Param 0 Len: %d\n", paramLen[0]);
        printf("Param 0 Len: %d\n", paramLen[1]);
        fflush(NULL);
    }

    char* params = new char[paramLen[0] + paramLen[1]];
    memcpy(params, cmnd.c_str(), paramLen[0]);
    memcpy(params + paramLen[0], ipToSend.c_str(), paramLen[1]);

    /*
    if(debug){
        int len = paramLen[0] + paramLen[1];
        
        printf("Parameters: %s\n", params);
        for(int i = 0; i<len; ++i){
            printf("%c\n", params[i]);
        }
    }*/

    // Allocate a large buffer to serialize the parameters
    char* msgBuffer = NULL;
    long messageLen = 0;
    messageLen = sizeof(command) + paramLen[0] + paramLen[1] + sizeof(int)*2;
    msgBuffer = new char[messageLen];
    memcpy(msgBuffer, Msg, sizeof(command));
    memcpy(msgBuffer + sizeof(command), paramLen, sizeof(int) * 2);
    memcpy(msgBuffer + sizeof(command) + sizeof(int) *2, params, (paramLen[0] + paramLen[1]));


    // Delete the memory allocated for paramLen and params
    delete[] paramLen;
    delete[] params;
    delete Msg;

    // Return the message buffer
    msgLength = messageLen;
    return msgBuffer;
}

void Chord::sendResponseToServer(int method, string responseID, string responseIP, string receiverIP){

    functionEntryLog("CHORD: sendResponseToServer");

    string commandName;
    char* msgBuffer = NULL;
    long messageLen = 0;
    bool useServSock = false, useStabSock = false;
    int sendSock = -1;

    switch(method){

        case 0:
            {
                generalInfoLog("In case 0");

                // create the message to send
                commandName = "findSuccessor";
                msgBuffer = getMessageToSend(SERVER_RES, commandName, responseID, 
                                            responseIP, messageLen);
                useServSock = true;
                useStabSock = false;
                break;
            }
        case 1:
            {
                generalInfoLog("In case 1");

                // create the message to send
                commandName = "getPredecessor";
                msgBuffer = getMessageToSend(SERVER_RES, commandName, responseID,
                                            responseIP, messageLen);

                useServSock = false;
                useStabSock = true;
                break;
            }

        default:
            generalInfoLog("Error while sending. Unkonwn command request found\n");
    }

    // check if socket is open for the servers
    if(useServSock){
        if(getServerSocket() == -1){

            printf("Opening socket again\n");
            // Need to open a socket
            if((serverSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1){
                printf("Error while opening socket to send the message\n");
                exit(1);
            }
        }

        sendSock = getServerSocket();
    }
    else if(useStabSock){
        if(stabilizeSocket == -1){

            printf("Opening socket again\n");
            // Need to open a socket
            if((stabilizeSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1){
                printf("Error while opening socket to send the message\n");
                exit(1);
            }
        }

        sendSock = stabilizeSocket;
    }

    struct sockaddr_in receiverAddr;

    memset((char*)&receiverAddr, 0, sizeof(receiverAddr));
    receiverAddr.sin_family = AF_INET;

    if(useServSock){
        receiverAddr.sin_port = htons(SERVER_PORT);
    }
    else if(useStabSock){
        receiverAddr.sin_port = htons(STABILIZE_PORT);
    }

    if(inet_aton(receiverIP.c_str(), &receiverAddr.sin_addr) == 0){
        printf("INET_ATON Failed\n");
    }

    if(debug){
        printf("Sending message of length: %ld\n", messageLen);
        fflush(NULL);
    }

    if(sendto(sendSock, msgBuffer, messageLen, 0,
                (struct sockaddr*) &receiverAddr, sizeof(receiverAddr)) == -1){

        printf("%s: Failed to send the message type %d to server: %s",
                localNode->getNodeIP().c_str(), SERVER_RES, receiverIP.c_str());
        fflush(NULL);
    }
    else{
        printf("%s: successfully sent the message type %d to %s\n",
                localNode->getNodeIP().c_str(), SERVER_RES, receiverIP.c_str());
        fflush(NULL);
    }

}

void Chord::stabilize(){
    functionEntryLog("CHORD: stabilize");

    // Send message to my successor to get his predecessor node
    Node* succNode = successors.getFirstSuccessor();
    string predIP;// = inet_ntoa(senderProcAddrUDP.sin_addr);
    string predID;

    if(!succNode){
        cout << "Successor Node is NULL\n";
        return;
    }

    string succIP = succNode->getNodeIP();
    string succID = succNode->getNodeID();

    if(succIP.compare(localNode->getNodeIP()) == 0){
        printf("I am the successor of myself\n");
        if(getPredecessor()){
            predID = getPredecessor()->getNodeID();
            predIP = getPredecessor()->getNodeIP();
        }
        else{
            predID = "NULL";
            predIP = "NULL";
        }
    }
    else{

        sendRequestToServer(GET_PREDECESSOR, succIP, localNode->getNodeID());

        // wait for response message to come
        char* maxMessage = new char[MAX_MSG_SIZE];
        struct sockaddr_in senderProcAddrUDP;

        // To store the address of the process from whom a message is received
        memset((char*)&senderProcAddrUDP, 0, sizeof(senderProcAddrUDP));
        socklen_t senderLenUDP = sizeof(senderProcAddrUDP);

        int recvRet = 0;

        printf("Waiting for response\n");
        recvRet = recvfrom(stabilizeSocket, maxMessage, MAX_MSG_SIZE,
                0, (struct sockaddr*) &senderProcAddrUDP, &senderLenUDP);

        printf("Response Message Received\n");
        fflush(NULL);

        if(recvRet > 0){
            handleResponseFromServer(maxMessage, predID, predIP);

            /*
               if(!predIP.compare("NULL") == 0 && !predID.compare("NULL") == 0){

               if(isInInterval(predID, localNode->getNodeID(), succID)){

               printf("stabilize: storing the first successor\n");
               printf("Self Id: %s\n", localNode->getNodeID().c_str());
               printf("Received Id: %s\n", predID.c_str());
               printf("Successor Id: %s\n", succID.c_str());
               fflush(NULL);

            // Store this successor as the first successor
            Node* firstSucc = new Node;
            firstSucc->setNodeID(predID);
            firstSucc->setNodeIP(predIP);

            successors.storeFirstSuccessor(firstSucc);
            }
            }
            else{
            printf("No information regarding predecessor received\n");
            }

            // Notify the successor and don't wait for any response
            succIP = successors.getFirstSuccessor()->getNodeIP();
            succID = successors.getFirstSuccessor()->getNodeID();

            sendRequestToServer(NOTIFY_SUCCESSOR, succIP, localNode->getNodeID());
            */
        }
        else{
            printf("Error while receiving in Stabilize\n");
            fflush(NULL);
            return;
        }
    }

    if(!predIP.compare("NULL") == 0 && !predID.compare("NULL") == 0){

        if(isInInterval(predID, localNode->getNodeID(), succID)){

            printf("stabilize: storing the first successor\n");
            printf("Self Id: %s\n", localNode->getNodeID().c_str());
            printf("Received Id: %s\n", predID.c_str());
            printf("Successor Id: %s\n", succID.c_str());
            fflush(NULL);

            // Store this successor as the first successor
            Node* firstSucc = new Node;
            firstSucc->setNodeID(predID);
            firstSucc->setNodeIP(predIP);

            successors.storeFirstSuccessor(firstSucc);
        }
    }
    else{
        printf("No information regarding predecessor received\n");
    }

    // Notify the successor and don't wait for any response
    succIP = successors.getFirstSuccessor()->getNodeIP();
    succID = successors.getFirstSuccessor()->getNodeID();

    sendRequestToServer(NOTIFY_SUCCESSOR, succIP, localNode->getNodeID());
}

Node* Chord::getPredecessor(){

    return predecessor;
}

void Chord::setPredecessor(string predIP, string predID)
{
    if(getPredecessor()){
        Node* currPred = getPredecessor();
        delete currPred;
    }

    Node* newPred = new Node;
    newPred->setNodeIP(predIP);
    newPred->setNodeID(predID);
    predecessor = newPred;
}

Node* Chord::getFirstSuccessor(){
    
    return successors.getFirstSuccessor();
}

string Chord::getLocalID(){

    return localNode->getNodeID();
}

string Chord::getLocalIP(){

    return localNode->getNodeIP();
}

void Chord::handleRequestFromClient(char* msgRcvd, long messageLen)
{
    functionEntryLog("handleRequestFromClient");

    ClientRequest* clientMsg = new ClientRequest;
    memcpy(clientMsg, msgRcvd, sizeof(ClientRequest));

    char* file = new char[clientMsg->lengthFileName];
    char* cmnd = new char[clientMsg->lengthCommandName];
    char* data = new char[clientMsg->lengthFileData];
    char* ip = new char[clientMsg->lengthClientIP];

    // Copy fileName
    long len = sizeof(ClientRequest);
    memcpy(file, msgRcvd + len, clientMsg->lengthFileName);

    // Copy CommandName
    len += clientMsg->lengthFileName;
    memcpy(cmnd, msgRcvd + len, clientMsg->lengthCommandName);

    // Copy fileDate
    len += clientMsg->lengthCommandName;
    memcpy(data, msgRcvd + len, clientMsg->lengthFileData);

    // copy client ip
    len += clientMsg->lengthFileData;
    memcpy(ip, msgRcvd + len, clientMsg->lengthClientIP);

    // Check for command name
    if(strcmp(cmnd, PUT) == 0){

        generalInfoLog("Received a put command from client\n");


    }
    else if(strcmp(cmnd, GET) == 0){

        generalInfoLog("Received a get command from client\n");
    
    }
    else if(strcmp(cmnd, EXISTS) == 0){

        generalInfoLog("Received a exists command from client\n");

    }
    else if(strcmp(cmnd, LS) == 0){

        generalInfoLog("Received a ls command from client\n");

    }
    else if(strcmp(cmnd, DELETE) == 0){

        generalInfoLog("Received a delete command from client\n");
    }
}
