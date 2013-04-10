service: Service.h Chord.h Node.h Entry.h 
	g++ -o service Service.cpp Chord.cpp Node.cpp Entry.cpp

clean:
	rm -rf service
