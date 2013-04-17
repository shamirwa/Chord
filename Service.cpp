#include "Service.h"
#include "Chord.h"
#include "Defs.h"
#include "myUtils.h"
#include "message.h"
#include <sys/time.h>


int main(int argc, char* argv[]){

    bool isFirstNode = true;
    string bootstrapNode;
    string selfName;
    string selfIP;
    string selfID;
    struct in_addr **ipAddr;
    struct hostent* he;
    int clientSocket, serverSocket;
    
    // Stabilize Variable
    struct timeval stabilizeTimer, endTime, selectTimer;

    // Parse the command line arguments
    if(argc == 3){
        if(strcmp(argv[1], "--bsNode") == 0){
            bootstrapNode = argv[2];
        }
        isFirstNode = false;
    }

    if(!isFirstNode){
        printf("Joining the chord network using the bootstrap node: %s\n", bootstrapNode.c_str());
    }
    else{
        printf("This the first node to create the chord network\n");
    }

    // Get self hostname
    char myName[100];
    gethostname(myName, 100);

    selfName = myName;

    if((he = gethostbyname(selfName.c_str())) == NULL){
        fprintf(stderr, "Unable to get the ip address of the host: %s\n",
                selfName.c_str());
        exit(1);
    }

    //Store the ip address
    ipAddr = (struct in_addr**)he->h_addr_list;
    selfIP = inet_ntoa(*ipAddr[XINU]);

    if(selfIP.find("127") == 0){
        selfIP = inet_ntoa(*ipAddr[VM]);
    }
    else{
        selfIP = inet_ntoa(*ipAddr[XINU]);
    }


    if(debug){
        printf("Host Name: %s, IP Address: %s \n", selfName.c_str(), selfIP.c_str());
    }

    // Compute the local id of this node using SHA1 hash function
    // Defined in Defs.h
    selfID = getLocalHashID(selfIP);

    // ******* COMMUNICATION VARIABLES ******//
    if((clientSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1){
        generalInfoLog("Error when trying to open the socket for client\n");
        exit(1);
    }

    if((serverSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1){
        generalInfoLog("Error when trying to open the socket for servers\n");
        exit(1);
    }

    /*
       for(int i = 0; i<20; ++i){
       printf("%d * ",selfID[i]);
       }
       printf("\n");
       */

    // Create an object of the chord class
    Chord myChordInstance(selfID, selfIP, NUM_SUCCESSOR, clientSocket, serverSocket);

    // Check if this is the first node or not
    if(!isFirstNode){
        // This node is not the first node to join. SO call the join method with the
        // bootstrap node
        myChordInstance.join(bootstrapNode);
    }
    else{
        // This is the first node to join/ Call the create method
        myChordInstance.create();
    }

    // SELECT with both client and server socket
    /****** COMMUNICATION VARIABLES ********/
    struct sockaddr_in rcvrAddrUDP;
    struct sockaddr_in senderProcAddrUDP;
    struct sockaddr_in myInfoUDP,myInfoUDP2;


    // To store the address of the process from whom a message is received
    memset((char*)&senderProcAddrUDP, 0, sizeof(senderProcAddrUDP));
    socklen_t senderLenUDP = sizeof(senderProcAddrUDP);

    // store the info for sending the message to a process. rcvrAddr will have all the info
    // about the process whom we are sending the message
    memset((char*)&rcvrAddrUDP, 0, sizeof(rcvrAddrUDP));
    rcvrAddrUDP.sin_family = AF_INET;
    rcvrAddrUDP.sin_port = htons(SERVER_PORT);

    // Store the info to bind receiving port with the socket.
    memset((char*)&myInfoUDP, 0, sizeof(myInfoUDP));
    myInfoUDP.sin_family = AF_INET;
    myInfoUDP.sin_port = htons(SERVER_PORT);
    myInfoUDP.sin_addr.s_addr = htonl(INADDR_ANY);

    // Store the info to bind receiving port with the socket.
    memset((char*)&myInfoUDP2, 0, sizeof(myInfoUDP2));
    myInfoUDP2.sin_family = AF_INET;
    myInfoUDP2.sin_port = htons(CLIENT_PORT);
    myInfoUDP2.sin_addr.s_addr = htonl(INADDR_ANY);


    // bind the UDP receiver socket
    if(bind(serverSocket, (struct sockaddr*) &myInfoUDP, sizeof(myInfoUDP)) == -1){
        generalInfoLog("Bind failed for receiving socket \n");
        exit(1);
    }

    if(bind(clientSocket, (struct sockaddr*) &myInfoUDP2, sizeof(myInfoUDP2)) == -1){
        generalInfoLog("Bind failed for client socket\n");
        exit(1);
    }

    fd_set read_fds; // set of read fds.

    FD_ZERO(&read_fds); // clear the read fd set

    // Add the UDP socket to the master list too
    FD_SET(serverSocket, &read_fds);
    FD_SET(clientSocket, &read_fds);

    int fdmax = (serverSocket > clientSocket ? serverSocket : clientSocket);

    // start the stabilize timer
    gettimeofday(&stabilizeTimer, NULL);


    int selectReturnValue;
    for(;;){


        // Check for stabilize timer expiry
        gettimeofday(&endTime, NULL);
        long diff = ((endTime.tv_sec * 1000000 + endTime.tv_usec) -
                                ((stabilizeTimer.tv_sec * 1000000) + (stabilizeTimer.tv_usec)));

        if(diff > STABILIZE_TIMEOUT){
            generalInfoLog("stabilizeTimer has expired\n");
            selectTimer.tv_sec = 0;
            selectTimer.tv_usec = 0;
        }
        else{
            diff = STABILIZE_TIMEOUT - diff;
            long int sec  =  diff / 1000000;
            long int msec = diff - (sec * 1000000);
            selectTimer.tv_sec = sec;
            selectTimer.tv_usec = msec;

        }


        selectReturnValue = -1;
        selectReturnValue = select(fdmax + 1, &read_fds, NULL, NULL, &selectTimer);

        if(selectReturnValue == -1){
            generalInfoLog("Error returned from the select\n");
            continue;
        }
        else if(selectReturnValue == 0){
            // Stabilize timer has expired

            myChordInstance.stabilize();

            // Reset the timer
            gettimeofday(&stabilizeTimer, NULL);
        }
        else{
            cout << "Received MEssage\n";
            // Run through the existing connections for the data to be read
            for(int i = 0; i<=fdmax; i++){
                if(FD_ISSET(i, &read_fds)){ // We got one active fd

                    if(i == clientSocket){

                        //handleClientRequest();
                    }
                    else if(i == serverSocket){

                        // Received a message from server
                        char* maxMessage = new char[MAX_MSG_SIZE];
                        struct sockaddr_in senderProcAddrUDP;

                        // To store the address of the process from whom a message is received
                        memset((char*)&senderProcAddrUDP, 0, sizeof(senderProcAddrUDP));
                        socklen_t senderLenUDP = sizeof(senderProcAddrUDP);

                        int recvRet = 0;

                        recvRet = recvfrom(serverSocket, maxMessage, MAX_MSG_SIZE,
                                0, (struct sockaddr*) &senderProcAddrUDP, &senderLenUDP);

                        //string senderIP = inet_ntoa(senderProcAddrUDP.sin_addr);

                        if(recvRet > 0){
                            // Get the type of the message
                            uint32_t* msgType = (uint32_t*)(maxMessage);
                            uint32_t type = *msgType;

                            if(type == SERVER_REQ){		
                                myChordInstance.handleRequestFromServer(maxMessage, recvRet);
                            }
                            else{
                                cout << "SERVICE: Invalid message received: " << type << endl;
                            }
                        }				
                    }
                }
            }// ENd of fd set for loop
        }
    }// ENd of infinite loop

    return 0;
}
