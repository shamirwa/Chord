
/* Client is used to ping server for a file name or put, get, ls , exists
 * Usage: ./Client
 */

/* TODO: When client receives the output of the ls command then it will
 * look into all the file names and remove the duplicate one.
 */

#include <iostream>
#include <stdio.h>
#include <cstdlib>
#include "Defs.h"
#include "myUtils.h"
#include "message.h"
#include <set>

#define XINU 0
#define VM 1
#define debug 1
#define FILE_DNOT_EXIST " File does not exist on the cloud "
#define FILE_EXISTS  " File Exists on the cloud "
#define FILE_STORED  " File was stored on the cloud "
#define FILE_NOT_STORED " File could not be stored on the ring. Try again as the ring is not formed yet. " 

#define ERROR " Unknown error "

#define FILE_DELETED " File deleted from cloud"
#define FILE_NOT_DELETED " File could not be deleted"
#define DATA_START " DATA : "
#define NO_FILES " No Files: "
#define LIST " LIST: "
#define ERROR_NO_FILES " No File: Ring is not Stabilized. Try again "
#define ERROR_SOME_FILES " Could retrieve some files. Cannot traverse entire ring. In stabilize phase "

using namespace std;

int client_sockfd;

void printSet(set<string> fileSet)
{
    
    cout<<LIST<<endl;    
    for (set<string>::iterator it=fileSet.begin(); it!=fileSet.end(); ++it)
        cout << *it << endl;

    cout<<endl;
}


void helperLS(char* maxMessage, int numParam)
{

    set<string> fileSet;          
    int numParameters = numParam;
    int skipLenInt = sizeof(ClientResponse);
    int skipLenEntry = sizeof(ClientResponse) + sizeof(int)*numParameters;

    for(int i=0; i < numParameters; i++)
    {
        int* length = new int[1];
        memcpy(length,maxMessage + skipLenInt, sizeof(int));
        skipLenInt += sizeof(int);
        char* entry = new char[*length];
        memcpy(entry,maxMessage+skipLenEntry,(*length)*sizeof(char));
        skipLenEntry += (*length)*sizeof(char);
        fileSet.insert(entry);

    }        
    printSet(fileSet);
}


void recieveOutputFromServer(string commandName)
{

    // wait for response message to come
    char* maxMessage = new char[MAX_MSG_SIZE];
    struct sockaddr_in senderProcAddrUDP;

    // To store the address of the process from whom a message is received
    memset((char*)&senderProcAddrUDP, 0, sizeof(senderProcAddrUDP));
    socklen_t senderLenUDP = sizeof(senderProcAddrUDP);


    int recvRet = 0;

    recvRet = recvfrom(client_sockfd, maxMessage, MAX_MSG_SIZE,
            0, (struct sockaddr*) &senderProcAddrUDP, &senderLenUDP);

    if(recvRet > 0)
    {
        ClientResponse* clientResponse = new ClientResponse;
        clientResponse = (ClientResponse *)maxMessage;

        if(commandName == EXISTS)
        {
            if(clientResponse->result == 1)
            {
                cout<< FILE_EXISTS <<endl;
            }
            else if(clientResponse->result == 0)
            {
                cout<< FILE_DNOT_EXIST <<endl;
            }
            else
            {
                cout<< ERROR <<endl;    
            }

        }
        else if(commandName == PUT)
        {
            if(clientResponse->result == 1)
                cout<< FILE_STORED <<endl;
            else if(clientResponse->result == 0)
                cout<< FILE_NOT_STORED <<endl;
            else
                cout<< ERROR <<endl;

        }
        else if(commandName == GET)
        {
            if(clientResponse->result == 0)
                cout<< FILE_DNOT_EXIST <<endl;
            else if(clientResponse->result == 1)
            {
                long buffSize = recvRet - (sizeof(ClientResponse)+sizeof(int));
                char* responseString = new char[buffSize + 1];
                memcpy(responseString,maxMessage + sizeof(ClientResponse)+sizeof(int), buffSize);
                responseString[buffSize] = '\0';

                cout<< DATA_START <<endl;
                cout<< responseString <<endl;

            }
            else
            {
                cout<< ERROR <<endl;
            }
        }
        else if(commandName == LS)
        {
            if(clientResponse->result == 1 && clientResponse->numParameters ==0)
            {
                cout << NO_FILES << endl;
            }
            else if(clientResponse->result == 1 && clientResponse->numParameters > 0)
            {
                helperLS(maxMessage,clientResponse->numParameters);        
            }
            else if(clientResponse->result == 0 && clientResponse->numParameters > 0)
            {
                cout<< ERROR_SOME_FILES << endl;
                helperLS(maxMessage,clientResponse->numParameters);
            }
            else if(clientResponse->result == 0 && clientResponse->numParameters == 0)
            {
                cout << ERROR_NO_FILES <<endl;
            }
            else
            {
                cout<< ERROR <<endl;   
            }       
        }
        else if(commandName == DELETE)
        {
            if(clientResponse->result == 1)
                cout<< FILE_DELETED <<endl;
            else if(clientResponse->result == 0)
                cout<< FILE_NOT_DELETED <<endl;
            else
                cout<< ERROR <<endl;
        }
        else
        {
            cout<< ERROR <<endl;
        }

    }
    else
    {
        cout<< " Connection Closed or other error " <<endl;
    }
}

