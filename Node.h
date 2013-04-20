#ifndef _NODE_H
#define _NODE_H
#include <iostream>
#include "Entry.h"
#include <map>
#include <string>

using namespace std;


class Node{
    private:
        string localIP; // Self IP Address of the node
        string localID; // Self hash code of the node
        map<string, Entry> entryList; // map of entries keyed with the hashcode
        map<string, Entry>::iterator myIter;

    public:
        Node();
        ~Node();
        const string getNodeID();
        const string getNodeIP();
        void setNodeID(string id);
        void setNodeIP(string ip);
        bool checkIfEntryExists(string fileKey);
        string getEntryValue(string fileKey);
        void storeEntry(string fileKey, string fileName, string fileValue);
        map<string,Entry> getAllEntries();

};


#endif
