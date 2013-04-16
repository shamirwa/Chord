#include "utility.h"
#include "Exception.h"
#include "Defs.h"

using namespace std;


int compareTo(string id1, string toCompareID)
{
    if(id1.length() != toCompareID.length()){
        // Throw exception
    }

    for(int i = 0; i < id1.length(); ++i){

        if((char)(id1[i] - 128) < (char)(toCompareID[i] - 128)){
            return -1;
        }

        else if((char)(id1[i] - 128) > (char)(toCompareID[i] - 128)){
            return 1;
        }
    }

    return 0;
}


bool isIdEqual(string id1, string id2)
{

    return (compareTo(id1, id2) == 0);
}

bool isInInterval(string ID, string fromID, string toID)
{
    // Check if both interval bounds are equal or not
    if(isIdEqual(fromID, toID)){

        // Every ID is contained in the interval except of two bounds
        return (!isIdEqual(ID, fromID));
    }

    // If intervals doesn't cross zero then compare from
    // both the from and to ID's
    if(compareTo(fromID ,toID) < 0){
        return (compareTo(ID, fromID) > 0 && compareTo(ID, toID) < 0);
    }

    else{

        return (compareTo(ID, fromID) > 0 || compareTo(ID, toID) < 0);
    }
    
}

string addPowerOfTwo(int powerOfTwo,string ID)
{
		
		if(powerOfTwo < 0 || powerOfTwo >= (FINGER_TABLE_SIZE)) {
		
					
				
		}
	


}


