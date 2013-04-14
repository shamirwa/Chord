all:	service chord slist node entry utils
	g++ service chord slist node entry utils -o all -lcrypto

service:	Service.cpp Service.h Chord.h Defs.h Node.h Successorlist.h Entry.h myUtils.h
	g++ -c -g -o service Service.cpp 

chord:	Chord.cpp Chord.h Exception.h Defs.h Node.h Entry.h Successorlist.h myUtils.h
	g++ -c -g -o chord Chord.cpp 

slist:	Successorlist.cpp Successorlist.h Node.h Entry.h
	g++ -c -g -o slist Successorlist.cpp 

node:	Node.cpp Node.h Defs.h Entry.h
	g++ -c -g -o node Node.cpp 

entry:	Entry.cpp Entry.h
	g++ -c -g -o entry Entry.cpp

utils:	myUtils.cpp myUtils.h Exception.h
	g++ -c -g -o utils myUtils.cpp -lcrypto
clean:
	rm -rf entry node slist chord service all
