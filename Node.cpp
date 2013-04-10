#include "Node.h"

Node::Node(){}

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
