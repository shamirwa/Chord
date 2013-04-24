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
    generalInfoLog("CHORD: getAllEntries");            

    return localNode->getAllEntries();
}

char* Chord::makeBufferToSend(string commandName, long& msgLength, string predID,
                             string predIP, bool isLeave)
{
    generalInfoLog("CHORD: makeBufferToSend");    

    map<string,Entry> entriesToBeSent = this->getAllEntries();

    int sumKeyLengths = 0;
    int sumValueLengths = 0;
    int i = 0, totalSize = 0, lenArraySize = 0, paramSize = 0, totalEntries = 0;

   // LeaveMsg* leaveMsg = new LeaveMsg; 
    command* leaveMsg = new command;
    msgLength = sizeof(command);

    leaveMsg->type = SERVER_REQ;
    memcpy(leaveMsg->senderID,predID.c_str(),predID.length());
    
    totalEntries = entriesToBeSent.size();
    leaveMsg->numParameters = totalEntries + 2;

    if(isLeave){
        lenArraySize = sizeof(int) * (leaveMsg->numParameters + totalEntries);
    }
    else{
        lenArraySize = sizeof(int) * (leaveMsg->numParameters);
    }

    int* lenArray = new int[lenArraySize];

    lenArray[i++] = commandName.length();
    totalSize = commandName.length();
    lenArray[i++] = predIP.length();
    totalSize += localNode->getNodeIP().length();
   
    map<string,Entry>::iterator myIter = entriesToBeSent.begin();

    if(isLeave){
        for(; myIter!= entriesToBeSent.end();myIter++)
        {
            // Store the length of each value
            lenArray[i++] = myIter->second.getFileValue().length();
            lenArray[i++] = myIter->second.getFileName().length();
            totalSize += ID_SIZE;
            totalSize += myIter->second.getFileName().length();
            totalSize += myIter->second.getFileValue().length();
        }
    }
    else{
        // For ls message
        for(; myIter!= entriesToBeSent.end();myIter++)
        {
            // Store the length of each value
            lenArray[i++] = myIter->second.getFileName().length();
            totalSize += myIter->second.getFileName().length();
        }
    }


    // Size of all param lengths
    msgLength += lenArraySize;

    paramSize = totalSize;
    msgLength += paramSize;

    char* params = new char[paramSize];

    memcpy(params, commandName.c_str(), commandName.length());
    memcpy(params + commandName.length(), predIP.c_str(),
            predIP.length());
    
    i = 2;
    totalSize = lenArray[0] + lenArray[1];
    
    if(isLeave){
        for(myIter = entriesToBeSent.begin();
                myIter!= entriesToBeSent.end();myIter++){

            // Store all the keys
            memcpy(params + totalSize, myIter->first.c_str(),
                    ID_SIZE);
            totalSize += ID_SIZE;

            // Store all the values
            memcpy(params + totalSize, myIter->second.getFileValue().c_str(),
                    lenArray[i]);
            totalSize += lenArray[i++];

            // Store all the names
            memcpy(params + totalSize, myIter->second.getFileName().c_str(),
                    lenArray[i]);
            totalSize += lenArray[i++];

        }
    }
    else{
        for(myIter = entriesToBeSent.begin();
                myIter!= entriesToBeSent.end();myIter++){

            // Store all the names
            memcpy(params + totalSize, myIter->second.getFileName().c_str(),
                    lenArray[i]);
            totalSize += lenArray[i++];

        }
    }
   
    // Allocate the large buffer to serialize
    char* maxBuff = new char[msgLength];
    int addLen = sizeof(command);
    memcpy(maxBuff,leaveMsg,sizeof(command));
    
    memcpy(maxBuff + addLen, lenArray, lenArraySize);
    addLen += lenArraySize;

    memcpy(maxBuff + addLen, params, paramSize);

    // Delete the memory
    delete leaveMsg;
    delete[] lenArray;
    delete[] params;

    generalInfoLog("Buffer to be sent is made");
    return maxBuff;

}

