#include "Service.h"
#include "Chord.h"
#include "Defs.h"

void functionEntryLog(const char msg[]){   
    if(functionDebug){ 
        printf("Entered the function %s\n", msg);
    }
}

void generalInfoLog(const char msg[]){ 
    if(infoDebug){
        printf("%s\n", msg);
    }
}


/* Function to get the hash code for the node IP                                     
 * * Input: nodeIP - IP address of the node for which hash key needs                    
 * * to be computed
 * * Returns: string - hashKey generated for the passed IP address                      
 * *       
 * * Description: This hashKey is the unique identifier of a node                       
 * * in the chord network                                                               
 * */
string getLocalHashID(string nodeIP){
    functionEntryLog("getLocalHashID");

    if(infoDebug){
        printf("Computing the local hash id for node %s\n", nodeIP.c_str());
    }

    string digest;
    EVP_MD_CTX mdctx;
    const EVP_MD *md;
    unsigned char md_value[EVP_MAX_MD_SIZE];
    unsigned int md_len;
    int i;

    OpenSSL_add_all_digests();

    md = EVP_get_digestbyname("SHA1");

    if(!md){
        generalInfoLog("Unknown message digest");
    }

    EVP_MD_CTX_init(&mdctx);
    EVP_DigestInit_ex(&mdctx, md, NULL);
    EVP_DigestUpdate(&mdctx, nodeIP.c_str(), nodeIP.length());
    EVP_DigestFinal_ex(&mdctx, md_value, &md_len);
    EVP_MD_CTX_cleanup(&mdctx);

    digest = reinterpret_cast<const char*>(md_value);

    return digest;
}


int main(int argc, char* argv[]){

    bool isFirstNode = true;
    string bootstrapNode;
    string selfName;
    string selfIP;
    string selfID;
    struct in_addr **ipAddr;
    struct hostent* he;

    Chord myVar;

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
    
    /*
    for(int i = 0; i<20; ++i){
        printf("%d * ",selfID[i]);
    }
    printf("\n");
    */

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
