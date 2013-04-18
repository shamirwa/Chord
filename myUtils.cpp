#include "myUtils.h"
#include "Exception.h"
#include <openssl/evp.h>
#include "Defs.h"
#include "message.h"

using namespace std;

void throwException(string msg)
{
    myError.setErrorMessage(msg);
    throw myError;
}


void functionEntryLog(const char msg[]){
    if(functionDebug){
        printf("Entered the function %s\n", msg);
        fflush(NULL);
    }
}

void generalInfoLog(const char msg[]){
    if(infoDebug){
        printf("%s\n", msg);
        fflush(NULL);
    }
}

int compareTo(string id1, string toCompareID)
{
    functionEntryLog("comapreTo");

    if(id1.length() != toCompareID.length()){
        printf("Diff length ID\n");
        printf("Length id1: %ld\n", id1.length());
        printf("Length toCompareID: %ld\n", toCompareID.length());
        // Throw exception
    }

    int minLen = min(id1.length(), toCompareID.length());

    for(int i = 0; i < minLen; ++i){

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

    functionEntryLog("isInInterval");

    bool result = false;
    int cond = 0;

    // Check if both interval bounds are equal or not
    if(isIdEqual(fromID, toID)){

        // Every ID is contained in the interval except of two bounds
        result = (!isIdEqual(ID, fromID));
        cond = 1;
        if(result){
            printf("Return true in cond %d isInInterval\n", cond);
        }
        else{
            printf("Return false in cond %d isInInterval\n", cond);
        }


        return result;

    }

    // If intervals doesn't cross zero then compare from
    // both the from and to ID's
    if(compareTo(fromID ,toID) < 0){

        result = (compareTo(ID, fromID) > 0 && compareTo(ID, toID) < 0);
        cond = 2;

    }

    else{

        result = (compareTo(ID, fromID) > 0 || compareTo(ID, toID) < 0);
        cond = 3;
    }

    if(result){
        printf("Return true in cond %d isInInterval\n", cond);
    }
    else{
        printf("Return false in cond %d isInInterval\n", cond);
    }

    return result;
    
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

Node* buildSuccessorNode(string IP, string ID){

    Node* successorNode = new Node;
    successorNode->setNodeID(ID);
    successorNode->setNodeIP(IP);

    return successorNode;
}


string addPowerOfTwo(int powerOfTwo,string ID)
{

		functionEntryLog("addPowerOfTwo");
		
		if(powerOfTwo < 0 || powerOfTwo >= (FINGER_TABLE_SIZE)) {
		
			generalInfoLog("The power of two is out of range! It must be in the interval");
				
		}

		int indexOfByte = ID.length() - 1 - (powerOfTwo / 8);

		char toAdd[] = { 1, 2, 4, 8, 16, 32, 64, -128 };
	
		char valueToAdd = toAdd[powerOfTwo % 8];

		char oldValue;

		do
		{
				oldValue = ID[indexOfByte];
				ID[indexOfByte] += valueToAdd;
				valueToAdd = 1;
				

		}while(oldValue < 0 && ID[indexOfByte] >= 0 && indexOfByte-- > 0);
		
		return ID;

}

void printMessageDetails(char* msg)
{
    functionEntryLog("printMessageDetails");

    command* cmndMsg = (command*)msg;
    
    //printf("Total Message: %s\n", msg);
    printf("Type: %d\n", cmndMsg->type);
    printf("Sender ID: %s\n", cmndMsg->senderID);
    printf("Num params: %d\n", cmndMsg->numParameters);

}
