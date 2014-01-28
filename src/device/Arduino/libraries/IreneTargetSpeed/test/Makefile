CXXFLAGS=-g -Wall

all: run_test

clean:
	rm -f test.o IreneTargetSpeed.o test

test: test.o IreneTargetSpeed.o

run_test: test
	./test

IreneTargetSpeed.o: ../IreneTargetSpeed.cpp ../IreneTargetSpeed.h
	$(CXX) $(CXXFLAGS) -c -o $@ $<

test.o: test.cpp ../IreneTargetSpeed.h
