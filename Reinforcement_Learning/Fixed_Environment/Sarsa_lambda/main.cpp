#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h> 
#include <GL/glut.h>
#include "Lista.hpp"
#include "Bspline.hpp"
#include "Cenario.hpp"
#include "Visual.hpp"
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
#define MAX_PASSOS 70000

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
Visual *visual;
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

///MAIN
int main(int argc, char** argv) {
	
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(900, 700);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Stealth Simulator");

    glutDisplayFunc(display);
    glutIdleFunc(idle);
    glutReshapeFunc(reshape);

    glutMouseFunc(mouse);
    glutKeyboardFunc(keyboard);
    glutMotionFunc(mouseMotion);

    init(argc, argv);
    
    glutMainLoop();

    return 0;
}

int usage(char *str) {
	fprintf(stderr,"Usage: %s [-h] [-m] [-i] [-e]\n ", str);
	fprintf(stderr,"\t-m : Execution mode\n");
	fprintf(stderr,"\t\t * 0 : Executes only once with the graphical interface\n");
	fprintf(stderr,"\t\t * 1 : Executes E times without the graphical interface, where E represents the number of executions (-e)\n");
	fprintf(stderr,"\t-i : Number of enemies\n");
	fprintf(stderr,"\t-e : Number of executions\n");
	fprintf(stderr,"\t-a : Input file containing the initial Q-Table (opcional)\n");
	fprintf(stderr,"\t-s : Name of the output file (which will contain the resulting Q-Table)\n");
	frpintf(stderr,"\t-z : Learning rate (value between 0-1)\n");
	frpintf(stderr,"\t-x : Discount rate (value between 0-1)\n");
	frpintf(stderr,"\t-c : Exploration rate (value between 0-1)\n");
	fprintf(stderr,"\t-h : Shows this menu\n");
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
	glClearColor(1.0, 1.0, 1.0, 0.0);
	modo_exec = 0;
	max_exec = 100;
	sucesso = 0;
	falhas = 0;
	exec = 0;
    num_inimigos = 3;
    nome_arq = NULL;
    arq_saida = NULL;
    aprendizado = ALPHA_INICIAL;
    desconto = GAMMA_INICIAL;
    exploracao = PHI_INICIAL;
    
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
    
    visual = new Visual();
    
    cont_passos = 0;
}

void reinicia() {
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
	
	cont_passos = 0;
}


///GLUT: DISPLAY
void display(void) {
	if (modo_exec == 0) {
		glClear (GL_COLOR_BUFFER_BIT);
		glMatrixMode(GL_PROJECTION);              
		glLoadIdentity();
		glOrtho(-2, TAM_CENARIO+2, -2, TAM_CENARIO+2, -1.0, 1.0);	
		
		glMatrixMode(GL_MODELVIEW); 
		glLoadIdentity();           
		
		visual->desenha_cenario(cenario, fugitivo->grafo, fugitivo, inimigo, num_inimigos);
		
		glFlush ();
	}
}


///GLUT: RESHAPE
void reshape(int w, int h) {
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glOrtho(0.0f, (float)w, (float)h, 0.0f, -1.0f, 1.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}


///GLUT: MOUSE
void mouse(int button, int state, int x, int y) {
    if(button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		///Nada...
	}
	else if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
		fugitivo->perseguindo = false;
		fugitivo->furtivo = true;
		int temp;
		Ponto_Mapa p;
		///Converte as coordenadas em pixel (coordenadas da janela) em coordenadas do mundo
		GLdouble real_x, real_y, real_z;
		GLint viewport[4];
		GLdouble modelview[16];
		GLdouble projection[16];
		GLfloat winX, winY, winZ;

		glGetDoublev( GL_MODELVIEW_MATRIX, modelview );
		glGetDoublev( GL_PROJECTION_MATRIX, projection );
		glGetIntegerv( GL_VIEWPORT, viewport );

		winX = (float)x;
		winY = (float)viewport[3] - (float)y;
		glReadPixels( x, int(winY), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ );
		gluUnProject( winX, winY, winZ, modelview, projection, viewport, &real_x, &real_y, &real_z);
		
		p.x = real_x; 
		p.y = real_y;
		temp = fugitivo->grafo->encontra_mesh(&p);
		if (temp != -1) {
			fugitivo->caminho_errado = false;
			fugitivo->path.mesh_end = temp;
			fugitivo->path.end.x = p.x;
			fugitivo->path.end.y = p.y;
			fugitivo->p_destino.x = p.x;
			fugitivo->p_destino.y = p.y;
			fugitivo->existe_caminho = true;
			fugitivo->gera_caminho(cenario, inimigo, num_inimigos);
		}
	}
}


