CC = g++
PARAMETERS = -g -Wall
LIB = -lGL -lGLU -lglui -lglut

Stealth: main.o Lista.o Cenario.o Visual.o Agente.o Fugitivo.o Inimigo.o Geometria.o
	$(CC) $(PARAMETERS) -o Stealth main.o Lista.o Cenario.o Visual.o Agente.o Fugitivo.o Inimigo.o Geometria.o $(LIB)

main.o: main.cpp
	$(CC) $(PARAMETERS) -c main.cpp -o main.o $(LIB)

Cenario.o: Cenario.cpp Cenario.hpp
	$(CC) $(PARAMETERS) -c Cenario.cpp -o Cenario.o $(LIB)

Lista.o: Lista.cpp Lista.hpp
	$(CC) $(PARAMETERS) -c Lista.cpp -o Lista.o $(LIB)
	
Visual.o: Visual.cpp Visual.hpp
	$(CC) $(PARAMETERS) -c Visual.cpp -o Visual.o $(LIB)
	
Agente.o: Agente.cpp Agente.hpp
	$(CC) $(PARAMETERS) -c Agente.cpp -o Agente.o $(LIB)
	
Fugitivo.o: Fugitivo.cpp Fugitivo.hpp
	$(CC) $(PARAMETERS) -c Fugitivo.cpp -o Fugitivo.o $(LIB)
	
Inimigo.o: Inimigo.cpp Inimigo.hpp
	$(CC) $(PARAMETERS) -c Inimigo.cpp -o Inimigo.o $(LIB)
	
Geometria.o: Geometria.cpp Geometria.hpp
	$(CC) $(PARAMETERS) -c Geometria.cpp -o Geometria.o $(LIB)

clean:
	rm -f *.o Stealth *~
