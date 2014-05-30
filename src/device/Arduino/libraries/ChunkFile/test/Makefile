CXXFLAGS=-g -Wall

all: run_test

clean:
	rm -f ChunkFileTest.o ChunkFile.o test

test: ChunkFileTest.o ChunkFile.o
	$(CXX) ChunkFileTest.o ChunkFile.o -o test

run_test: test
	./test

ChunkFile.o: ../ChunkFile.cpp ../ChunkFile.h
	$(CXX) $(CXXFLAGS) -c -o $@ $<

ChunkFileTest.o: ChunkFileTest.cpp ../ChunkFile.h
