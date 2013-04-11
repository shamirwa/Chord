#include "Successorlist.h"


SuccessorList::SuccessorList(){
    maxNumSuccessors = 0;
    currentCount = 0;
}

SuccessorList::SuccessorList(int capacity, string myLocalKey){

    SuccessorList();
    maxNumSuccessors = capacity;
    localID = myLocalKey;
}

SuccessorList::~SuccessorList(){

}

void SuccessorList::setMaxNumSuccssor(int maxSuccCount){
    maxNumSuccessors = maxSuccCount;
}

void SuccessorList::setLocalID(string id){
    localID = id;
}

bool SuccessorList::checkIfSuccessorExists(Node successor){

    bool isNodePresent = false;

    // Check if a node with the same IP exists in the
    // successor list. If its already there then just
    // update the ID of the node
    for(this->myIterator = this->successorList.begin();
        this->myIterator != this->successorList.end();
        ++(this->myIterator)){

        if(this->myIterator->localIP == successor.localIP){
            cout << "Node " << successor.localIP << " found in the successor list" << endl;
            cout << "Updating the localID of the successor" <<endl;
            this->myIterator->localID = successor.localID;

            isNodePresent = true;
            break;
        }
    }

    if(!isNodePresent){
        successorList
}



void SuccessorList::storeSuccessor(Node newSuccessor){



}
