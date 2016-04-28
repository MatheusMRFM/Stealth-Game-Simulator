#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h> 
#include "Lista.hpp"
#include "Bspline.hpp"
#include "Cenario.hpp"
#include "A_Star.hpp"
#include "QL.hpp"
#include "Inimigo.hpp"

///Define Sistema Operacional
#ifdef _WIN32
	#define CLEAR "cls"
#elif defined __gnu_linux__
	#define CLEAR "clear"
#endif

#define TAM_CENARIO 500
#define MAX_PASSOS 180000

#define WIDTH 900
#define HEIGHT 700

///Callbacks
void init(int argc, char** argv);
void display(void);
void reshape(int, int);
void mouseMotion(int, int);
void mouse(int button, int state, int x, int y);
void keyboard(unsigned char, int, int);
void idle ();
void reinicia();

int num_inimigos;
Cenario *cenario;
Fugitivo_RL *fugitivo;
Inimigo **inimigo;

int sucesso;
int falhas;
int exec;
int max_exec;

char* nome_arq;
char* arq_saida;
int modo_exec;

float aprendizado;
float desconto;
float exploracao;

///Controla o número de pasos máximo permitido por simulação (para encerrar o programa caso ocorra algum problema)
int cont_passos;
FILE* arq;

extern int SEED_FIXA;
int seeds[100] = {  84,  96,  103, 113, 129, 156, 159, 162, 164, 168, 171, 185, 210, 233, 238, 285, 301, 316, 326, 333, 343, 356, 361, 376, 378, 
					395, 411, 413, 423, 425, 429, 435, 443, 455, 464, 475, 483, 492, 500, 504, 505, 511, 513, 519, 522, 529, 539, 545, 555, 563, 571, 580, 588, 592, 
					599, 595, 604, 608, 612, 615, 618, 619, 622, 625, 628, 645, 657, 652, 660, 661, 666, 669, 671, 672, 681, 684, 693, 696, 699, 701, 713, 717, 720,
					724, 734, 741, 750, 755, 760, 769, 770, 781, 790, 810, 850, 992, 1007,1011,1012,1013,};

///MAIN
int main(int argc, char** argv) {
    init(argc, argv);
    
    while (1) {
		idle();
	}
    
    return 0;
}

int usage(char *str) {
	fprintf(stderr,"Uso: %s [-h] [-m] [-i] [-e]\n ", str);
	fprintf(stderr,"\t-m : Modo de execução\n");
	fprintf(stderr,"\t\t * 0 : Executa apenas 1 vez usando a interface gráfica\n");
	fprintf(stderr,"\t\t * 1 : Executa E vezes sem a interface gráfica, onde E é o numéro de execuções (-e)\n");
	fprintf(stderr,"\t-i : Número de inimigos\n");
	fprintf(stderr,"\t-e : Número de execuções\n");
	fprintf(stderr,"\t-a : Arquivo de entrada contendo a QTable inicial (opcional)\n");
	fprintf(stderr,"\t-s : Nome do arquivo de saída\n");
	fprintf(stderr,"\t-h : Exibe esta mensagem de ajuda\n");
	exit(0);
}

void mensagem_inicial () {
	/*printf("1 - Oculta ou desenha o grafo;\n");
	printf("2 - Oculta ou desenha as meshes;\n");
	printf("3 - Oculta ou desenha o caminho de meshes;\n");
	printf("4 - Oculta ou desenha os nodos fechados pelo A*;\n");
	printf("5 - Oculta ou desenha o caminho exato;\n");
	printf("6 - Oculta ou desenha os pontos de controle da Bspline;\n");
	printf("7 - Oculta ou desenha as linhas de controle da Bspline;\n\n");
	
	printf("c - Alterna entre modo de cobertura ou modo contínuo;\n");
	printf("p - Move para a próxima cobertura (funciona somente no modo Cobertura);\n");
	printf("e - Move-se para um esconderijo seguro;\n");
	printf("s - Para ou move o fugitivo;\n");*/
}

