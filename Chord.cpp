#include "Chord.h"
#include "Exception.h"

Chord::Chord(){
	
	this->localNode = NULL;	
}

Chord::Chord(string localID,string localIP) {

	this->localNode = NULL;
	this->localNode = new Node;
	this->localNode->setNodeID(localID);
	this->localNode->setNodeIP(localIP);
	
}

Chord::~Chord() {}

void Chord::create(){
		
	//is Node already connected	
	if(this->localNode != NULL)
	{
		string msg = ERR_ALREADY_EXISTS;
		myError.setErrorMessage(msg);
		//throw myError(msg);
		throw myError;
	}

	//has NodeIP been set?
	if(this->localNode->getNodeIP().length() == 0)
	{	
		string msg = ERR_NODE_NOT_SET;
		myError.setErrorMessage(msg);
		throw myError;
	}

	/*
	 *
	 * TO Do
	*/
	//if necessary generate NodeId out of IP
	if(this->localNode->getNodeID().length() == 0)
	{
				
	}

}

void Chord::join(string IP){

		

}

void Chord::leave(){


}

void Chord::ping(){


}

void Chord::buildFingerTable(){


}

void Chord::insert(string id,string fileContent){


}


int main(){
	
	return 0;
}


