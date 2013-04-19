#include "Node.h"
#include "Defs.h"

Node::Node(){
    entryList.clear();
}

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

string Node::getEntryValue(string fileKey){

    string value;

    if(checkIfEntryExists(fileKey)){
        value = this->entryList[fileKey].getFileValue();
    }
    else{
        // Entry doesn't exists on this node
        cout << "Entry with the key " << fileKey << " not found" << endl;
        value = FILE_NOT_FOUND;
    }

    return value;
}

void Node::storeEntry(string fileKey, string fileValue){

    Entry newEntry(fileKey, fileValue); 
    // Check if an entry exists with this key
    if(checkIfEntryExists(fileKey)){
        cout << FILE_ALREADY_STORED << endl;
        return;
    }
    else{
        this->entryList[fileKey] = newEntry;
    }

}

map<string,Entry> Node::getAllEntries()
{
    return this->entryList;
}






