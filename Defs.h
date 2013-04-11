#ifndef _DEFS_H_
#define _DEFS_H_

#include <string>

#define ERR_ALREADY_EXISTS "Cannot create network; node is already connected!\n"

#define NUMBER_OF_SUCCESSORS 5

#define ERR_NODE_IP_NOT_SET "Node URL is not set yet!"

#define ERR_NODE_ID_NOT_SET "Node ID is not set yet!"

#define ERR_IP_INVALID "IP address is invalid!"

#define FILE_NOT_FOUND "Error: File not found"

#define WRONG_NUM_SUCCESSORS "NUMBER_OF_SUCCESSORS intialized with wrong value!"

#define ERR_NODE_NULL "Error: Node is null"

#define FILE_NOT_FOUND "Error: File not found"

#define FILE_ALREADY_STORED "Error: File is already stored in the node"

#define NUM_SUCCESSOR 1 // Number of successors to store in the list

/* Function to get the hash code for the node IP
 * Input: nodeIP - IP address of the node for which hash key needs
 * to be computed
 * Returns: string - hashKey generated for the passed IP address
 *
 * Description: This hashKey is the unique identifier of a node
 * in the chord network
*/
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

#endif
