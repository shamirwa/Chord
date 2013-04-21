
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

#define XINU 0
#define VM 1
#define debug 1


using namespace std;

int client_sockfd;

void recieveOutputFromServer()
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
        printf("Response from server %s \n",string(maxMessage).c_str());				}
}


void sendRequestToServer(string commandName,string receiverIP,string filename,string data,string myIP)
{
    //Socket variables for client
    int n;
    struct sockaddr_in servaddr;
    socklen_t lensock;

    client_sockfd = socket(AF_INET,SOCK_DGRAM,0);

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
    receiverAddr.sin_port = htons(SERVER_PORT);

    if(inet_aton(receiverIP.c_str(), &receiverAddr.sin_addr) == 0){
        printf("INET_ATON Failed\n");
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

    cout<<"List of operations available are: "<<endl;
    cout<<"0 - put"<<endl;
    cout<<"1 - get"<<endl;
    cout<<"2 - exists"<<endl;
    cout<<"3 - ls"<<endl;
    cout<<"4 - delete"<<endl;

    while(1)
    {
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
                    recieveOutputFromServer();
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
                    recieveOutputFromServer();
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
                    recieveOutputFromServer();

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
                    recieveOutputFromServer();

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
                    recieveOutputFromServer();
                    break;
                }
            default:
                {
                    cout<<"Please select an option from the available choices"<<endl;
                }
        }
    }

}
