#include "Service.h"
#include "Chord.h"
#include "Defs.h"
#include "myUtils.h"
#include "message.h"
#include <sys/time.h>

string selfIP;
Chord* myChordInstance;


// Define the function to be called when ctrl-c (SIGINT) signal is sent to process
void  signal_callback_handler(int signum)
{
    printf("Caught signal %d\n",signum);
    // Cleanup and close up stuff here
    cout<<"Testing Leaving Chord: "<<endl;
    myChordInstance->leave();
    // Terminate program
    exit(signum);
}



int main(int argc, char* argv[]){


    // Register signal and signal handler
    signal(SIGINT, signal_callback_handler);

        
    bool isFirstNode = true;
    string bootstrapNode;
    string selfName;
    //string selfIP;
    string selfID;
    struct in_addr **ipAddr;
    struct hostent* he;
    int clientSocket, serverSocket, stabilizeSocket;
    
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

    printf("Local Hash ID length: %ld\n", selfID.length());
    fflush(NULL);

    // ******* COMMUNICATION VARIABLES ******//
    if((clientSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1){
        generalInfoLog("Error when trying to open the socket for client\n");
        exit(1);
    }

    if((serverSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1){
        generalInfoLog("Error when trying to open the socket for servers\n");
        exit(1);
    }
    
    if((stabilizeSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1){
        generalInfoLog("Error when trying to open the socket for stabilize\n");
        exit(1);
    }
    printf("Server Socket: %d\n", serverSocket);
    printf("Client Socket: %d\n", clientSocket);
    printf("Stabilize Socket: %d\n", stabilizeSocket);
    
    // SELECT with both client and server socket
    /****** COMMUNICATION VARIABLES ********/
    struct sockaddr_in senderProcAddrUDP;
    struct sockaddr_in myInfoUDP,myInfoUDP2, myInfoUDP3;


    // To store the address of the process from whom a message is received
    memset((char*)&senderProcAddrUDP, 0, sizeof(senderProcAddrUDP));
    socklen_t senderLenUDP = sizeof(senderProcAddrUDP);

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

    // Store the info to bind receiving port with the socket.
    memset((char*)&myInfoUDP3, 0, sizeof(myInfoUDP3));
    myInfoUDP3.sin_family = AF_INET;
    myInfoUDP3.sin_port = htons(STABILIZE_PORT);
    myInfoUDP3.sin_addr.s_addr = htonl(INADDR_ANY);

    // bind the UDP receiver socket
    if(bind(serverSocket, (struct sockaddr*) &myInfoUDP, sizeof(myInfoUDP)) == -1){
        generalInfoLog("Bind failed for server socket \n");
        exit(1);
    }
    
    if(bind(stabilizeSocket, (struct sockaddr*) &myInfoUDP3, sizeof(myInfoUDP3)) == -1){
        generalInfoLog("Bind failed for stabilize socket \n");
        exit(1);
    }
    if(bind(clientSocket, (struct sockaddr*) &myInfoUDP2, sizeof(myInfoUDP2)) == -1){
        generalInfoLog("Bind failed for client socket\n");
        exit(1);
    }
    /*
       for(int i = 0; i<20; ++i){
       printf("%d * ",selfID[i]);
       }
       printf("\n");
       */

    // Create an object of the chord class
    myChordInstance = new Chord(selfID, selfIP, NUM_SUCCESSOR, clientSocket, serverSocket, stabilizeSocket);


    // Check if this is the first node or not
    if(!isFirstNode){
        // This node is not the first node to join. SO call the join method with the
        // bootstrap node
        myChordInstance->join(bootstrapNode);
    }
    else{
        // This is the first node to join/ Call the create method
        myChordInstance->create();
    }
    
    
    fd_set read_fds, master; // set of read fds.

    FD_ZERO(&read_fds); // clear the read fd set
    FD_ZERO(&master); // clear the read fd set

    // Add the UDP socket to the master list too
    FD_SET(serverSocket, &read_fds);
    FD_SET(clientSocket, &read_fds);
    FD_SET(stabilizeSocket, &read_fds);

    int fdmax = (serverSocket > clientSocket ? serverSocket : clientSocket);

    fdmax = (stabilizeSocket > fdmax ? stabilizeSocket : fdmax);

    master = read_fds;
    
   printf("Max Socket: %d\n", fdmax);

    // start the stabilize timer
    gettimeofday(&stabilizeTimer, NULL);


    int selectReturnValue;
    for(;;){

        
        read_fds = master;

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

            myChordInstance->stabilize();

            if(debug){
                printf("After stabilize:\n");

                printf("Self ID: %s  Self IP: %s\n",myChordInstance->getLocalID().c_str(), myChordInstance->getLocalIP().c_str());
                if(!myChordInstance->getPredecessor()){
                    printf("No predecessor yet\n");
                }
                else{
                    printf("Predecessor ID: %s and Predecessor IP: %s\n", myChordInstance->getPredecessor()->getNodeID().c_str(),
                                                                          myChordInstance->getPredecessor()->getNodeIP().c_str());
                }

                if(!myChordInstance->getFirstSuccessor()){
                    printf("No successor yet\n");
                }
                else{
                    printf("Successor ID: %s and Successor IP: %s\n", myChordInstance->getFirstSuccessor()->getNodeID().c_str(),
                                                                    myChordInstance->getFirstSuccessor()->getNodeIP().c_str());
                }

                if(myChordInstance->getFirstSuccessor() && myChordInstance->getPredecessor()){

                    if(myChordInstance->getLocalID().compare(myChordInstance->getPredecessor()->getNodeID()) != 0 &&
                       myChordInstance->getPredecessor()->getNodeID().compare(myChordInstance->getFirstSuccessor()->getNodeID()) != 0 &&
                       myChordInstance->getLocalID().compare(myChordInstance->getFirstSuccessor()->getNodeID()) != 0){

                        printf("System is stabilized\n");
                        printf("---------------------------\n");
                    }
                }

                fflush(NULL);
            }
            // Reset the timer
            gettimeofday(&stabilizeTimer, NULL);
        }
        else{
            cout << "Received MEssage\n";
            // Run through the existing connections for the data to be read
            for(int i = 0; i<=fdmax; i++){
                if(FD_ISSET(i, &read_fds)){ // We got one active fd

                    if(i == serverSocket){

                        cout << "Received a request from server\n";

                        // Received a message from server
                        char* maxMessage = new char[MAX_MSG_SIZE];
                        struct sockaddr_in senderProcAddrUDP;

                        // To store the address of the process from whom a message is received
                        memset((char*)&senderProcAddrUDP, 0, sizeof(senderProcAddrUDP));
                        socklen_t senderLenUDP = sizeof(senderProcAddrUDP);

                        int recvRet = 0;

                        recvRet = recvfrom(serverSocket, maxMessage, MAX_MSG_SIZE,
                                0, (struct sockaddr*) &senderProcAddrUDP, &senderLenUDP);

                        if(recvRet > 0){
                            // Get the type of the message
                            uint32_t* msgType = (uint32_t*)(maxMessage);
                            uint32_t type = *msgType;

                            if(type == SERVER_REQ){		
                                myChordInstance->handleRequestFromServer(maxMessage, recvRet);
                            }
                            else if(type == CLIENT_REQ){
                                myChordInstance->handleRequestFromClient(maxMessage, recvRet);
                            }
                            else{
                                cout << "SERVICE: Invalid message received: " << type << endl;
                            }
                        }

                        delete[] maxMessage;
                    }
                    else if(i == stabilizeSocket){

                        cout << "Received a stabilize message from server\n";

                        // Received the stabilize related message from server
                        char* maxMessage = new char[MAX_MSG_SIZE];
                        struct sockaddr_in senderProcAddrUDP;

                        // To store the address of the process from whom a message is received
                        memset((char*)&senderProcAddrUDP, 0, sizeof(senderProcAddrUDP));
                        socklen_t senderLenUDP = sizeof(senderProcAddrUDP);

                        int recvRet = 0;

                        recvRet = recvfrom(stabilizeSocket, maxMessage, MAX_MSG_SIZE,
                                0, (struct sockaddr*) &senderProcAddrUDP, &senderLenUDP);

                        if(recvRet > 0){
                            // Get the type of the message
                            uint32_t* msgType = (uint32_t*)(maxMessage);
                            uint32_t type = *msgType;

                            if(type == SERVER_REQ){		
                                myChordInstance->handleRequestFromServer(maxMessage, recvRet);
                            }
                            /*else if(type == CLIENT_REQ){
                                myChordInstance->handleRequestFromClient(maxMessage, recvRet);
                            }*/
                            else if(type == SERVER_RES){
                                myChordInstance->handleStabilizeResponse(maxMessage);              
                            }
                            else{
                                cout << "SERVICE: Invalid message received: " << type << endl;
                            }
                        }

                        delete[] maxMessage;

                    }
                    else if(i == clientSocket){
                        
                        cout << "Received a client request\n";

                        // Received a client request
                        char* maxMessage = new char[MAX_MSG_SIZE];
                        struct sockaddr_in senderProcAddrUDP;

                        // To store the address of the process from whom a message is received
                        memset((char*)&senderProcAddrUDP, 0, sizeof(senderProcAddrUDP));
                        socklen_t senderLenUDP = sizeof(senderProcAddrUDP);
                        
                        int recvRet = 0;

                        recvRet = recvfrom(clientSocket, maxMessage, MAX_MSG_SIZE,
                                0, (struct sockaddr*) &senderProcAddrUDP, &senderLenUDP);

                        if(recvRet > 0){
                            // Get the type of the message
                            uint32_t* msgType = (uint32_t*)(maxMessage);
                            uint32_t type = *msgType;

                            if(type == CLIENT_REQ){		
                                myChordInstance->handleRequestFromClient(maxMessage, recvRet);
                            }
                            else{
                                cout << "SERVICE: Invalid message received: " << type << endl;
                            }
                        }

                        delete[] maxMessage;
                    }
                }
            }// ENd of fd set for loop
        }
        

    }// ENd of infinite loop

    return 0;
}