void Chord::leave(){

    functionEntryLog("CHORD: leave");

    //This node is about to leave
    Node* predecessor = this->getPredecessor();
    Node* successor   = this->getFirstSuccessor();

    if(!predecessor || !successor){
        cout << "I am the only node. BYE\n";
        return;
    }

    //transfer all its keys to the successor
    long msgLength;
    string cmnd = "leaving";
    char* buffToSend = makeBufferToSend(cmnd, msgLength,predecessor->getNodeID(), predecessor->getNodeIP(), true);

    sendRequestToServer(LEAVE_MSG_FOR_SUCCESSOR,successor->getNodeIP(), successor->getNodeID(),buffToSend,msgLength);

    //Prepare message for Predecessor
    msgLength = 0;
    string commandName = "changeSuccessor";
    command* leaveMessageForPred = new command;
    msgLength += sizeof(command);
    
    leaveMessageForPred->type = SERVER_REQ;
    memcpy(leaveMessageForPred->senderID, successor->getNodeID().c_str(),
            successor->getNodeID().length());
    leaveMessageForPred->numParameters = 2;

    int* lenArray = new int[2];
    lenArray[0] = commandName.length();
    lenArray[1] = successor->getNodeIP().length();
    msgLength += sizeof(int) * 2;

    char* params = new char[lenArray[0] + lenArray[1]];
    msgLength += lenArray[0] + lenArray[1];

    memcpy(params, commandName.c_str(), commandName.length());
    memcpy(params + commandName.length(), successor->getNodeIP().c_str(),
            lenArray[1]);

    char* buffForPred = new char[msgLength];
    int addLen = sizeof(command);

    memcpy(buffForPred, leaveMessageForPred, addLen);
    memcpy(buffForPred + addLen, lenArray, sizeof(int) * 2);
    addLen += sizeof(int) * 2;
    memcpy(buffForPred + addLen, params, lenArray[0] + lenArray[1]);

    delete leaveMessageForPred;
    delete[] lenArray;
    delete[] params;

    generalInfoLog("Buffer to be sent is made");

    //send Message to Predecessor

    sendRequestToServer(LEAVE_MSG_FOR_PREDECESSOR,predecessor->getNodeIP(),
         predecessor->getNodeID(),buffForPred, msgLength);

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
         case 3:
         case 4:
         case 5:
         case 6:
         case 7:
         case 8:
         case 9:
            {
                cout << "In case " << method << endl;
                
                if(!message){
                    cout<<"Message was NULL " <<endl; 
                    return;
                }                
                else{
                    msgBuffer = message;
                    messageLen = msgLen;
                }

                useServerSock = true;
                useStabSock = false;
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

        if(responseID.length() > 20){
            responseID.erase(20);
        }
        responseIP = respIP;
    }
    else if(strcmp(commandName,"getPredecessor") == 0){
        generalInfoLog("Received response for getPredecessor");

        responseID = rcvdMsg->senderID;
        responseID[20] = '\0';
        if(responseID.length() > 20){
            responseID.erase(20);
        }
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
    
    string senderID = rcvdMsg->senderID;

    if(senderID.length() > 20){
        senderID.erase(20, 1);
    }

    // Check if request is for FIND_SUCCESSOR
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
    else if(strcmp(commandName, "leaving") == 0){
   
        generalInfoLog("Received request for leave");

        totalParamsSize = 0;
        delete[] parameters;
        delete[] paramLenArr;
        int numEntries = paramCount - 2;

        paramCount += numEntries;

        int* paramLenArr = new int[paramCount];
        memcpy(paramLenArr, msgRcvd + sizeof(command), sizeof(int) * paramCount);

        // Find the actual size
        for(int i = 0; i < paramCount; i++){
            totalParamsSize += paramLenArr[i];
        }
        // Add for the space of keys
        totalParamsSize += (ID_SIZE * numEntries);

        parameters = new char[totalParamsSize];
        memcpy(parameters, msgRcvd + sizeof(command) + sizeof(int)*paramCount, totalParamsSize);

        // Store the entries
        int skipLen = paramLenArr[0] + paramLenArr[1];
        int valueLen = 0;
        char* key = new char[ID_SIZE];
        char* value;
        char* name;
        for(int j = 0; j < (paramCount - 2); j++){
            valueLen = paramLenArr[j+2];
            value = new char[valueLen];
            ++j;
            
            // Copy the key
            memcpy(key, parameters + skipLen, ID_SIZE);
            skipLen += ID_SIZE;

            // Copy the value
            memcpy(value, parameters + skipLen, valueLen);
            skipLen += valueLen;

            // Copy the name
            name = new char[paramLenArr[j+2]];
            memcpy(name, parameters + skipLen, paramLenArr[j+2]);
            skipLen += paramLenArr[j+2];

            // Store this entry
            localNode->storeEntry(key, name, value);
        }
       
        cout<<"My new predecessor is: "<<senderIP<<endl;

        // Store the new predecessor
        setPredecessor(senderIP, senderID);
        
    }
    else if(strcmp(commandName, "changeSuccessor") == 0){
       
        generalInfoLog("Received request for changeSuccessor");

        Node* succ = new Node;
        succ->setNodeID(senderID);
        succ->setNodeIP(senderIP);

        cout<<"My new successor is: "<<senderIP<<endl;

        successors.storeFirstSuccessor(succ);

    }
    else if(strcmp(commandName, "storeFile") == 0 ||
           (strcmp(commandName, "getFile") == 0)){

        bool forStore = false;
        if(strcmp(commandName, "storeFile") == 0){
            forStore = true;
        }
       
        if(forStore){
            generalInfoLog("Received request for storefile");
        }
        else{
            generalInfoLog("Received request for getFile");
        }

        char* fileID = new char[paramLenArr[2] + 1];
        memcpy(fileID, parameters + paramLenArr[0] + paramLenArr[1], paramLenArr[2]);
        fileID[paramLenArr[2]] = '\0';

        string key = fileID;
        if(key.length() > 20){
            key.erase(20, 1);
        }

        char* fileValue = NULL;
        char* fileName = NULL;

        if(forStore){
            fileValue = new char[paramLenArr[3] + 1];
            memcpy(fileValue, parameters + paramLenArr[0] + paramLenArr[1] + paramLenArr[2], paramLenArr[3]);
            fileValue[paramLenArr[3]] = '\0';

            fileName = new char[paramLenArr[4] + 1];
            memcpy(fileName, parameters + paramLenArr[0] + paramLenArr[1] + paramLenArr[2] + paramLenArr[3], paramLenArr[4]);
            fileName[paramLenArr[4]] = '\0';
        }
        else{
            fileName = new char[paramLenArr[3] + 1];
            memcpy(fileName, parameters + paramLenArr[0] + paramLenArr[1] + paramLenArr[2], paramLenArr[3]);
            fileName[paramLenArr[3]] = '\0';
        }

        if(forStore){
            // Store and send response to client
            storeFileAndSendResponse(key, fileValue, fileName, senderIP);
        }
        else{
            // Get the file and send response to client
            getFileAndSendResponse(key, fileName, senderIP);
        }
    }
    else if(strcmp(commandName, "listAllFile") == 0){

        generalInfoLog("Received request for listAllFile");
        char* cliMessageToSend = NULL;
        long cliMessageLen = 0;
        
        // Check if the senderID is same as mine. If yes then I need to send this message
        // to the client
        if(isIdEqual(senderID, localNode->getNodeID())){
            
            // Send reponse to the client
            cliMessageToSend = getLSClientMessage(paramCount, totalParamsSize, paramLenArr,
                                                    parameters, cliMessageLen);

            sendResponseToClient(LS_RESP, senderIP, 1, cliMessageToSend, cliMessageLen);

        }
        else
        {
            // check if my successor id is not equal to my Id.
            // Send the request to the successor
            Node* succ = successors.getFirstSuccessor();

            if(succ->getNodeIP().compare(localNode->getNodeIP()) == 0){

                // That means network is unstable. But send the list of message so far collected
                // with result code 0
                cliMessageToSend = getLSClientMessage(paramCount, totalParamsSize, paramLenArr,
                        parameters, cliMessageLen);

                sendResponseToClient(LS_RESP, senderIP, 0, cliMessageToSend, cliMessageLen);
            }
            else{
                // Make a valid message and send to the server
                // Add my file entries and send it to the client
                map<string,Entry> entrylist = localNode->getAllEntries();
                map<string,Entry>::iterator myIter = entrylist.begin();

                int myEntryCount = entrylist.size();

                int totalEntries = myEntryCount + paramCount;
                int* lengthArray = new int[totalEntries];

                // Copy all the older paramters
                memcpy(lengthArray, paramLenArr, paramCount);
                int i = 0;

                for(; myIter != entrylist.end(); ++myIter){

                    lengthArray[paramCount + i] = myIter->second.getFileName().length();
                    ++i;
                }

                int totalLength = totalParamsSize;

                for(int i = 0; i<myEntryCount; i++){
                    totalLength += lengthArray[paramCount + i];
                }

                char* newParameters = new char[totalLength];
                memcpy(newParameters, parameters, totalParamsSize);

                myIter = entrylist.begin();
                i = 0;
                for(; myIter != entrylist.end(); ++myIter){

                    memcpy(newParameters + totalParamsSize + i, 
                            myIter->second.getFileName().c_str(),
                            myIter->second.getFileName().length());
                    i += myIter->second.getFileName().length();
                }

                long largeBufferLen = sizeof(command) + (sizeof(int) * totalEntries) + totalLength;
                char* largeBuffer = new char[largeBufferLen];
                int skipLen = sizeof(command);

                memcpy(largeBuffer, rcvdMsg, skipLen);
                memcpy(largeBuffer + skipLen, lengthArray, totalEntries);
                skipLen += sizeof(int) * totalEntries;
                memcpy(largeBuffer + skipLen, newParameters, totalLength);

                delete[] lengthArray;
                delete[] newParameters;

                sendRequestToServer(LIST_ALL, succ->getNodeIP(), "NULL", 
                        largeBuffer, largeBufferLen);
            }
        }
    }
    else if(strcmp(commandName, "deleteFile") == 0){

        char* delMessage = msgRcvd;
        long messageSize = messageLen;
            
        generalInfoLog("Received request for deleteFile");
        
        // Get the file name and delete it from my node
        char* fileName = new char[paramLenArr[2] + 1];
        int skipLen = paramLenArr[0] + paramLenArr[1];

        memcpy(fileName, parameters + skipLen, paramLenArr[2]);
        fileName[paramLenArr[2]] = '\0';
        skipLen += paramLenArr[2];

        char* deleteStatus = new char[paramLenArr[3] + 1];
        memcpy(deleteStatus, parameters + skipLen, paramLenArr[3]);
        deleteStatus[paramLenArr[3]] = '\0';

        // Check if the message has travelled whole circle
        if(senderID.compare(localNode->getNodeID()) == 0){
            if(strcmp(deleteStatus, "found") == 0){
                sendResponseToClient(DELETE_RESP, senderIP, 1);
            }
            else{
                sendResponseToClient(DELETE_RESP, senderIP, 0);
            }
        }
        else{

            string fileToDel = fileName;
            string fileKey = getLocalHashID(fileName);

            // Check if file is there and delete it
            if(localNode->checkIfEntryExists(fileKey, fileToDel)){
                
                if(strcmp(deleteStatus, "notFound") == 0){
                    
                    // Delete the older message buffer
                    delete[] delMessage;

                    delMessage = NULL;
                    delMessage = getDeleteExistsMessage(senderIP, fileName, messageSize, true,
                                    "found");
                }

                localNode->removeEntry(fileKey, fileToDel);
            }

            // Forward the message to my successor
            Node* succ = successors.getFirstSuccessor();

            // TODO: Here we can check that if my ID is same as my successor id then that mean
            // the network is not stable and there is some error in ring formation.
            // We can send error code to the client. We are not handling it now.
            if(localNode->getNodeIP().compare(succ->getNodeIP()) == 0){

                // Send response to client with result code 0. This might be bcoz network is 
                // not stable
                if(strcmp(deleteStatus, "found") == 0){
                    cout << "Have deleted few instance of the file but not all. The network structure is not stable\n";
                }
                sendResponseToClient(DELETE_RESP, senderIP, 0);

                return;
            }
            
            sendRequestToServer(DELETE_FILE, succ->getNodeIP(), "NULL", delMessage, messageSize);
        }
    }
    else if(strcmp(commandName, "fileExists") == 0){
            
        generalInfoLog("Received request for fileExists");

        // Check if the message has travelled whole circle
        if(senderID.compare(localNode->getNodeID()) == 0){

            // If yes that means that the file is not there in the network
            sendResponseToClient(EXISTS_RESP, senderIP, 0);
        }
        else{
            // Get the file name and check if it exists
            char* fileName = new char[paramLenArr[2] + 1];
            int skipLen = paramLenArr[0] + paramLenArr[1];

            memcpy(fileName, parameters + skipLen, paramLenArr[2]);
            fileName[paramLenArr[2]] = '\0';

            string fileToDel = fileName;
            string fileKey = getLocalHashID(fileName);

            // Check if file is there 
            if(localNode->checkIfEntryExists(fileKey, fileToDel)){
            
                // Send the response to client
                sendResponseToClient(EXISTS_RESP, senderIP, 1);

                return;
            }

            // Forward the message to my successor
            Node* succ = successors.getFirstSuccessor();

            // TODO: Here we can check that if my ID is same as my successor id then that mean
            // the network is not stable and there is some error in ring formation.
            // We can send error code to the client. We are not handling it now.
            if(localNode->getNodeIP().compare(succ->getNodeIP()) == 0){

                // Send response to client with result code 0. This might be bcoz network is 
                // not stable
                sendResponseToClient(EXISTS_RESP, senderIP, 0);

                return;
            }

            sendRequestToServer(EXISTS_FILE, succ->getNodeIP(), "NULL", msgRcvd, messageLen);
        }
    }
    else{
        generalInfoLog("Unknown request received\n");
    }
}


char* Chord::getLSClientMessage(int paramCount, int totalParamSize,
                         int* lenArray, char* params, long& msgLen){


    ClientResponse* msg = new ClientResponse;
    long finalMsgSize = 0;
    char* finalMsg;

        // Send reponse to the client
        msg->type = CLIENT_RESP;
        msg->numParameters = paramCount - 2;
        msg->result = 1;

        finalMsgSize = sizeof(ClientResponse) + (sizeof(int) * (msg->numParameters))
            + (totalParamsSize - lenArray[0] - lenArray[1]);
        finalMsg = new char[finalMsgSize];

        int skipLen = 0;
        memcpy(finalMsg, msg, sizeof(ClientResponse));
        skipLen += sizeof(ClientResponse);

        memcpy(finalMsg + skipLen, lenArray + 2, (sizeof(int) * (msg->numParameters)));
        skipLen += (sizeof(int) * (msg->numParameters));

        int addLen = totalParamsSize - lenArray[0] - lenArray[1];
        memcpy(finalMsg + skipLen, params + lenArray[0] + lenArray[1],  addLen);

    msgLen = finalMsgSize;
    return finalMsg;

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
    memcpy(Msg->senderID, idToSend.data(), idToSend.length());
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

void Chord::handleStabilizeResponse(char* maxMessage)
{
    string predIP;// = inet_ntoa(senderProcAddrUDP.sin_addr);
    string predID;
    
    // Send message to my successor to get his predecessor node
    Node* succNode = successors.getFirstSuccessor();
    
    string succIP = succNode->getNodeIP();
    string succID = succNode->getNodeID();

    handleResponseFromServer(maxMessage, predID, predIP);
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
        
        return;
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
        handleClientPutMessage(file, data, ip, msgRcvd, messageLen);
    }
    else if(strcmp(cmnd, GET) == 0){
        handleClientGetMessage(file,ip,msgRcvd, messageLen);
    }
    else if(strcmp(cmnd, EXISTS) == 0){
        handleClientExistsMessage(ip, file);
    }
    else if(strcmp(cmnd, LS) == 0){
        handleClientLsMessage(ip);
    }
    else if(strcmp(cmnd, DELETE) == 0){
        handleClientDeleteMessage(ip, file);
    }
}

void Chord::handleClientGetMessage(string fileName, string clientIP, 
                                    char* msg, long msgLen)
{
    functionEntryLog("handleClientGetMessage");

    // get the local hash id for this file
    string fileID = getLocalHashID(fileName);

    //string successorID = successors.getFirstSuccessor()->getNodeID();
    Node* successNode = successors.getFirstSuccessor();

    string localNodeID = localNode->getNodeID();

    // find the successor of this fileID
    //Lies between the current node and successor node 
    //
    if(isInInterval(fileID, localNodeID, successNode->getNodeID()))
    {
        // Send the request to my successor as he will have the file
        if(debug){
            printf("Sending file to successor\n");
            fflush(NULL);
        }

        char* messageToSend = getStoreFileMsg(clientIP, fileID, fileName, "NULL", msgLen, false);

        // Send the request to successor to store this file
        sendRequestToServer(GET_FILE, successNode->getNodeIP(), NULL , messageToSend, msgLen);

    }
    else
    {
        string closestPrecedingNodeIP = this->closestPrecedingNode(fileID);

        if(closestPrecedingNodeIP.compare(localNode->getNodeIP()) == 0){

            // I am the only node in the network
            // Send my ID  SEND RESPONSE TO SERVER
            if(debug){
                printf("Sending response with self id. No preceeding node\n");
                fflush(NULL);
            }
            
            // Get the file and send the response to client
            getFileAndSendResponse(fileID, fileName, clientIP);
        }
        else{
            if(debug){
                printf("Forwarding the message to the preceeding node\n");
                fflush(NULL);
            }
            // Forward this message to the preceding Node
            sendRequestToServer(CLIENT_REQ, closestPrecedingNodeIP, "NULL", msg, msgLen);
        }
    }
}

void Chord::getFileAndSendResponse(string fileID, string fileName, string clientIP){
    functionEntryLog("getFileAndSendResponse");

    // Get the entry from the list
    string fileData = localNode->getEntryValue(fileID, fileName);

    // Make the client response message
    ClientResponse* getResp = new ClientResponse;
    getResp->type = CLIENT_RESP;
    getResp->numParameters = 1;
    getResp->result = 1;

    int* lenArray = new int[1];
    lenArray[0] = fileData.length();

    char* param = new char[lenArray[0]];
    memcpy(param, fileData.c_str(), fileData.length());

    long largeBuffSize = sizeof(ClientResponse) + sizeof(int) + lenArray[0];
    char* largeBuff = new char[largeBuffSize];

    int skipLen = sizeof(ClientResponse);
    memcpy(largeBuff, getResp, skipLen);
    memcpy(largeBuff + skipLen, lenArray, sizeof(int));
    skipLen += sizeof(int);
    memcpy(largeBuff + skipLen, param, lenArray[0]);

    // Send Response to the client
    sendResponseToClient(GET_RESP, clientIP, 1, largeBuffSize, largeBuff);
}

char* Chord::getStoreFileMsg(string clientIP, string fileKey, string fileName,
        string fileValue, long& msgSize, bool isPutMsg)
{
    functionEntryLog("getStoreFileMsg");

    if(debug){
        printf("Sending Message details\n");
        printf("Type: SERV_REQ \n");
        printf("SenderID: %s\n", localNode->getNodeID().c_str());
        printf("Length of ID: %ld\n", localNode->getNodeID().length());
        printf("IP to send: %s\n", clientIP.c_str());
        fflush(NULL);
    }

    string cmnd;
    
    if(isPutMsg){
        cmnd = "storeFile";
    }
    else{
        cmnd = "getFile";
    }

    // create the message to send
    command* Msg = new command;
    Msg->type = SERVER_REQ;
    memcpy(Msg->senderID, localNode->getNodeID().c_str(), localNode->getNodeID().length());

    if(isPutMsg){
        Msg->numParameters = 5;
    }
    else{
        Msg->numParameters = 4;
    }

    printf("SenderID in msg: %s\n", Msg->senderID);

    int* paramLen = new int[Msg->numParameters];
    paramLen[0] = cmnd.length();
    paramLen[1] = clientIP.length();
    paramLen[2] = fileKey.length();

    if(isPutMsg){
        paramLen[3] = fileValue.length();
        paramLen[4] = fileName.length();
    }
    else{
        paramLen[3] = fileName.length();
    }

    int totalParamLen = 0;
    for(int i = 0; i<Msg->numParameters; i++){
        totalParamLen += paramLen[i];
    }

    char* params = new char[totalParamLen];
    int skipLen = paramLen[0];
    memcpy(params, cmnd.c_str(), paramLen[0]);
    memcpy(params + skipLen, clientIP.c_str(), paramLen[1]);
    skipLen += paramLen[1];
    memcpy(params + skipLen, fileKey.c_str(), fileKey.length());
    skipLen += fileKey.length();

    if(isPutMsg){
        memcpy(params + skipLen, fileValue.c_str(), fileValue.length());
        skipLen += fileValue.length();
    }
    memcpy(params + skipLen, fileName.c_str(), fileName.length());

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
    messageLen = sizeof(command) + totalParamLen  + sizeof(int) * Msg->numParameters;
    msgBuffer = new char[messageLen];
    memcpy(msgBuffer, Msg, sizeof(command));
    memcpy(msgBuffer + sizeof(command), paramLen, sizeof(int) * Msg->numParameters);
    memcpy(msgBuffer + sizeof(command) + sizeof(int) * Msg->numParameters, params, totalParamLen);


    // Delete the memory allocated for paramLen and params
    delete[] paramLen;
    delete[] params;
    delete Msg;

    // Return the message buffer
    msgSize = messageLen;
    return msgBuffer;
}

void Chord::handleClientPutMessage(string fileName, string fileValue, string clientIP, 
                                    char* msg, long msgLen)
{
    functionEntryLog("handleClientPutMessage");

    // get the local hash id for this file
    string fileID = getLocalHashID(fileName);

    //string successorID = successors.getFirstSuccessor()->getNodeID();
    Node* succNode = successors.getFirstSuccessor();

    string localNodeID = localNode->getNodeID();

    // find the successor of this fileID
    //Lies between the current node and successor node 
    //
    if(isInInterval(fileID, localNodeID, succNode->getNodeID()))
    {
        // Send the file to my successor to store it
        if(debug){
            printf("Sending file to successor\n");
            fflush(NULL);
        }

        char* messageToSend = getStoreFileMsg(clientIP, fileID, fileName, fileValue, msgLen, true);

        // Send the request to successor to store this file
        sendRequestToServer(STORE_FILE, succNode->getNodeIP(), "NULL", messageToSend, msgLen);

    }
    else
    {
        string closestPrecedingNodeIP = this->closestPrecedingNode(fileID);

        if(closestPrecedingNodeIP.compare(localNode->getNodeIP()) == 0){

            // I am the only node in the network
            // Send my ID  SEND RESPONSE TO SERVER
            if(debug){
                printf("Sending response with self id. No preceeding node\n");
                fflush(NULL);
            }
            
            // Store and send the response to client
            storeFileAndSendResponse(fileID, fileValue, fileName, clientIP);
        }
        else{
            if(debug){
                printf("Forwarding the message to the preceeding node\n");
                fflush(NULL);
            }
            // Forward this message to the preceding Node
            sendRequestToServer(CLIENT_REQ, closestPrecedingNodeIP, "NULL", msg, msgLen);
        }
    }   

}

void Chord::storeFileAndSendResponse(string fileID, string fileValue, string fileName, string clientIP){
    functionEntryLog("storeFileAndSendResponse");

    // Store the entry in the list
    localNode->storeEntry(fileID, fileName, fileValue);

    // Send Response to the client
    sendResponseToClient(PUT_RESP, clientIP, 1);
}


void Chord::sendResponseToClient(int method, string receiverIP, int resultCode, long msgSize, char* msg){

    functionEntryLog("CHORD: sendResponseToClient");

    char* message = NULL;
    ClientResponse* myMessage = new ClientResponse;
    myMessage->type = CLIENT_RESP;
    myMessage->result = resultCode;


    long messageLen = 0;

    switch(method){

        case 0:
            {
                generalInfoLog("In case 0");
                
                myMessage->numParameters = 0;

                messageLen = sizeof(ClientResponse);
                message = new char[messageLen];
                memcpy(message, myMessage, messageLen);

                break;
            }
        case 1:
            {
                generalInfoLog("In case 1");
                if(!msg){
                    cout << "ERROR: Get Message is NULL\n";
                    return;
                }
                else{
                    message = msg;
                    messageLen = msgSize;
                }

                break;
            }
        case 2:
            {
                generalInfoLog("In case 2");

                myMessage->numParameters = 0;
                messageLen = sizeof(ClientResponse);
                message = new char[messageLen];
                memcpy(message, myMessage, messageLen);

                break;
            }
        case 3:
            {
                generalInfoLog("In case 3");

                if(!msg){
                    messageLen = sizeof(ClientResponse);

                    map<string,Entry> entriesToBeSent = this->getAllEntries();
                    int i = 0, totalSize = 0, lenArraySize = 0, paramSize = 0, totalEntries = 0;

                    totalEntries = entriesToBeSent.size();
                    myMessage->numParameters = totalEntries;

                    lenArraySize = sizeof(int) * (myMessage->numParameters);

                    int* lenArray = new int[lenArraySize];
                    map<string,Entry>::iterator myIter = entriesToBeSent.begin();

                    for(; myIter!= entriesToBeSent.end();myIter++)
                    {
                        // Store the length of each value
                        lenArray[i++] = myIter->second.getFileName().length();
                        totalSize += myIter->second.getFileName().length();
                    }
                    messageLen += lenArraySize;

                    paramSize = totalSize;
                    messageLen += paramSize;

                    char* params = new char[paramSize];
                    totalSize = 0;

                    for(myIter = entriesToBeSent.begin();
                            myIter!= entriesToBeSent.end();myIter++)
                    {
                        // Store all the names
                        memcpy(params + totalSize, myIter->second.getFileName().c_str(), lenArray[i]);
                        totalSize += lenArray[i++];

                    }

                    // Allocate the large buffer to serialize
                    message = new char[messageLen];

                    int addLen = sizeof(ClientResponse);
                    memcpy(message, myMessage, sizeof(ClientResponse));
                    memcpy(message + addLen, lenArray, lenArraySize);
                    addLen += lenArraySize;
                    memcpy(message + addLen, params, paramSize);

                    delete[] lenArray;
                    delete[] params;
                }
                else{
                    message = msg;
                    messageLen = msgSize;
                }

            }
        case 4:
            {
                generalInfoLog("In case 4");
        
                myMessage->numParameters = 0;
                messageLen = sizeof(ClientResponse);
                message = new char[messageLen];
                memcpy(message, myMessage, messageLen);

                break;

            }

        default:
            generalInfoLog("Error while sending. Unkonwn command request found\n");
    }

    delete myMessage;

    // check if socket is open for the servers
    if(getClientSocket() == -1){

        printf("Opening socket again\n");
        // Need to open a socket
        if((clientSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1){
            printf("Error while opening socket to send the message\n");
            exit(1);
        }
    }

    struct sockaddr_in receiverAddr;

    memset((char*)&receiverAddr, 0, sizeof(receiverAddr));
    receiverAddr.sin_family = AF_INET;

    receiverAddr.sin_port = htons(CLIENT_PORT);

    if(inet_aton(receiverIP.c_str(), &receiverAddr.sin_addr) == 0){
        printf("INET_ATON Failed\n");
    }

    if(debug){
        printf("Sending message of length: %ld\n", messageLen);
        fflush(NULL);
    }

    if(sendto(clientSocket, message, messageLen, 0,
                (struct sockaddr*) &receiverAddr, sizeof(receiverAddr)) == -1){

        printf("%s: Failed to send the message type %d to client: %s",
                localNode->getNodeIP().c_str(), SERVER_RES, receiverIP.c_str());
        fflush(NULL);
    }
    else{
        printf("%s: successfully sent the message type %d to %s\n",
                localNode->getNodeIP().c_str(), SERVER_RES, receiverIP.c_str());
        fflush(NULL);
    }

}

void Chord::handleClientLsMessage(string clientIP)
{
    functionEntryLog("handleClientLsMessage");
    
    // create a command message with all of my entries and send it to other server
    // insert my ID in senderID field. I will reply to client with all the file names
    // once I have received all the filenames.
    string cmnd = "listAllFile";
    long messageLen = 0;
    char* message = makeBufferToSend(cmnd, messageLen, localNode->getNodeID(), clientIP);

    // Get my successor  node
    Node* succ = successors.getFirstSuccessor();

    // Check if my successor ip is same as my ip.
    // if yes then I am the only node in the network and will send response to the
    // client
    if(succ->getNodeIP().compare(localNode->getNodeIP()) == 0){

        // Send response to the client with my file entries
        sendResponseToClient(LS_RESP, clientIP, 1);
    }
    else{
        // Send this request to all the servers in liner way. I will forward it to my
        // successor and so on
        sendRequestToServer(LIST_ALL, succ->getNodeIP(), "NULL", message, messageLen);
    }
}

void Chord::handleClientDeleteMessage(string clientIP, string fileName)
{
    functionEntryLog("handleClientDeleteMessage");
    bool isFilePresent = false;

    // Get the hash key for the file name
    string fileKey = getLocalHashID(fileName);

    // Check if file exists
    if(localNode->checkIfEntryExists(fileKey, fileName)){
        isFilePresent = true;
    }
    else{
        isFilePresent = false;
    }

    // Delete all the instance of the file in my entry list
    localNode->removeEntry(fileKey, fileName);

    // Pass this fileName to the other node via successor node
    Node* succ = successors.getFirstSuccessor();

    // Check if my successor ip is same as my ip.
    // if yes then I am the only node in the network and will send response to the
    // client
    if(succ->getNodeIP().compare(localNode->getNodeIP()) == 0){

        // Send response to the client
        if(isFilePresent){
            sendResponseToClient(DELETE_RESP, clientIP, 1);
        }
        else{
            sendResponseToClient(DELETE_RESP, clientIP, 0);
        }
    }
    else
    {
        // Create the message and send it
        long messageLen = 0;
        char* message  = getDeleteExistsMessage(clientIP, fileName, messageLen, true, "notFound");
        // Send this request to all the servers in liner way. I will forward it to my
        // successor and so on
        sendRequestToServer(DELETE_FILE, succ->getNodeIP(), "NULL", message, messageLen);

    }
}

char* Chord::getDeleteExistsMessage(string ipToSend, string fileName, long& msgSize, bool isDelete,
                                    string deleteStat)
{
    functionEntryLog("getDeleteExistsMessage");

    command* delMsg = new command;
   
    string commandName;

    if(isDelete){
        commandName = "deleteFile";
    }
    else
    {
        commandName = "fileExists";
    }

    delMsg->type = SERVER_REQ;
    memcpy(delMsg->senderID, localNode->getNodeID().c_str(), localNode->getNodeID().length());

    if(isDelete){
        delMsg->numParameters = 4;
    }
    else{
        delMsg->numParameters = 3;
    }

    int* lenArray = new int[delMsg->numParameters];
    lenArray[0] = commandName.length();
    lenArray[1] = ipToSend.length();
    lenArray[2] = fileName.length();

    if(isDelete){
        lenArray[3] = deleteStat.length();
    }
    
    long paramSize = commandName.length() + ipToSend.length() + fileName.length();
    
    if(isDelete){
        paramSize += deleteStat.length();
    }

    char* parameters = new char[paramSize];

    long skipLen = commandName.length();
    memcpy(parameters, commandName.c_str(), skipLen);
    memcpy(parameters + skipLen, ipToSend.c_str(), ipToSend.length());
    skipLen += ipToSend.length();
    memcpy(parameters + skipLen, fileName.c_str(), fileName.length());
    skipLen += fileName.length();

    if(isDelete){
        memcpy(parameters + skipLen, deleteStat.c_str(), deleteStat.length()); 
    }

    msgSize = sizeof(command) + (sizeof(int) * delMsg->numParameters) + paramSize;
    char* msgBuffer = new char[msgSize];

    skipLen = sizeof(command);
    memcpy(msgBuffer, delMsg, skipLen);
    memcpy(msgBuffer + skipLen, lenArray, (sizeof(int) * delMsg->numParameters));
    skipLen += (sizeof(int) * delMsg->numParameters);
    memcpy(msgBuffer + skipLen, parameters, paramSize);
    
    delete[] lenArray;
    delete[] parameters;
    delete delMsg;

    return msgBuffer;
}

void Chord::handleClientExistsMessage(string clientIP, string fileName)
{
    functionEntryLog("handleClientExistsMessage");

    // Get the file hash key
    string fileKey = getLocalHashID(fileName);

    // Check if file exists in my entry list
    if(localNode->checkIfEntryExists(fileKey, fileName)){

        // Send the client response with result code 1
        sendResponseToClient(EXISTS_RESP, clientIP, 1);

        return;
    }
    else
    {
        // Send the message to the successor server
        Node* succ = successors.getFirstSuccessor();

        // Check if I am the successor of myself that means
        // I am the only node in the network
        if(localNode->getNodeIP().compare(succ->getNodeIP()) == 0){

            // Check if entryexists on my system
            if(localNode->checkIfEntryExists(fileKey, fileName)){

                //send response to the client
                sendResponseToClient(EXISTS_RESP, clientIP, 1);

                return;
            }
            else{
                // Send failure response to the client
                sendResponseToClient(EXISTS_RESP, clientIP, 0);

                return;
            }
        }
        else
        {
            // Forward the request to the successor
            long msgLen = 0;
            char* message = getDeleteExistsMessage(clientIP, fileName, msgLen, false, "NULL");

            sendRequestToServer(EXISTS_FILE, succ->getNodeIP(), "NULL", message, msgLen);
        }
    }
}
