chord: Chord.cpp Chord.h Entry.cpp Entry.h Node.cpp Node.h
	g++ -o chord Chord.cpp Entry.cpp Node.cpp 

clean:
	rm -rf chord
