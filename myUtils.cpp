#include "myUtils.h"
#include "Exception.h"
#include <openssl/evp.h>

using namespace std;

void throwException(string msg)
{
    myError.setErrorMessage(msg);
    throw myError;
}


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

int compareTo(string id1, string toCompareID)
{
    if(id1.length() != toCompareID.length()){
        // Throw exception
    }

    for(int i = 0; i < id1.length(); ++i){

        if((char)(id1[i] - 128) < (char)(toCompareID[i] - 128)){
            return -1;
        }

        else if((char)(id1[i] - 128) > (char)(toCompareID[i] - 128)){
            return 1;
        }
    }

    return 0;
}


bool isIdEqual(string id1, string id2)
{

    return (compareTo(id1, id2) == 0);
}

bool isInInterval(string ID, string fromID, string toID)
{
    // Check if both interval bounds are equal or not
    if(isIdEqual(fromID, toID)){

        // Every ID is contained in the interval except of two bounds
        return (!isIdEqual(ID, fromID));
    }

    // If intervals doesn't cross zero then compare from
    // both the from and to ID's
    if(compareTo(fromID ,toID) < 0){
        return (compareTo(ID, fromID) > 0 && compareTo(ID, toID) < 0);
    }

    else{

        return (compareTo(ID, fromID) > 0 || compareTo(ID, toID) < 0);
    }
    
}


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

