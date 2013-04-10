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
			
	if(this->localNode != NULL)
	{
		string msg = "Node already exists\n";
		myError.setErrorMessage(msg);
		//throw myError(msg);
		throw myError;
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