///GLUT: KEYBOARD
void keyboard(unsigned char key, int x, int y) {
	switch(key) {
		///TECLA 'ESC': SAIR
		case 27:
			exit(EXIT_SUCCESS);
		break;
		///Faz os agentes pararem de andar ou continuarem a andar
		case 's':
			fugitivo->furtivo = true;
			fugitivo->perseguindo = false;
			if (fugitivo->path.velocidade == 0)
				fugitivo->path.velocidade = MODULO_VELOCIDADE_ANDA;
			else
				fugitivo->path.velocidade = 0;
			for (int i = 0; i < num_inimigos; i++) {
				if (inimigo[i]->path.velocidade == 0)
					inimigo[i]->path.velocidade = MODULO_VELOCIDADE_PATRULHA;
				else
					inimigo[i]->path.velocidade = 0;
			}
		break;
		///Alterna entre os modos de caminhamento continuo ou entre coberturas
		case 'c':
			fugitivo->furtivo = true;
			fugitivo->perseguindo = false;
			if (fugitivo->path.end.x != fugitivo->p_destino.x) 
				fugitivo->restaura_caminho(cenario, inimigo, num_inimigos);
			fugitivo->entre_coberturas = !fugitivo->entre_coberturas;
			if (fugitivo->entre_coberturas) {
				fugitivo->encontra_prox_cobertura();
			}
			else
				fugitivo->path.mesh_objetivo = fugitivo->path.mesh_end;
		break;
		///Faz o fugitivo andar para a próxima cobertura
		case 'q':
			fugitivo->furtivo = true;
			fugitivo->perseguindo = false;
			if (fugitivo->entre_coberturas) {
				if (fugitivo->path.end.x != fugitivo->p_destino.x) 
					fugitivo->restaura_caminho(cenario, inimigo, num_inimigos);
				fugitivo->encontra_prox_cobertura();
			}
		break;
		///Esconde
		case 'p':
			fugitivo->furtivo = true;
			fugitivo->perseguindo = false;
			fugitivo->entre_coberturas = false;
			fugitivo->encontra_esconderijo(cenario, inimigo, num_inimigos);
		break;
		///Persegue inimigo
		case 'w':
			fugitivo->persegue_mais_proximo(cenario, inimigo, num_inimigos);
		break;
		///Mata inimigo
		case 'o':
			fugitivo->matar_inimigo_prox (cenario, inimigo, num_inimigos);
		break;
		///Distrai inimigos fazendo um barulho
		case 'b':
			fugitivo->faz_barulho (cenario, inimigo, num_inimigos);
		break;
		case '1':
			visual->visualiza_grafo = !visual->visualiza_grafo;
		break;
		case '2':
			visual->visualiza_mesh = !visual->visualiza_mesh;
		break;
		case '3':
			visual->visualiza_caminho_mesh = !visual->visualiza_caminho_mesh;
		break;
		case '4':
			visual->visualiza_nodos_fechados = !visual->visualiza_nodos_fechados;
		break;
		case '5':
			visual->visualiza_caminho_exato = !visual->visualiza_caminho_exato;
		break;
		case '6':
			visual->visualiza_pontos_controle = !visual->visualiza_pontos_controle;
		break;
		case '7':
			visual->visualiza_linhas_controle = !visual->visualiza_linhas_controle;
		break;
	}
}


///GLUT: MOUSE MOTION
void mouseMotion(int x, int y){
	
}

void idle () {
	if (cont_passos > MAX_PASSOS) {
		//printf("Reiniciando\n");
		fugitivo->encontrado = true;
		fugitivo->atualiza_dados(cenario, inimigo, num_inimigos);
		exec++;
		falhas++;
		//printf("Total = %d \tSucesso = %f%% (%d) (f)\n", exec, ((float)sucesso/(float)exec)*100.00, sucesso);
		printf("%d\t%d\n", exec, sucesso);
		if (exec >= max_exec) {
			fugitivo->savla_QValues(arq_saida);
			printf("%f\n", ((float)sucesso/(float)max_exec)*100.00);
			exit(0);
		}
		reinicia();
	}
	else if (fugitivo->chegou_destino) {
		exec++;
		sucesso++;
		//printf("Total = %d \tSucesso = %f%% (%d) (f)\n", exec, ((float)sucesso/(float)exec)*100.00, sucesso);
		printf("%d\t%d\n", exec, sucesso);
		if (exec >= max_exec) {
			fugitivo->savla_QValues(arq_saida);
			printf("%f\n", ((float)sucesso/(float)max_exec)*100.00);
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
		printf("%d\t%d\n", exec, sucesso);
		if (exec >= max_exec) {
			fugitivo->savla_QValues(arq_saida);
			printf("%f\n", ((float)sucesso/(float)max_exec)*100.00);
			exit(0);
		}
		///Restart
		reinicia();
	}
	
	
	for (int i = 0; i < num_inimigos; i++) {
		if (!inimigo[i]->morto) {
			inimigo[i]->atualiza_posicao();
			inimigo[i]->atualiza_campo_visao();
			inimigo[i]->define_novo_destino( cenario);
			if (!inimigo[i]->alerta)
				inimigo[i]->ve_inimigo_morto (cenario, inimigo, num_inimigos);
		}
	}
	
	//if (fugitivo->existe_caminho) {
		fugitivo->atualiza_posicao();
	//}
	
	fugitivo->atualiza_dados(cenario, inimigo, num_inimigos);
	
	cont_passos++;
	
	glutPostRedisplay();
}
