#include "Node.h"
#include "Defs.h"

Node::Node(){
    entryList.clear();
}

Node::~Node(){
    entryList.clear();    
}

const string Node::getNodeID(){

    return localID;
}

const string Node::getNodeIP(){

    return localIP;
}

void Node::setNodeID(string id){

    this->localID = id;
}

void Node::setNodeIP(string ip){

    this->localIP = ip;
}

bool Node::checkIfEntryExists(string fileKey, string fileName){

    bool entryFound = false;

    if(this->entryList.find(fileKey) != this->entryList.end()){
        // Found the entry with the given key
        cout << "Entry with the key " << fileKey << " and name " << fileName << " exists" << endl;
        entryFound = true;
    }

    return entryFound;
}

string Node::getEntryValue(string fileKey, string fileName){

    string retValue;

    if(checkIfEntryExists(fileKey, fileName)){
        retValue = this->entryList[fileKey].getFileValue();
    }
    else{
        // Entry doesn't exists on this node
        cout << "Entry with the key " << fileKey << "and name " << fileName << " not found" << endl;
        retValue = FILE_NOT_FOUND;
    }

    return retValue;
}

void Node::storeEntry(string fileKey, string fileName, string fileValue){

    Entry newEntry(fileName, fileValue); 
    // Check if an entry exists with this key
    if(checkIfEntryExists(fileKey, fileName)){
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


void Node::removeEntry(string fileKey, string fileName)
{
    if(checkIfEntryExists(fileKey, fileName)){

        cout << "Removing the entry for file name " << fileName << endl;
        entryList.erase(fileKey);
    }
}

