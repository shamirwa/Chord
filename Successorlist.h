#ifndef _SUCCESSORLIST_H
#define _SUCCESSORLIST_H

#include "Node.h"
#include <list>

class SuccessorList{

    private:
        int maxNumSuccessors;
        int currentCount;
        List<Node> successorList;
        List<Node>::iterator myIterator;
        string localID;
    
    public:
        SuccessorList();
        SuccessorList(int capacity, string myLocalKey);
        ~SuccessorList();

        void setMaxNumSuccssor(int maxSuccCount);
        void setLocalID(string id);
        void storeSuccessor(Node newSuccessor);
        bool checkIfSuccessorExists(Node successor);
        void removeSuccessor(Node successor);

        string getLocalID();
        int getCurrentSuccessorCount();
        int getMaxSuccessorCount();
};

#endif
