#ifndef _SUCCESSORLIST_H
#define _SUCCESSORLIST_H

#include "Node.h"
#include <list>

class SuccessorList{

    private:
        int maxNumSuccessors;
        list<Node*> successors;
        list<Node*>::iterator myIterator;
        string localID;
    
    public:
        SuccessorList();
        SuccessorList(int capacity, string myLocalKey);
        ~SuccessorList();

        void setMaxNumSuccssor(int maxSuccCount);
        void setLocalID(string id);
        void storeSuccessor(Node* newSuccessor);
        bool checkIfSuccessorExists(Node* successor);
        void removeSuccessor(string ipToFind);

        string getLocalID();
        int getCurrentSuccessorCount();
        int getMaxSuccessorCount();

	    //Added to pop the
	    Node* getFirstSuccessor();
        void storeFirstSuccessor(Node* firstSucc);
};

#endif
