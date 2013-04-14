

#include "Chord.h"
#include "Exception.h"
#include "Defs.h"
#include <utility>
#include <stdio.h>

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

Chord::Chord(string localID,string localIP,int numSuccessor,int clientSocket,int serverSocket) {

	this->localNode = NULL;
	this->localNode = new Node;
	this->localNode->setNodeID(localID);
	this->localNode->setNodeIP(localIP);
	this->successors.setLocalID(localID);
	this->successors.setMaxNumSuccssor(numSuccessor);
	this->clientSocket = clientSocket;
	this->serverSocket = serverSocket;
	
}

Chord::~Chord() {}

void Chord::createHelp()
{
	
	//The map and finger table are already instantiated so no work here

}



//searches the local table for the highest predecessor of id
string Chord::closestPrecedingNode(string id)
{
	
	string localNodeID = this->localNode->getNodeID();

	//Need a reverse iterator
  std::map<int,pair<string,string> >::reverse_iterator finger_table_iterator;

	for(finger_table_iterator=this->fingerTable.rbegin(); finger_table_iterator != this->fingerTable.rend(); ++finger_table_iterator)
	{
				//check fingerTable(i) if between localID,id
				//
				// pair has id,ip
				pair<string,string> curr_id_ip = finger_table_iterator->second;
			  string curr_finger_id = curr_id_ip.first;
				string curr_finger_ip = curr_id_ip.second;

				fprintf(stderr,"%s\n",curr_id_ip.first.c_str());

			if(strcmp(localNodeID.c_str(),id.c_str()) < 0 && strcmp(curr_finger_id.c_str(),id.c_str()) > 0)
			{
				return curr_finger_ip; 	
			}
			else if(strcmp(localNodeID.c_str(),id.c_str()) > 0 && strcmp(curr_finger_id.c_str(),id.c_str()) < 0)
			{
				return curr_finger_ip;	
			}				
	}
	
	return this->localNode->getNodeIP();
	
}


string Chord::findSuccessor(string IP)
{
		
		//TO DO:
		//Get id from IP
		//
		string id; // = ;

		//TO DO:
		//Check for cross over with zero
		//
		
		string localNodeID = this->localNode->getNodeID();

		string successorID = this->successors.getFirstSuccessor()->getNodeID();
	
	
		//Lies between the current node and successor node 
		//
		if(strcmp(localNodeID.c_str(),id.c_str()) < 0 && strcmp(successorID.c_str(),id.c_str()) > 0)
		{
			return this->successors.getFirstSuccessor()->getNodeIP(); 	
		}
		else if(strcmp(localNodeID.c_str(),id.c_str()) > 0 && strcmp(successorID.c_str(),id.c_str()) < 0)
		{
			return this->localNode->getNodeIP();	
		}
		
		else
		{
			string closestPrecedingNodeIP = this->closestPrecedingNode(id);
			/* TO DO:
			 * Call the successor of the closestPrecedingNodeIP
			 *
			 *return closestPrecedingNode->findSuccessor(id);
			 */
		}
		
}


void Chord::create(){
		
	//is Node already connected	
	if(this->localNode == NULL)
	{
		throwException(ERR_ALREADY_EXISTS);
	}

	//has NodeIP been set?
	if(this->localNode->getNodeIP().length() == 0)
	{	
		throwException(ERR_NODE_IP_NOT_SET);
	}

	/*
	 *
	 * TO DO
	*/
	//if necessary generate NodeId out of IP
	if(this->localNode->getNodeID().length() == 0)
	{
		throwException(ERR_NODE_ID_NOT_SET);		
	}
	
	this->createHelp();
}

void Chord::joinHelp(string IP)
{
	
		//create local repository for successors
		if (NUMBER_OF_SUCCESSORS >=1)
		{
			
		}
		else
		{
			throwException(WRONG_NUM_SUCCESSORS);
		}

		//check if local node exists
		if(this->localNode == NULL)
			throwException(ERR_NODE_NULL);
	
		this->predecessor = NULL;

		//Get successor
		string IP ; //==find_successor(IP)

		buildFingerTable(IP);

		


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
	
	this->joinHelp(IP);

		
		



}

void Chord::leave(){


}

void Chord::ping(){


}

void Chord::buildFingerTable(){


}

void Chord::insert(string id,string fileContent){


}

