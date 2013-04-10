#include "Node.h"
#include "Error.h"

Node::Node(){}

Node::~Node(){}

const string Node::getNodeID(){

    return this->localID;
}

const string Node::getNodeIP(){

    return this->localIP;
}

void Node::setNodeID(string id){

    this->localID = id;
}

void Node::setNodeIP(string ip){

    this->localIP = ip;
}

bool Node::checkIfEntryExists(string fileKey){

    bool entryFound = false;

    if(this->entryList.find(fileKey) != this->entryList.end()){
        // Found the entry with the given key
        cout << "Entry with the key " << fileKey << " exists" << endl;
        entryFound = true;
    }

    return entryFound;
}

string getEntryValue(string fileKey){

    string value;

    if(checkIfEntryExists(fileKey)){
        value = this->entryList[fileKey];
    }
    else{
        // Entry doesn't exists on this node
        cout << "Entry with the key " << fileKey << " not found" << endl;
        value = FILE_NOT_FOUND;
    }

    return value;
}