void setSystemParam()
{
     //Socket variables for client
    int n;
    struct sockaddr_in servaddr;
    socklen_t lensock;

    client_sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if(client_sockfd == -1)
    {
        printf("Could not create socket \n");
        exit(EXIT_FAILURE);
    }

    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
    servaddr.sin_port=htons(CLIENT_PORT);

    int bval = bind(client_sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr));

    if(bval==-1)
    {
        printf("Bind failed\n");
        exit(EXIT_FAILURE);
    }
}


void sendRequestToServer(string commandName,string receiverIP,string filename,string data,string myIP)
{
    //Create a client request
    ClientRequest* req = new ClientRequest;
    req->type = CLIENT_REQ;
    req->lengthFileName = filename.length();
    req->lengthCommandName = commandName.length();
    req->lengthFileData = data.length();
    req->lengthClientIP= myIP.length();

    char* msgBuffer = NULL;
    long messageLen = 0;
    messageLen = sizeof(ClientRequest) + filename.length() + commandName.length() + data.length() + myIP.length();

    msgBuffer = new char[messageLen];
    memcpy(msgBuffer,req,sizeof(ClientRequest));
    memcpy(msgBuffer+sizeof(ClientRequest),filename.c_str(),filename.length());
    memcpy(msgBuffer+sizeof(ClientRequest)+filename.length(),commandName.c_str(),commandName.length());
    memcpy(msgBuffer+sizeof(ClientRequest)+filename.length()+commandName.length(),data.c_str(),data.length());
    memcpy(msgBuffer+sizeof(ClientRequest)+filename.length()+commandName.length()+data.length(),myIP.c_str(),myIP.length());

    struct sockaddr_in receiverAddr;

    memset((char*)&receiverAddr, 0, sizeof(receiverAddr));
    receiverAddr.sin_family = AF_INET;
    receiverAddr.sin_port = htons(CLIENT_PORT);

    if(inet_aton(receiverIP.c_str(), &receiverAddr.sin_addr) == 0){
        printf("INET_ATON Failed\n");
        exit(1);
    }

    if(sendto(client_sockfd, msgBuffer, messageLen, 0,
                (struct sockaddr*) &receiverAddr, sizeof(receiverAddr)) == -1){

        fprintf(stderr, "Failed to send the message type %s to server: %s",
                commandName.c_str(), receiverIP.c_str());
        fflush(stderr);
    }
    else{
        fprintf(stderr, "Successfully sent the message type %s to %s\n",
                commandName.c_str(), receiverIP.c_str());
        fflush(stderr);
    }

}




