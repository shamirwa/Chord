#include <iostream>
#include "Entry.h"
#include <vector>

using namespace std;


class Node{
    private:
        string localIP; // Self IP Address of the node
        string localID; // Self hash code of the node
        vector<Entry> entryList; // List of entries

    public:
        Node();
        ~Node();
        const string getNodeID();
        const string getNodeIP();
        void setNodeID(string id);
        void setNodeIP(string ip);
};

