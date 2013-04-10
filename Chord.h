#include "Node.h"
#include<map>
/*
 *
 * This class will handle the core functionality of the Chord Protocol. 
 *
 * */

class Chord
{
		private:
			Node* localNode; //stores the local ip, local id and entry list of a node

		public:
			Chord();
			Chord(string localID,string localIP);
			~Chord();
			
			void create(); //creates a new chord network which is not connected to any other node
			
			void join(string IP); //joins an existing node with ip address "IP"
			
			void stabilize(); //runs the stabilize operations of this node
			
			void findSuccessor(string id); //finds the successor of a given node with id "id"
			
			void leave(); //runs the leave operations for this node, updates information of adjacent nodes
			
			void notify(string predecessor); //This method is invoked by another node which thinks it is this node's predecessor

			void ping(); // Requests a sign of live. This method is invoked by another node which thinks it is this node's successor.

			/*
			 * Stores a finger table in the form:
			 * 0 ==> <id,ip>
			 * 1 ==> <id,ip>
			 *
			 *
			*/
			map<int,pair<string,string> > fingerTable; 
		
			void buildFingerTable(); //Builds the finger table

			void insert(string id, string fileContent); //Inserts a new data object into the network with the Key 'id' and Value 'fileContent'

			void remove(string id, string fileContent); //Removes a file with the Key 'id' and value 'fileContent'

			void disconnect(); //Disconnects this node

			void retrieve(string id); //Retrieves the file with the Key 'id'

			void insertReplicas(string id); //Insert replica of a file with Key 'id'


};
