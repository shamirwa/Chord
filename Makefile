all:	service chord slist node entry
	g++ service chord slist node entry -o all -lcrypto

service:	Service.cpp Service.h Chord.h Defs.h Node.h Successorlist.h Entry.h
	g++ -c -g -o service Service.cpp -lcrypto

chord:	Chord.cpp Chord.h Exception.h Defs.h Node.h Entry.h Successorlist.h
	g++ -c -g -o chord Chord.cpp 

slist:	Successorlist.cpp Successorlist.h Node.h Entry.h
	g++ -c -g -o slist Successorlist.cpp 

node:	Node.cpp Node.h Defs.h Entry.h
	g++ -c -g -o node Node.cpp 

entry:	Entry.cpp Entry.h
	g++ -c -g -o entry Entry.cpp

clean:
	rm -rf entry node slist chord service
