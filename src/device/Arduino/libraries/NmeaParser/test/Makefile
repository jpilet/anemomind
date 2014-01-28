CXXFLAGS=-g -Wall

all: run_test

clean:
	rm -f test.o nmeaparser.o test

test: test.o nmeaparser.o

run_test: test
	./test

nmeaparser.o: ../NmeaParser.cpp ../NmeaParser.h
	$(CXX) $(CXXFLAGS) -c -o $@ $<

test.o: test.cpp ../NmeaParser.h
