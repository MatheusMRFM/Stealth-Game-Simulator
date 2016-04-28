#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NUM_ACAO 		4
#define NUM_ESTADOS 	1536
#define R_MAX			1
#define R_MIN			-1
#define LIMIAR_ZERO		0.01

#define FITNESS_NULO	-1
#define PCT_MUTACAO		2
#define DESVIO_P		0.6
#define PESO_PAI		0.85
#define TAM_TORNEIO		2

#define TAM_POPULACAO	40
#define TAM_ELITE		3				///Indica quantos indivíduos passam para a próxima geração (os TAM_ELITE primeiros passam para a próxima geração)
#define NUM_MUT_ELITE	1
#define NUM_GERACAO 	1000
#define NUM_EXEC_TESTE 	100
#define NUM_EXEC_MAX	200
#define INCR_EXEC		10
#define NUM_EXEC_INCR	10



#define VAL_POS_INIMIGOS	8
#define VAL_CAMINHO_SEGURO	2
#define VAL_INIMIGO_APROX	4
#define VAL_VISAO_LIVRE		2
#define VAL_DIST_PROX		2
#define VAL_INIMIGO_PARADO	2

class Individuo {
    public:
		int fitness;
		float** func;
		
		Individuo(bool init);
		~Individuo();
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
		void ordena_fitness (bool recalcula_tudo);
		Individuo* crossover (Individuo* ind1, Individuo* ind2);
		void gera_filhos ();
		void salva_populacao (int geracao);
		void salva_dados_populacao (int geracao);
};

void quicksort(Individuo** vetor, int esq, int dir);
