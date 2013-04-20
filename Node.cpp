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

bool Node::checkIfEntryExist(string fileKey, string fileName){

    bool entryFound = false;

    if(this->entryList.find(fileKey) != this->entryList.end()){
        // Found the entry with the given key
        cout << "Entry with the key " << fileKey << " and name " << fileName << " exists" << endl;
        entryFound = true;
    }

    return entryFound;
}

string Node::getEntryValue(string fileKey, string fileName){

    string value;

    if(checkIfEntryExists(fileKey)){
        value = this->entryList[fileKey].getFileValue();
    }
    else{
        // Entry doesn't exists on this node
        cout << "Entry with the key " << fileKey << "and name " << fileName << " not found" << endl;
        value = FILE_NOT_FOUND;
    }

    return value;
}

void Node::storeEntry(string fileKey, string fileName, string fileValue){

    Entry newEntry(fileName, fileValue); 
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






