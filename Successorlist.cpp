#include "Successorlist.h"


SuccessorList::SuccessorList(){
    maxNumSuccessors = 0;
    this->successorList.clear();
}

SuccessorList::SuccessorList(int capacity, string myLocalKey){

    SuccessorList();
    maxNumSuccessors = capacity;
    localID = myLocalKey;
}

SuccessorList::~SuccessorList(){

    if(this->successorList.size() != 0){
        for(myIterator = successorList.begin();
            myIterator != successorList.end();
            ++myIterator)
        {
            if(*myIterator){
                delete(*myIterator);
            }
        }
    }

    successorList.clear();
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
    for(myIterator = successorList.begin();
        myIterator != successorList.end();
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
    if(successorList.size() > 0){

        return successorList.front();
    }
    else{
        return NULL;
        cout << "List of successor is empty\n";
    }


}

void SuccessorList::storeFirstSuccessor(Node* firstSucc){

    if(successorList.size() > 0){
        Node* currSucc = successorList.front();
        successorList.pop_front();

        delete currSucc;

        // store the new succ
        successorList.push_front(firstSucc);

    }
    else if(successorList.size() == 0){
        successorList.push_front(firstSucc);

    }

}



void SuccessorList::storeSuccessor(Node* newSuccessor){

    // Right now just store the successor node in the list
    // Later we can check if we need to comparethe index before insert
    if(checkIfSuccessorExists(newSuccessor)){
        cout << "Successor is already present in the list\n";
    }
    else if(successorList.size() < maxNumSuccessors){

        //Find the correct position of the newSuccessor
        list<Node*>::iterator myIter;
        myIter = successorList.begin();

        while(myIter != successorList.end())
        {
            if(isInInterval(newSuccessor->getNodeID(),getLocalID(),(*myIter)->getNodeID()))
            {
                   cout<< "Found position for the new successor\n"; 
                   successorList.insert(myIter,newSuccessor);
                   break;
            }
            ++myIter;
        }
        if(myIter == successorList.end())
            successorList.push_back(newSuccessor);
    }
    else{
        cout << " Successor list is already full\n";
    }
}

string SuccessorList::getLocalID(){

    return localID;
}

int SuccessorList::getCurrentSuccessorCount(){
    
    return successorList.size();
}

int SuccessorList::getMaxSuccessorCount(){
    return maxNumSuccessors;
}

void SuccessorList::removeSuccessor(string ipToFind){
    //Node* nodeToRemove = NULL;
    functionEntryLog("IN removeSuccessor");
    /*
    for(myIterator = successorList.begin();
        myIterator != successorList.end();
        ++myIterator)
    {

        if((*myIterator) && 
            (*myIterator)->getNodeIP().compare(ipToFind)== 0){

            cout << "Successor found in the list. So now remove\n";
            delete nodeToRemove;
            successorList.erase(myIterator);
        }
    }*/

    int count = 0;
    string ipInList;
    cout << "Count in success remove method " << successorList.size() << endl;

    list<Node*>::iterator myIter = successorList.begin();
    /*for(myIter = successorList.begin();
        myIter != successorList.end();
        ++myIter)
    {


        if((*myIter) && 
                ipInList.compare(ipToFind)== 0){

            cout << "Successor found in the list. So now remove\n";
            delete nodeToRemove;
            //successorList.erase(myIter);
            break;
        }
    }*/

    while(myIter != successorList.end()){

        ipInList.assign((*myIter)->getNodeIP());
        cout << "Count: " << count++ << "IP: " << ipInList << endl;
        
        if((*myIter) && ipInList.compare(ipToFind)== 0){

            myIter = successorList.erase(myIter);
        }
        else{
            ++myIter;
        }
    }

    cout << "After erasing in remove method" << endl;
}