int main(int argc,char **argv)
{

    // Get self hostname
    char myName[100];
    gethostname(myName, 100);
    string selfName;
    struct hostent* he;

    selfName = myName;

    if((he = gethostbyname(selfName.c_str())) == NULL){
        fprintf(stderr, "Unable to get the ip address of the host: %s\n",
                selfName.c_str());
        exit(1);
    }

    struct in_addr **ipAddr;

    string selfIP;

    //Store the ip address
    ipAddr = (struct in_addr**)he->h_addr_list;
    selfIP = inet_ntoa(*ipAddr[XINU]);

    if(selfIP.find("127") == 0){
        selfIP = inet_ntoa(*ipAddr[VM]);
    }
    else{
        selfIP = inet_ntoa(*ipAddr[XINU]);
    }

    cout<<"Self IP is: "<<selfIP<<endl;

    int command;

        setSystemParam();
    
    while(1)
    {
        cout<<"List of operations available are: "<<endl;
        cout<<"0 - put"<<endl;
        cout<<"1 - get"<<endl;
        cout<<"2 - exists"<<endl;
        cout<<"3 - ls"<<endl;
        cout<<"4 - delete"<<endl;

        cout<<"Please enter the operation (0-4) you intend to do: "<<endl;
        cin >> command;

        switch(command)
        {
            case 0:
                {
                    cout<< "You selected put"<<endl;
                    cout<< "Please enter the file name"<<endl;

                    string filename;
                    cin>>filename;

                    cout<< "Please enter the IP address of the server"<<endl;

                    string serverIP;
                    cin>>serverIP;

                    string data = DATA;				
                    string commandName = PUT;		

                    sendRequestToServer(commandName,serverIP,filename,data,selfIP);
                    recieveOutputFromServer(commandName);
                    break;
                }
            case 1:
                {
                    cout<< "You selected get"<<endl;
                    cout<< "Please enter the file name"<<endl;

                    string filename;
                    cin>>filename;

                    cout<< "Please enter the IP address of the server"<<endl;

                    string serverIP;
                    cin>>serverIP;

                    string data = "";				
                    string commandName = GET;		

                    sendRequestToServer(commandName,serverIP,filename,data,selfIP);
                    recieveOutputFromServer(commandName);
                    break;
                }
            case 2:
                {
                    cout<< "You selected exists"<<endl;
                    cout<< "Please enter the file name"<<endl;

                    string filename;
                    cin>>filename;

                    cout<< "Please enter the IP address of the server"<<endl;

                    string serverIP;
                    cin>>serverIP;

                    string data = "";				
                    string commandName = EXISTS;		

                    sendRequestToServer(commandName,serverIP,filename,data,selfIP);
                    recieveOutputFromServer(commandName);

                    break;
                }
            case 3:
                {
                    cout<< "You selected ls"<<endl;

                    cout<< "Please enter the IP address of the server"<<endl;

                    string serverIP;
                    cin>>serverIP;

                    string filename = "";
                    string data = "";				
                    string commandName = LS;		

                    sendRequestToServer(commandName,serverIP,filename,data,selfIP);
                    recieveOutputFromServer(commandName);

                    break;
                }
            case 4:
                {
                    cout<< "You selected delete"<<endl;
                    cout<< "Please enter the file name"<<endl;

                    string filename;
                    cin>>filename;

                    cout<< "Please enter the IP address of the server"<<endl;

                    string serverIP;
                    cin>>serverIP;

                    string data = "";				
                    string commandName = DELETE;		

                    sendRequestToServer(commandName,serverIP,filename,data,selfIP);
                    recieveOutputFromServer(commandName);
                    break;
                }
            default:
                {
                    cout<<"Please select an option from the available choices"<<endl;
                }
        }
    }

}
