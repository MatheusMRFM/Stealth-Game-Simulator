#include "AG.hpp"

Populacao* populacao;

int val_variaveis[NUM_VAR] = {8, 4, 2, 2, 2};

int usage(char *str) {
    fprintf(stderr,"Usage: %s [-h] [-m]\n ", str);
    fprintf(stderr,"\t-m : Execution mode\n");
    fprintf(stderr,"\t\t * 0 : Initial population is generated randomly\n");
    fprintf(stderr,"\t\t * 1 : Initial population is loaded from the folder 'Populacao/'\n");
    fprintf(stderr,"\t- g : Number of the starting generation\n");
    fprintf(stderr,"\t-h : Shows this menu\n");
    exit(0);
}

int main (int argc, char** argv) {
    srand (time(NULL));
    int modo_exec = 0;
    int geracao_inicial = 0;
    
    ///Pega os parametros do programa
    int c;
    for (int i = 1; i < argc; i++)   {
	if (argv[i][0] != '-')
		usage(argv[0]);

	c = argv[i][1];

	switch (c) {
	    case 'm':
		if (++i >= argc)
		    usage(argv[0]);
		
		modo_exec = atoi(argv[i]);
		break;
	    case 'g':
		if (++i >= argc)
		    usage(argv[0]);
		
		geracao_inicial = atoi(argv[i]);
		break;
	    case 'h':
	    default:
		usage(argv[0]);
	}
    }
    
    ///Gera a população inicial
    populacao = new Populacao(modo_exec);
    populacao->salva_populacao(-1);
    
    ///Gerações
    for (int g = geracao_inicial; g < NUM_GERACAO; g++) {
	printf("********************************\n");
	printf("********* GERAÇÃO %d ***********\n", g);
	printf("********************************\n");
	populacao->ordena_fitness(false);
	populacao->salva_dados_populacao (g);
	for (int p = 0; p < TAM_POPULACAO; p++) {
	    printf("%d\n", populacao->individuos[p]->fitness);
	}
	populacao->gera_filhos();
	populacao->salva_populacao(g);
    }
    
    populacao->ordena_fitness(false);
    populacao->salva_populacao(NUM_GERACAO);
    
    for (int g = 0; g < TAM_POPULACAO; g++) {
	printf("%d\n", populacao->individuos[g]->fitness);
    }
    
    return 0;
}
