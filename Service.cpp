#include "Service.h"
#include "Chord.h"
#include "Defs.h"

void functionEntryLog(char msg[]){
    if(functionDebug){
        printf("Entered the function %s\n", msg);
    }
}

void generalInfoLog(char msg[]){
    if(infoDebug){
        printf("%s\n", msg);
    }
}

int main(int argc, char* argv[]){

    bool isFirstNode = false;
    string bootstrapNode;
    string selfName;
    string selfIP;
    string selfID;
    struct in_addr **ipAddr;
    struct hostent* he;

    // Parse the command line arguments
    if(argc == 3){
        if(strcmp(argv[1], "--bsNode") == 0){
            bootstrapNode = argv[2];
        }
        isFirstNode = true;
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

    // Create an object of the chord class
    Chord myChordInstance(selfID, selfIP, NUM_SUCCESSOR);

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



    return 0;
}
