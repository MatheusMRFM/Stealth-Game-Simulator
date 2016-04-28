#include "AG.hpp"
#include <math.h>

#define PI 3.14159265358979323846

extern int val_variaveis[NUM_VAR];
int e_atual[NUM_VAR];

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
	int var, val, a;
	for (int i = 0; i < NUM_REGRA_INICIAL; i++) {
	    var = rand() % NUM_VAR;
	    val = rand() % val_variaveis[var];
	    a = rand() % (NUM_ACAO);		
	    this->regras[i].variavel = var;
	    this->regras[i].valor = val;
	    this->regras[i].acao = a;
	}
	this->regras[NUM_REGRA_INICIAL].variavel = -1;	///Fim das regras
	
	this->atribui_regras();
    }
}
//----------------------------------------------------------------------
Individuo::~Individuo () {
    for (int i = 0; i < NUM_ESTADOS; i++) 
	free(this->func[i]);
    free(this->func);
}
//----------------------------------------------------------------------
void Individuo::atribui_regras () {
    ///Verifica se todas as regras são válidas
    for (int r = 0; r < NUM_REGRA_TOTAL && this->regras[r].variavel != -1; r++) {
	///Se pos_inimigos == 0, então force o agente a andar em frente
	if (this->regras[r].variavel == 0 && this->regras[r].valor == 0)
	    this->regras[r].acao = 0;
    }
    
    for (int i = 0; i < NUM_ESTADOS; i++) {
	this->define_variaveis_estado(i);
	///Preenche os valores padrões
	for (int j = 0; j < NUM_ACAO; j++) {
	    if (j == 0)
		this->func[i][j] = R_MAX;
	    else
		this->func[i][j] = R_MIN;
	}
	///Atribui as regras aos estados correspondentes
	for (int r = 0; r < NUM_REGRA_TOTAL && this->regras[r].variavel != -1; r++) {
	    if (e_atual[this->regras[r].variavel] == this->regras[r].valor) {
		for (int j = 0; j < NUM_ACAO; j++) {
		    if (j == this->regras[r].acao)
			this->func[i][j] = R_MAX;
		    else
			this->func[i][j] = R_MIN;
		}
	    }
	}
    }
}
//----------------------------------------------------------------------
void Individuo::define_variaveis_estado (int id_estado) {
    int t;
    
    ///VAL_POS_INIMIGOS
    t = id_estado % (VAL_POS_INIMIGOS*VAL_INIMIGO_APROX*VAL_VISAO_LIVRE*VAL_DIST_PROX*VAL_INIMIGO_PARADO);
    this->estado.pos_inimigos = VAL_POS_INIMIGOS-1;
    for (int i = 1; i < VAL_POS_INIMIGOS; i++) {
	if (t < i*VAL_INIMIGO_APROX*VAL_VISAO_LIVRE*VAL_DIST_PROX*VAL_INIMIGO_PARADO) {
	    this->estado.pos_inimigos = i-1;
	    break;
	}
    }
    
    ///VAL_INIMIGO_APROX
    t = id_estado % (VAL_INIMIGO_APROX*VAL_VISAO_LIVRE*VAL_DIST_PROX*VAL_INIMIGO_PARADO);
    this->estado.aproxima = VAL_INIMIGO_APROX-1;
    for (int i = 1; i < VAL_INIMIGO_APROX; i++) {
	if (t < i*VAL_VISAO_LIVRE*VAL_DIST_PROX*VAL_INIMIGO_PARADO) {
	    this->estado.aproxima = i-1;
	    break;
	}
    }
    
    ///VAL_VISAO_LIVRE
    t = id_estado % (VAL_VISAO_LIVRE*VAL_DIST_PROX*VAL_INIMIGO_PARADO);
    this->estado.visao_livre = VAL_VISAO_LIVRE-1;
    for (int i = 1; i < VAL_VISAO_LIVRE; i++) {
	if (t < i*VAL_DIST_PROX*VAL_INIMIGO_PARADO) {
	    this->estado.visao_livre = i-1;
	    break;
	}
    }
    
    ///VAL_DIST_PROX
    t = id_estado % (VAL_DIST_PROX*VAL_INIMIGO_PARADO);
    this->estado.dist_prox = VAL_DIST_PROX-1;
    for (int i = 1; i < VAL_DIST_PROX; i++) {
	if (t < i*VAL_INIMIGO_PARADO) {
	    this->estado.dist_prox = i-1;
	    break;
	}
    }
    
    ///VAL_INIMIGO_PARADO
    t = id_estado % (VAL_INIMIGO_PARADO);
    this->estado.parado = VAL_INIMIGO_PARADO-1;
    for (int i = 1; i < VAL_INIMIGO_PARADO; i++) {
	if (t < i) {
	    this->estado.parado = i-1;
	    break;
	}
    }
    
    e_atual[0] = this->estado.pos_inimigos;
    e_atual[1] = this->estado.aproxima;
    e_atual[2] = this->estado.visao_livre;
    e_atual[3] = this->estado.dist_prox;
    e_atual[4] = this->estado.parado;
}
//----------------------------------------------------------------------
void Individuo::mutacao () {
    int m;
    for (int r = 0; r < NUM_REGRA_TOTAL && this->regras[r].variavel != -1; r++) {
	m = rand() % 101;
	///Regra será mutada
	if (m < PCT_MUTACAO) {
	    ///Define qual valor será mutado (variavel, valor ou acao)
	    m = rand() % 3;
	    if (m == 0) 
		this->regras[r].variavel = rand() % NUM_VAR;
	    else if (m == 1) 
		this->regras[r].valor = rand() % val_variaveis[this->regras[r].variavel];
	    else 
		this->regras[r].acao = rand() % NUM_ACAO;
	}
    }
    
    this->atribui_regras();
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
    
    if (melhor_ind) 
	sprintf(nome, "Melhores_Ind/%d_Regras.txt", geracao);
    else
	sprintf(nome, "Populacao/Ind%d_Regras.txt", id);
    arq = fopen(nome, "w");
    
    ///Escreve no arquivo as regras usadas pelo indivíduo
    for (int r = 0; r < NUM_REGRA_TOTAL && this->regras[r].variavel != -1; r++) {
	fprintf(arq, "%d\t%d\t%d\n", this->regras[r].variavel, this->regras[r].valor, this->regras[r].acao);
    }
    fclose(arq);
}
//----------------------------------------------------------------------
void Individuo::carrega_individuo (int id) {
    char cod[200];
    sprintf(cod, "Populacao/Ind%d_Regras.txt", id);
    FILE *arq = fopen(cod, "r");
    int r = 0;
    
    ///Le as regras do individuo
    while (!feof(arq)) {
	fscanf(arq, "%d\t%d\t%d\n", &this->regras[r].variavel, &this->regras[r].valor, &this->regras[r].acao);
	r++;
    }
    this->regras[r].variavel = -1;
    this->atribui_regras();
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
    int e;
    int num_regras = 0;
    Individuo* filho = new Individuo(false);
    
    ///Escolhe as regras do pai 1
    for (int r = 0; r < NUM_REGRA_TOTAL && ind1->regras[r].variavel != -1 && num_regras < NUM_REGRA_TOTAL; r++) {
	e = rand() % 101;
	if (e < PCT_ATRIBUI) {
	    filho->regras[num_regras].variavel = ind1->regras[r].variavel;
	    filho->regras[num_regras].valor = ind1->regras[r].valor;
	    filho->regras[num_regras].acao = ind1->regras[r].acao;
	    num_regras++;
	}
    }
    
    ///Escolhe as regras do pai 2
    for (int r = 0; r < NUM_REGRA_TOTAL && ind2->regras[r].variavel != -1 && num_regras < NUM_REGRA_TOTAL; r++) {
	e = rand() % 101;
	if (e < PCT_ATRIBUI) {
	    filho->regras[num_regras].variavel = ind2->regras[r].variavel;
	    filho->regras[num_regras].valor = ind2->regras[r].valor;
	    filho->regras[num_regras].acao = ind2->regras[r].acao;
	    num_regras++;
	}
    }
    
    if (num_regras < NUM_REGRA_TOTAL)
	filho->regras[num_regras].variavel = -1;
    
    return filho;
}
//----------------------------------------------------------------------
void Populacao::gera_filhos () {
    Individuo** filho = (Individuo**) malloc (sizeof(Individuo*) * TAM_POPULACAO-TAM_ELITE);
    Individuo* result_cross;
    int ind1 = 0, ind_torneio[TAM_TORNEIO], ind2;
    
    ///Muta alguns individuos da Elite
    int f = 0;
    for (int m = 0; m < NUM_MUT_ELITE; m++) {
	filho[f] = new Individuo(false);
	for (int r = 0; r < NUM_REGRA_TOTAL; r++) {
	    filho[f]->regras[r].variavel = this->individuos[m]->regras[r].variavel;
	    filho[f]->regras[r].valor = this->individuos[m]->regras[r].valor;
	    filho[f]->regras[r].acao = this->individuos[m]->regras[r].acao;
	}
	filho[f]->mutacao();
	f++;
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
	    if (vetor[i]->fitness != vetor[j]->fitness) {
		Individuo* aux = vetor[i];
		vetor[i] = vetor[j];
		vetor[j] = aux;
	    }
            i++;
            j--;
        }
    } while (i <= j);

    if (esq < j)
        quicksort(vetor, esq, j);
    if (i < dir)
        quicksort(vetor, i, dir);

}
