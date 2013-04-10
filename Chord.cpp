#include "Chord.h"

Chord::Chord(){
	
}

Chord::Chord(string localID,string localIP) {

	this->localNode.setNodeID(localID);
	this->localNode.setNodeIP(localIP);

	
}

Chord::~Chord() {}

int main(){

	return 0;
	}


