all: hw4

.cpp.o:
	g++ -g -Wall -c $< -lcapstone

hw4: elftool.o hw4.o
	g++ -g -Wall -o $@ $^ -lcapstone


clean:
	rm *.o hw4
