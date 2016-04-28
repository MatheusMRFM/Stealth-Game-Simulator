#ifndef ASTAR_HPP
#define ASTAR_HPP

#include "Cenario.hpp"
#include "Geometria.hpp"

#define PESO_MANTEM_COBERTURA  0//0.9
#define PESO_COBERTURA_NEG 3//10 - Usado no artigo
#define PESO_SAI_COBERTURA 3//10 - Usado no artigo
#define PESO_H_COBERTURA 0.1
#define PESO_AREA_RISCO 2//50 - Usado no artigo
#define PESO_APROXIMA 2

#define PESO_IMPEDIMENTO 9999999
#define LIMITE_IMPEDIMENTO 3

#define DISTANCIA_SEGURANCA 150

class Inimigo;

class Nodo {
	public:
		float g, h, f;		
		char estado;		///-1 = nó fechado, 0 = nó não explorado, 1 = nó aberto
		int pai;			///Informação usada para geração do caminho
		Mesh *mesh;
		float x, y;			///coordenadas do centro da mesh (se for um triangulo, trata-se do baricentro)
		float max_x, min_x;	///Min e Max valores de X da mesh correspondente
		float max_y, min_y;	///Min e Max valores de Y da mesh correspondente
		bool pertence_caminho;
		int existe_inimigo;
		
		Nodo(Mesh *m);
		~Nodo();
};

class Grafo {
	private:
		bool modo_furtivo;
		
	public:
		int num_no;
		Nodo **nodo;
		float **m_adj;
		int **peso;
		ListaDE *closed;
		ListaDE *open;
		bool caminho_circular;
		Inimigo **inimigo;
		int num_inimigos;
		
		Grafo(NavMesh *nav);
		~Grafo();
		void set_modo (bool valor);
		void Liga_Nodos_Cobertura ();
		void Liga_Nodos_Triangulares ();
		void Liga_Nodos_Restantes ();
		float encontra_g (int m1, int m2, Ponto_Mapa *p, bool start);		///Encontra a distância entre duas meshes
		float encontra_h (int atual, Ponto_Mapa *destino, Ponto_Mapa *start);
		int encontra_mesh (Ponto_Mapa *p);
		bool encontra_menor_caminho (Ponto_Mapa *start, int mesh_start, Ponto_Mapa *destino, int mesh_destino);
		void limpa_caminho_anterior (bool limpa_pesos);
		void penaliza_transicao_mesh (int m, int destino);
};

#endif
