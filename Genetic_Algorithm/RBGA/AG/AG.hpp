#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NUM_ACAO 		4
#define NUM_ESTADOS 	256
#define R_MAX			1
#define R_MIN			-1
#define LIMIAR_ZERO		0.01

#define FITNESS_NULO	-1
#define PCT_MUTACAO		10
#define PCT_ATRIBUI		50
#define TAM_TORNEIO		2

#define TAM_POPULACAO	40
#define TAM_ELITE		3
#define NUM_MUT_ELITE	1
#define NUM_GERACAO 	1000
#define NUM_EXEC_TESTE 	100

#define NUM_REGRA_INICIAL 	10
#define NUM_REGRA_TOTAL		20

#define NUM_VAR				5
#define VAL_POS_INIMIGOS	8	///0
#define VAL_INIMIGO_APROX	4	///1
#define VAL_VISAO_LIVRE		2	///2
#define VAL_DIST_PROX		2	///3
#define VAL_INIMIGO_PARADO	2	///4

typedef struct Regra {
	int variavel;
	int valor;
	int acao;
}Regra;

typedef struct Estado {
	int id;
	int pos_inimigos;
	int aproxima;
	bool visao_livre;
	int dist_prox;
	bool parado; 
}Estado;

class Individuo {
    public:
		int fitness;
		float** func;
		Regra regras[NUM_REGRA_TOTAL];
		Estado estado;
		
		Individuo(bool init);
		~Individuo();
		void define_variaveis_estado (int id_estado);
		void atribui_regras ();
		void mutacao ();
		void calcula_fitness (int id, int num_exec);
		void salva_individuo (int id, bool melhor_ind, int geracao);
		void carrega_individuo (int id);
};

class Populacao {
    public:
		Individuo** individuos;
		int num_exec;
		
		Populacao(int modo_exec);
		~Populacao();
		Individuo* crossover (Individuo* ind1, Individuo* ind2);
		void ordena_fitness (bool recalcula_tudo);
		void gera_filhos ();
		void salva_populacao (int geracao);
		void salva_dados_populacao (int geracao);
};

void quicksort(Individuo** vetor, int esq, int dir);
