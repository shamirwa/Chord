service: Service.h Chord.h Node.h Entry.h Defs.h Successorlist.h
	g++ -o service Service.cpp Chord.cpp Node.cpp Entry.cpp Successorlist.cpp

clean:
	rm -rf service