///GLUT: INIT
void init(int argc, char** argv) {
	modo_exec = 0;
	max_exec = 100;
	sucesso = 0;
	falhas = 0;
	exec = 0;
	SEED_FIXA = seeds[exec];
	//printf("Seed = %d\n", SEED_FIXA);
    num_inimigos = 3;
    nome_arq = NULL;
    arq_saida = NULL;
    aprendizado = ALPHA_INICIAL;
    desconto = GAMMA_INICIAL;
    exploracao = PHI_INICIAL;
    arq = fopen("Fitness.txt", "w");
    
    char c;
	
	for (int i = 1; i < argc; i++)   {
		if (argv[i][0] != '-')
			usage(argv[0]);

		c = argv[i][1];

		switch (c)      {
			case 'm':
				if (++i >= argc)
					usage(argv[0]);
				
				modo_exec = atoi(argv[i]);
				break;
			case 'i':
				if (++i >= argc)
					usage(argv[0]);
				
				num_inimigos = atoi(argv[i]);
				break;
			case 'e':
				if (++i >= argc)
					usage(argv[0]);
				
				max_exec = atoi(argv[i]);
				break;
			case 'a':
				if (++i >= argc)
					usage(argv[0]);
					
				nome_arq = argv[i];
				break;
			case 's':
				if (++i >= argc)
					usage(argv[0]);
					
				arq_saida = argv[i];
				break;
			case 'z':
				if (++i >= argc)
					usage(argv[0]);
				
				aprendizado = atof(argv[i]);
				break;
			case 'x':
				if (++i >= argc)
					usage(argv[0]);
				
				desconto = atof(argv[i]);
				break;
			case 'c':
				if (++i >= argc)
					usage(argv[0]);
				
				exploracao = atof(argv[i]);
				break;
			case 'h':
			default:
				usage(argv[0]);
		}
	}
	
	mensagem_inicial();
    	
    cenario = new Cenario(TAM_CENARIO);
    cenario->Cria_Paredes();
    cenario->Cria_NavMesh();
    while (cenario->erro) {
		delete cenario;
		cenario = new Cenario(TAM_CENARIO);
		cenario->Cria_Paredes();
		cenario->Cria_NavMesh();
	}
    //printf("\n\tMeshes : %d\n", cenario->nav_mesh->num_meshes);
    
    fugitivo = new Fugitivo_RL(cenario, nome_arq, aprendizado, desconto, exploracao);
    fugitivo->gera_posicao_inicial(cenario->tamanho);
    
    inimigo = (Inimigo**) malloc (sizeof(Inimigo*)*num_inimigos);
    for (int i = 0; i < num_inimigos; i++) {
		inimigo[i] = new Inimigo(cenario, i);
		inimigo[i]->gera_posicao_inicial(cenario->tamanho, fugitivo, cenario->paredes, cenario->num_parede);
	}
	
	fugitivo->grafo->inimigo = inimigo;
	fugitivo->grafo->num_inimigos = num_inimigos;
	
	fugitivo->gera_caminho(cenario, inimigo, num_inimigos);
    //printf("\tOk\n");
    cont_passos = 0;
}

void reinicia() {
	SEED_FIXA = seeds[exec];
	//printf("Seed = %d\n", SEED_FIXA);
	cenario->erro = true;
	while (cenario->erro) {
		delete cenario;
		cenario = new Cenario(TAM_CENARIO);
		cenario->Cria_Paredes();
		cenario->Cria_NavMesh();
	}
    //printf("\tMeshes : %d\n", cenario->nav_mesh->num_meshes);
    
    fugitivo->inicializa_dados(true);
    fugitivo->restaura_valores(cenario);
    fugitivo->gera_posicao_inicial(cenario->tamanho);
    
    for (int i = 0; i < num_inimigos; i++) {
		inimigo[i]->inicializa_dados(true);
		inimigo[i]->restaura_valores(cenario);
		inimigo[i]->gera_posicao_inicial(cenario->tamanho, fugitivo, cenario->paredes, cenario->num_parede);
	}
	
	fugitivo->grafo->inimigo = inimigo;
	fugitivo->grafo->num_inimigos = num_inimigos;
	
	fugitivo->gera_caminho(cenario, inimigo, num_inimigos);
	//printf("\tOk\n");
	cont_passos = 0;
}


void idle () {
	if (modo_exec == 1) {
		if (cont_passos > MAX_PASSOS) {
			//printf("Reiniciando\n");
			exec++;
			falhas++;
			reinicia();
		}
		else if (fugitivo->chegou_destino) {
			exec++;
			sucesso++;
			//printf("Total = %d \tSucesso = %f%% (%d) (f)\n", exec, ((float)sucesso/(float)exec)*100.00, sucesso);
			if (exec >= max_exec) {
				//fugitivo->savla_QValues(arq_saida);
				fprintf(arq, "%d", ((int)(((float)sucesso/(float)max_exec)*100.00)));
				exit(0);
			}
			///Restart
			reinicia();
		}
		else if (fugitivo->encontrado) {
			fugitivo->atualiza_dados(cenario, inimigo, num_inimigos);
			exec++;
			falhas++;
			//printf("Total = %d \tSucesso = %f%% (%d) (f)\n", exec, ((float)sucesso/(float)exec)*100.00, sucesso);
			if (exec >= max_exec) {
				//fugitivo->savla_QValues(arq_saida);
				fprintf(arq, "%d\n", ((int)(((float)sucesso/(float)max_exec)*100.00)));
				exit(0);
			}
			///Restart
			reinicia();
		}
	}
	
	
	for (int i = 0; i < num_inimigos; i++) {
		if (!inimigo[i]->morto) {
			inimigo[i]->atualiza_posicao();
			inimigo[i]->atualiza_campo_visao();
			inimigo[i]->define_novo_destino( cenario, &fugitivo->p_destino );
			if (!inimigo[i]->alerta)
				inimigo[i]->ve_inimigo_morto (cenario, inimigo, num_inimigos);
		}
	}
	
	//if (fugitivo->existe_caminho) {
		fugitivo->atualiza_posicao();
	//}
	
	fugitivo->atualiza_dados(cenario, inimigo, num_inimigos);
	
	cont_passos++;
}
