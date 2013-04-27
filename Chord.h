#ifndef _CHORD_H
#define _CHORD_H

#include <map>
#include "Successorlist.h"

using namespace std;
/*
 *
 * This class will handle the core functionality of the Chord Protocol. 
 *
 * */

class Chord
{
    private:
        Node* localNode; //stores the local ip, local id and entry list of a node
        Node* predecessor; //stores the predecessor of this node
        SuccessorList successors; //stores the successors
        int clientSocket;
        int serverSocket;
        int stabilizeSocket;
        int fingerTableSocket;
        int pingSocket;
        bool pingSent;


    public:
        Chord();
        Chord(string localID,string localIP,int numSuccessor,int clientSocket,int serverSocket, 
                    int stabilizeSocket, int fingerSocket, int pingSock);
        ~Chord();

        void createHelp(); //Performs all necessary tasks for creating a new chord ring	


        void create(); //creates a new chord network which is not connected to any other node

        void joinHelp(string IP); // Performs all necessary tasks for joining an existing Chord ring.

        void join(string IP); //joins an existing node with ip address "IP"

        void stabilize(); //runs the stabilize operations of this node

        string getLocalIP();

        string getLocalID();

        string closestPrecedingNode(string id); //search the local table for the highest predecessor of id

        void findSuccessor(string IP, char* message, long messageLen, bool isFingerMsg = false); //finds the successor of a given node with IP "IP"

        void leave(); //runs the leave operations for this node, updates information of adjacent nodes

        void notify(string predecessor); //This method is invoked by another node which thinks it is this node's predecessor

        void ping(); // Requests a sign of live. This method is invoked by another node which thinks it is this node's successor.

        void setPingFlag(bool status);

        /*
         * Stores a finger table in the form:
         * 0 ==> <id,ip>
         * 1 ==> <id,ip>
         *
         *
         */
        map<int,pair<string,string> > fingerTable; 

        void buildFingerTable(string IP, string ID, bool isForStab = false); //Builds the finger table

        void insert(string id, string fileContent); //Inserts a new data object into the network with the Key 'id' and Value 'fileContent'

        void remove(string id, string fileContent); //Removes a file with the Key 'id' and value 'fileContent'

        void disconnect(); //Disconnects this node

        void retrieve(string id); //Retrieves the file with the Key 'id'

        void insertReplicas(string id); //Insert replica of a file with Key 'id'

        void sendRequestToServer(int method, string rcvrIP, string idToSend, char* message = NULL, 
                long msgLen = 0, bool isFingerMsg = false);

        void sendResponseToServer(int method, string responseID, string responseIP, string receiverIP, bool isFingerMsg = false);

        void handleResponseFromServer(char* msgRcvd, string& responseID, string& responseIP); 

        void handleRequestFromServer(char* msgRcvd, long messageLen, bool isForFinger = false);

        void handleRequestFromClient(char* msgRcvd, long messageLen);

        int getServerSocket();

        int getClientSocket();

        void setServerSocket(int servSock);

        void setClientSocket(int cliSock);

        Node* getPredecessor();

        Node* getFirstSuccessor();

        void setPredecessor(string predIP, string predID);

        char* getMessageToSend(int msgType, string cmnd, string idToSend, string ipToSend,
                long& msgLength);

        map<string, Entry> getAllEntries();

        char* makeBufferToSend(string commandName, long& msgLength,string predID, string predIP, bool isLeave = false);

        void storeFileAndSendResponse(string fileID,  string fileValue, string fileName, string clientIP);

        void handleClientPutMessage(string fileName, string fileValue, string clientIP,
                                            char* msg, long msgLen);
        void handleClientLsMessage(string clientIP);

        char* getStoreFileMsg(string clientIP, string fileKey, string fileName, 
                string fileValue, long& msgSize, bool isPutMsg);

        void sendResponseToClient(int method, string receiverIP, int resultCode, long msgSize = 0, char* msg = NULL);

        void handleStabilizeResponse(char* maxMessage);

        void handleClientGetMessage(string fileName, string clientIP, 
                                        char* msg, long msgLen);
        void getFileAndSendResponse(string fileID, string fileName, string clientIP);

        void handleClientExistsMessage(string clientIP, string fileName);

        void handleClientDeleteMessage(string clientIP, string fileName);

        char* getDeleteExistsMessage(string ipToSend, string fileName, long& msgSize, bool isDelete, string deleteStat);

        char* getLSClientMessage(int paramCount, int totalParamSize, int* lenArray, char* params, long& msgLen);

        void removeReference(Node* nodeToRem);

        void unsetEntry(int index);

        void setEntry(int index,string referenceForReplacementID,string referenceForReplacementIP);

        void addReference(Node* nodeToAdd);

};


#endif
