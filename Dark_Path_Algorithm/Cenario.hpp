#ifndef CENARIO_HPP
#define CENARIO_HPP

#include "Lista.hpp"
#include "Geometria.hpp"

#define TEMPO_RECALCULA 100

#define PI 3.14159265
#define TAM_GRID 100
#define PCT_NUM_PAREDES 0.5			///O número de paredes é MIN_PAREDE + um valor aleatório x TAM_CENARIO x PCT_NUM_PAREDES
#define MIN_PAREDE 20
#define PCT_MAX_PAREDE 0.05			///Tamanho de uma parede é MIN_LARGURA_PAREDE + um valor aleatório x PCT_MAX_PAREDE x TAM_CENARIO
#define MIN_LARGURA_PAREDE 7	

#define VALOR_GRANDE 100000000
#define COEF_VISIBILIDADE 0.99
#define CUSTO_DIAGONAL 1.414213562	///sqrt(2)
#define CUSTO_RETO	1.0

#define COEF_FURTIVIDADE 10.0
#define COEF_DIST 1.0

float minimo (float a1, float a2, float a3, float a4, float a5);

typedef struct Grid_Cell {
	float x, y;			///Coordenadas do centro da celula
	int exp_inimigo;	///Indica quantos inimigos conseguem ver esta celula + exp_ambiente
	int exp_ambiente;	///Indica o quantas células conseguem ver esta celula (== -1 -> obstaculo)
	float dt;
	float dp;
}Grid_Cell;

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

class Cenario {
	public:
		float tamanho;
		float tam_cell;
		int num_parede;
		Grid_Cell **grid;
		Parede **paredes;
		float max_exposicao;
		int recalcula;
		
		Cenario(float tam);
		~Cenario();
		void Cria_Paredes();
		void insere_paredes_grid ();
		void encontra_maxima_exposicao ();
		bool Verifica_intersecao ();
		void reinicia_visao_inimigo ();
		void inicializa_grids (Grid_Cell **g1, Grid_Cell **g2, int a, int b);
		void inicializa_grid_principal (int a, int b);
		void inicializa_grid_inimigo (Grid_Cell **g, int a, int b);
		void Distance_Transform (Grid_Cell **g);
		void Visibility_Algorithm ();
		void Dark_Path_Algortihm (Grid_Cell **g, Ponto_Mapa *destino);
};

#endif
