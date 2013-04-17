#include "Successorlist.h"


SuccessorList::SuccessorList(){
    maxNumSuccessors = 0;
    this->successors.clear();
}

SuccessorList::SuccessorList(int capacity, string myLocalKey){

    SuccessorList();
    maxNumSuccessors = capacity;
    localID = myLocalKey;
}

SuccessorList::~SuccessorList(){

    if(this->successors.size() != 0){
        for(myIterator = successors.begin();
            myIterator != successors.end();
            ++myIterator)
        {
            if(*myIterator){
                delete(*myIterator);
            }
        }
    }

    successors.clear();
}

void SuccessorList::setMaxNumSuccssor(int maxSuccCount){
    maxNumSuccessors = maxSuccCount;
}

void SuccessorList::setLocalID(string id){
    localID = id;
}

bool SuccessorList::checkIfSuccessorExists(Node* successor){

    bool isNodePresent = false;

    // Check if a node with the same IP exists in the
    // successor list. If its already there then just
    // update the ID of the node
    for(myIterator = successors.begin();
        myIterator != successors.end();
        ++myIterator){

        if((*myIterator)->getNodeIP() == successor->getNodeIP()){
            cout << "Node " << successor->getNodeIP() << " found in the successor list" << endl;
            cout << "Updating the localID of the successor" <<endl;
            (*myIterator)->setNodeID(successor->getNodeID());

            isNodePresent = true;
            break;
        }
    }

    return isNodePresent;
}

Node* SuccessorList::getFirstSuccessor(){
    // Returns the first successor
    // First checks the count of the successor present in the list
    if(successors.size() > 0){

        return successors.front();
    }
    else{
        cout << "List of successor is empty\n";
    }


}

void SuccessorList::storeFirstSuccessor(Node* firstSucc){

    if(successors.size() > 0){
        Node* currSucc = successors.front();
        successors.pop_front();

        delete currSucc;

        // store the new succ
        successors.push_front(firstSucc);

    }
    else if(successors.size() == 0){
        successors.push_front(firstSucc);

    }

}



void SuccessorList::storeSuccessor(Node* newSuccessor){

    // Right now just store the successor node in the list
    // Later we can check if we need to comparethe index before insert
    if(checkIfSuccessorExists(newSuccessor)){
        cout << "Successor is already present in the list\n";
    }
    else if(successors.size() < maxNumSuccessors){
        successors.push_back(newSuccessor);
    }
    else{
        cout << " Successor list is already full\n";
    }
}

string SuccessorList::getLocalID(){

    return localID;
}

int SuccessorList::getCurrentSuccessorCount(){
    
    return successors.size();
}

int SuccessorList::getMaxSuccessorCount(){
    return maxNumSuccessors;
}

void SuccessorList::removeSuccessor(string ipToFind){

    for(myIterator = successors.begin();
        myIterator != successors.end();
        ++myIterator)
    {

        if((*myIterator) && 
            (*myIterator)->getNodeIP() == ipToFind){

            cout << "Successor found in the list. So now remove\n";
            delete(*myIterator);
        }
    }
}
