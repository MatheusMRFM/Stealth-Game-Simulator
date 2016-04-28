#ifndef CENARIO_HPP
#define CENARIO_HPP

#include "Lista.hpp"

#define PI 3.14159265
#define NUM_MESH_DEFAULT 2000
#define PCT_NUM_PAREDES 0.5			///O número de paredes é MIN_PAREDE + um valor aleatório x TAM_CENARIO x PCT_NUM_PAREDES
#define MIN_PAREDE 20
#define PCT_MAX_PAREDE 0.05			///Tamanho de uma parede é MIN_LARGURA_PAREDE + um valor aleatório x PCT_MAX_PAREDE x TAM_CENARIO
#define MIN_LARGURA_PAREDE 7	
#define LARGURA_MESH_COVER 5 
#define DIFF_PERMETIDA 3			///Cada mesh do cenário pode ser um retângulo onde um dos lados é no máximo DIFF_PERMITIDA vezes maior que o outro

class Poligono {
	public:
		Ponto_Mapa pontos[4];
		float dx;
		float dy;
		
		Poligono() { };
		void Print_pontos();
};

class Parede : public Poligono {
	public:
		float x,y;
		float rotacao; ///em graus (0 - 180)
		
		Parede(float x, float y, float dimx, float dimy, float rot);
		~Parede();
		bool Verifica_valido (float tam);
};

class Mesh : public Poligono {
	public:
		int tipo;					///0 -> Normal, 1 -> cobertura, 2 -> quadradinhos entre coberturas, 3 -> triangulos
		int parede_pertencente;		///Se a Mesh for de cobertura, indica a qual parede pertence
		
		Mesh(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, int t, int parede_pai);
		Mesh(float x1, float y1, float x2, float y2, float x3, float y3, int t, int parede_pai);
		~Mesh(){ };
};

class NavMesh {
	public:
		int num_meshes;
		Mesh **meshes;
		
		NavMesh();
		~NavMesh();
};

class Cenario {
	public:
		float tamanho;
		int num_parede;
		Parede **paredes;
		NavMesh *nav_mesh;
		Pilha *frontWall;
		Pilha *aberto;
		bool erro;
		
		Cenario(float tam);
		~Cenario();
		void Cria_Paredes();
		bool Verifica_intersecao ();
		void Cria_Mesh_Cobertura ();
		void Cria_Mesh_Restante();
		void cria_Mesh_quadrada (Elemento *fora, Elemento *aux);
		void Cria_NavMesh ();
};

#endif
