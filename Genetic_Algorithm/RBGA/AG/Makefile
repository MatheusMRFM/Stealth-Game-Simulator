CC = g++
PARAMETERS = -g -Wall
LIB = 

Reward: main.o AG.o 
	$(CC) $(PARAMETERS) -o Reward main.o AG.o $(LIB)

main.o: main.cpp
	$(CC) $(PARAMETERS) -c main.cpp -o main.o $(LIB)
	
AG.o: AG.cpp AG.hpp
	$(CC) $(PARAMETERS) -c AG.cpp -o AG.o $(LIB)

clean:
	rm -f *.o Reward *~
