#include "Chord.h"
#include "Exception.h"


/*
 *	Method to throw an exception with the message 'msg'
 *	
 * */
void throwException(string msg)
{
	myError.setErrorMessage(msg);
	throw myError;
}



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
		throwException(ERR_ALREADY_EXISTS);
	}

	//has NodeIP been set?
	if(this->localNode->getNodeIP().length() == 0)
	{	
		throwException(ERR_NODE_NOT_SET);
	}

	/*
	 *
	 * TO DO
	*/
	//if necessary generate NodeId out of IP
	if(this->localNode->getNodeID().length() == 0)
	{
				
	}

}

void Chord::join(string IP){

	//Check if IP is valid
	if(IP.length() == 0)
	{
		throwException(ERR_IP_INVALID);
	}

	if(this->localNode != NULL)
	{
		throwException(ERR_ALREADY_EXISTS);
	}

}

void Chord::leave(){


}

void Chord::ping(){


}

void Chord::buildFingerTable(){


}

void Chord::insert(string id,string fileContent){


}


