#include "AG.hpp"
#include <math.h>

#define PI 3.14159265358979323846

//**********************************************************************
//************************** INDIVÍDUOS ********************************
//**********************************************************************
Individuo::Individuo (bool init) {
    this->func = (float**) malloc (sizeof(float*) * NUM_ESTADOS);
    for (int i = 0; i < NUM_ESTADOS; i++) {
	this->func[i] = (float*) malloc (sizeof(float) * NUM_ACAO);
    }
    this->fitness = FITNESS_NULO;
    
    if (init) {
	for (int i = 0; i < NUM_ESTADOS; i++) {
	    if (i % (VAL_POS_INIMIGOS*VAL_CAMINHO_SEGURO*VAL_INIMIGO_APROX*VAL_VISAO_LIVRE*VAL_DIST_PROX*VAL_INIMIGO_PARADO) < VAL_CAMINHO_SEGURO*VAL_INIMIGO_APROX*VAL_VISAO_LIVRE*VAL_DIST_PROX*VAL_INIMIGO_PARADO) {
		this->func[i][0] = R_MAX;
		for (int j = 1; j < NUM_ACAO; j++) {
		    this->func[i][j] = R_MIN;
		}
	    }
	    else {
		for (int j = 0; j < NUM_ACAO; j++) {
		    this->func[i][j] = (rand() % 2000)/1000.0 - 1;
		}
	    }
	}
    }
}
//----------------------------------------------------------------------
Individuo::~Individuo () {
    for (int i = 0; i < NUM_ESTADOS; i++) 
	free(this->func[i]);
    free(this->func);
}
//----------------------------------------------------------------------
void Individuo::mutacao () {
    float u1, u2, z1;
    int m;
    
    for (int i = 0; i < NUM_ESTADOS; i++) {
	///Caso o estado indique que nenhum inimigo esta perto do agente (VAL_POS_INIMIGOS = 0), então força-se uma recompensa por andar em direção ao destino
	if (i % (VAL_POS_INIMIGOS*VAL_CAMINHO_SEGURO*VAL_INIMIGO_APROX*VAL_VISAO_LIVRE*VAL_DIST_PROX*VAL_INIMIGO_PARADO) < VAL_CAMINHO_SEGURO*VAL_INIMIGO_APROX*VAL_VISAO_LIVRE*VAL_DIST_PROX*VAL_INIMIGO_PARADO) {
	    this->func[i][0] = R_MAX;
	    for (int j = 1; j < NUM_ACAO; j++) {
		this->func[i][j] = R_MIN;
	    }
	}
	else {
	    for (int j = 0; j < NUM_ACAO; j++) {
		m = rand() % 100;
		///Valor [i, j] vai sofrer mutação
		if (m < PCT_MUTACAO) {
		    ///Método de Box-Muller para gerar valores aleatórios com uma distribuição normal
		    u1 = (float)rand() / (float)RAND_MAX;
		    u2 = (float)rand() / (float)RAND_MAX;
		    z1 = sqrt(-2.0 * log(u1)) * cos(2.0*PI*u2);
		    this->func[i][j] += z1 * DESVIO_P;
		    
		    if (this->func[i][j] < R_MIN)
			this->func[i][j] = R_MAX;
		    if (this->func[i][j] > R_MAX)
			this->func[i][j] = R_MIN;
		    if (this->func[i][j] > -LIMIAR_ZERO && this->func[i][j] < LIMIAR_ZERO)
			this->func[i][j] = 0.0;
		}
	    }
	}
    }
}
//----------------------------------------------------------------------
void Individuo::calcula_fitness (int id, int num_exec) {
    ///Copia a função contida em Ind(id).cpp para Reward.cpp
    char comando[200];
    sprintf(comando, "cp Populacao/Ind%d.cpp ../Reward.cpp", id);
    system(comando);
    
    ///Executa o simulador para a nova função Reward.cpp
    printf("\n\n------- Indivíduo %d -------\n", id);
    system("make clean -C ../");
    system("make -C ../");
    sprintf(comando, ".././Stealth -m 1 -i 4 -e %d -z 0.1 -x 0.5 -c 0.0", num_exec);
    printf("%s\n", comando);
    system(comando);
    
    ///Lê no arquivo de saida o Fitness do individuo
    FILE *arq = fopen("Fitness.txt", "r");
    fscanf(arq, "%d", &this->fitness);
    printf("\nFitness do individuo %d = %d\n\n", id, this->fitness);
    fclose(arq);
}
//----------------------------------------------------------------------
void Individuo::salva_individuo (int id, bool melhor_ind, int geracao) {
    char nome[50];
    if (melhor_ind) 
	sprintf(nome, "Melhores_Ind/%d.cpp", geracao);
    else
	sprintf(nome, "Populacao/Ind%d.cpp", id);
    FILE *arq = fopen(nome, "w");
    
    ///Escreve no arquivo a definição da matriz de recompensas
    fprintf(arq, "float reward[%d][%d] = {\n", NUM_ESTADOS, NUM_ACAO);
    for (int i = 0; i < NUM_ESTADOS; i++) {
	for (int j = 0; j < NUM_ACAO; j++) {
	    fprintf(arq, "\t%f\t,", this->func[i][j]);
	}
	fprintf(arq, "\n");
    }
    fprintf(arq, "};\n\n");
    
    ///Escreve no arquivo a função de recompensa
    fprintf(arq, "float Fugitivo_RL::calcula_reforco() {\n");
    fprintf(arq, "\treturn reward[this->estado_anterior->id][this->acao_anterior];\n}");
    fclose(arq);
}
//----------------------------------------------------------------------
void Individuo::carrega_individuo (int id) {
    char cod[200], aux;
    sprintf(cod, "Populacao/Ind%d.cpp", id);
    FILE *arq = fopen(cod, "r");
    
    ///Le a linha inicial
    do {
	fscanf(arq, "%c", &aux);
    } while (aux != '\n');
    ///Le a matriz de recompensa
    for (int i = 0; i < NUM_ESTADOS; i++) {
	for (int j = 0; j < NUM_ACAO; j++) {
	    fscanf(arq, "\t%f\t%c", &this->func[i][j], &aux);
	}
	fscanf(arq, "\n");
    }
    
    fclose(arq);
}
//**********************************************************************
//*************************** POPULAÇÃO ********************************
//**********************************************************************
Populacao::Populacao (int modo_exec) {
    this->num_exec = NUM_EXEC_TESTE;
    ///Aloca espaço
    this->individuos = (Individuo**) malloc (sizeof(Individuo*) * TAM_POPULACAO);
    
    ///Cria os indivíduos da população
    if (modo_exec == 0) {
	for (int i = 0; i < TAM_POPULACAO; i++) {
	    this->individuos[i] = new Individuo(true);
	}
    }
    ///Carrega os indivíduos a partir dos arquivos na pasta População/
    else {
	for (int i = 0; i < TAM_POPULACAO; i++) {
	    this->individuos[i] = new Individuo(false);
	    this->individuos[i]->carrega_individuo(i);
	}
    }
}
//----------------------------------------------------------------------
Populacao::~Populacao () {
    for (int i = 0; i < TAM_POPULACAO; i++) 
	delete this->individuos[i];
    free(this->individuos);
}
//----------------------------------------------------------------------
void Populacao::ordena_fitness (bool recalcula_tudo) {
    for (int i = 0; i < TAM_POPULACAO; i++) {
	if (this->individuos[i]->fitness == FITNESS_NULO || recalcula_tudo)
	    this->individuos[i]->calcula_fitness(i, this->num_exec);
    }
    
    quicksort (this->individuos, 0, TAM_POPULACAO-1);
}
//----------------------------------------------------------------------
Individuo* Populacao::crossover (Individuo* ind1, Individuo* ind2) {
    Individuo* filho = new Individuo(false);
    
    for (int i = 0; i < NUM_ESTADOS; i++) {
	for (int j = 0; j < NUM_ACAO; j++) {
	    filho->func[i][j] = PESO_PAI*ind1->func[i][j] + ((float)(1.0-PESO_PAI))*ind2->func[i][j];
	}
    }
    
    return filho;
}
//----------------------------------------------------------------------
void Populacao::gera_filhos () {
    Individuo** filho = (Individuo**) malloc (sizeof(Individuo*) * TAM_POPULACAO-TAM_ELITE);
    Individuo* result_cross;
    int ind1 = 0, ind_torneio[TAM_TORNEIO], ind2;
    
    ///Muta alguns individuos da Elite
    int f = 0;
    for (int e = 0; e < TAM_ELITE; e++) {
	for (int m = 0; m < NUM_MUT_ELITE; m++) {
	    filho[f] = new Individuo(false);
	    for (int i = 0; i < NUM_ESTADOS; i++) {
		for (int j = 0; j < NUM_ACAO; j++) {
		    filho[f]->func[i][j] = this->individuos[e]->func[i][j];
		}
	    }
	    filho[f]->mutacao();
	    f++;
	}
    }
    
    ///Cada par de pais gera um filho
    for (f = f; f < (TAM_POPULACAO-TAM_ELITE); f++) {
	///Encontra os pais que vão realizar o crossover
	ind_torneio[TAM_TORNEIO-1] = f;
	///Sorteia k-1 individuos para o primeiro torneio
	for (int i = 0; i < TAM_TORNEIO-1; i++) {
	    ind_torneio[i] = rand() % TAM_POPULACAO;
	}
	///Seleciona o melhor dos k individuos
	ind1 = ind_torneio[0];
	for (int i = 1; i < TAM_TORNEIO; i++) {
	    if (this->individuos[ind1]->fitness < this->individuos[ind_torneio[i]]->fitness)
		ind1 = ind_torneio[i];
	}
	
	///Sorteia k individuos para o segundo torneio
	for (int i = 0; i < TAM_TORNEIO; i++) {
	    do {
		ind_torneio[i] = rand() % TAM_POPULACAO;
	    }while (ind1 == ind_torneio[i]);
	}
	///Seleciona o melhor dos k individuos
	ind2 = ind_torneio[0];
	for (int i = 1; i < TAM_TORNEIO; i++) {
	    if (this->individuos[ind2]->fitness < this->individuos[ind_torneio[i]]->fitness)
		ind2 = ind_torneio[i];
	}
	
	///Gera um par de filhos
	result_cross = this->crossover(this->individuos[ind1], this->individuos[ind2]);
	filho[f] = result_cross;
	filho[f]->mutacao();
	result_cross = NULL;
	free(result_cross);
    }
    
    ///Substitui alguns individuos da população pelos filhos
    for (int f = 0; f < TAM_POPULACAO-TAM_ELITE; f++) {
	delete this->individuos[f+TAM_ELITE];
	this->individuos[f+TAM_ELITE] = filho[f];
	filho[f] = NULL;
    }
    free(filho);
}
//----------------------------------------------------------------------
void Populacao::salva_dados_populacao (int geracao) {
    ///Salva um arquivo contendo o fitness de toda a população
    char nome[200];
    sprintf(nome, "Melhores_Ind/%d_populacao.txt", geracao);
    FILE *arq = fopen(nome, "w");
    for (int i = 0; i < TAM_POPULACAO; i++) {
	fprintf(arq, "%d\n", this->individuos[i]->fitness);
    }
    fclose(arq);
    
    ///Salva o melhor individuo
    this->individuos[0]->salva_individuo(0, true, geracao);
}
//----------------------------------------------------------------------
void Populacao::salva_populacao (int geracao) {
    ///Salva todos os individuos da população
    for (int i = 0; i < TAM_POPULACAO; i++) {
	this->individuos[i]->salva_individuo(i, false, geracao);
    }
}
//**********************************************************************
//*************************** QUICKSORT ********************************
//**********************************************************************
void quicksort(Individuo** vetor, int esq, int dir) {
    int i, j, pivo;

    i = esq;
    j = dir;
    pivo = vetor[(esq + dir)/2]->fitness;

    do {
        while (vetor[i]->fitness > pivo)
            i++;
        while (pivo > vetor[j]->fitness)
            j--;

        if (i <= j) {
	    Individuo* aux = vetor[i];
	    vetor[i] = vetor[j];
	    vetor[j] = aux;
            i++;
            j--;
        }
    } while (i <= j);

    if (esq < j)
        quicksort(vetor, esq, j);
    if (i < dir)
        quicksort(vetor, i, dir);

}
