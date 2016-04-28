CC = g++
PARAMETERS = -g -Wall
LIB = -lGL -lGLU -lglui -lglut

Stealth: main.o Lista.o Bspline.o Cenario.o Visual.o AStar.o Agente.o Fugitivo.o Inimigo.o QL.o Geometria.o
	$(CC) $(PARAMETERS) -o Stealth main.o Lista.o Bspline.o Cenario.o Visual.o AStar.o Agente.o Fugitivo.o Inimigo.o QL.o Geometria.o $(LIB)

main.o: main.cpp
	$(CC) $(PARAMETERS) -c main.cpp -o main.o $(LIB)

Cenario.o: Cenario.cpp Cenario.hpp
	$(CC) $(PARAMETERS) -c Cenario.cpp -o Cenario.o $(LIB)

Lista.o: Lista.cpp Lista.hpp
	$(CC) $(PARAMETERS) -c Lista.cpp -o Lista.o $(LIB)

Bspline.o: Bspline.cpp Bspline.hpp
	$(CC) $(PARAMETERS) -c Bspline.cpp -o Bspline.o $(LIB)
	
Visual.o: Visual.cpp Visual.hpp
	$(CC) $(PARAMETERS) -c Visual.cpp -o Visual.o $(LIB)
	
AStar.o: A_Star.cpp A_Star.hpp
	$(CC) $(PARAMETERS) -c A_Star.cpp -o AStar.o $(LIB)
	
Agente.o: Agente.cpp Agente.hpp
	$(CC) $(PARAMETERS) -c Agente.cpp -o Agente.o $(LIB)
	
Fugitivo.o: Fugitivo.cpp Fugitivo.hpp
	$(CC) $(PARAMETERS) -c Fugitivo.cpp -o Fugitivo.o $(LIB)
	
Inimigo.o: Inimigo.cpp Inimigo.hpp
	$(CC) $(PARAMETERS) -c Inimigo.cpp -o Inimigo.o $(LIB)
	
QL.o: QL.cpp QL.hpp
	$(CC) $(PARAMETERS) -c QL.cpp -o QL.o $(LIB)
	
Geometria.o: Geometria.cpp Geometria.hpp
	$(CC) $(PARAMETERS) -c Geometria.cpp -o Geometria.o $(LIB)

clean:
	rm -f *.o Stealth *~
