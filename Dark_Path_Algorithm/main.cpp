#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h> 
#include <GL/glut.h>
#include "Lista.hpp"
#include "Fugitivo.hpp"
#include "Inimigo.hpp"
#include "Visual.hpp"


///Define Sistema Operacional
#ifdef _WIN32
	#define CLEAR "cls"
#elif defined __gnu_linux__
	#define CLEAR "clear"
#endif

#define TAM_CENARIO 500
#define MAX_PASSOS 180000

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
Fugitivo *fugitivo;
Inimigo **inimigo;

int sucesso;
int exec;
int max_exec;

int modo_exec;
bool imune;

extern int SEED_FIXA;

///Controla o número de pasos máximo permitido por simulação (para encerrar o programa caso ocorra algum problema)
int cont_passos;

///MAIN
int main(int argc, char** argv) {
	//srand (time(NULL));
	SEED_FIXA = 50;
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
	fprintf(stderr,"\t-h : Shows this menu\n");
	exit(0);
}

void mensagem_inicial () {
	printf("1 - Draws or hides the graph;\n");
	printf("2 - Draws or hides the navigation mesh;\n");
	printf("3 - Draws of hides the navigation meshe's path;\n");
	printf("4 - Draws of hides the closed nodes of the A*;\n");
	printf("5 - Draws of hides the exact path (B-Spline);\n");
	printf("6 - Draws of hides the the control points of the B-Spline;\n");
	printf("7 - Draws of hides the the control lines of the B-Spline;\n\n");
}

///GLUT: INIT
void init(int argc, char** argv) {
	glClearColor(1.0, 1.0, 1.0, 0.0);
	modo_exec = 0;
	max_exec = 100;
	sucesso = 0;
	exec = 0;
    num_inimigos = 3;
    imune = false;
    
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
			case 'h':
			default:
				usage(argv[0]);
		}
	}
	
	mensagem_inicial();
    	
    cenario = new Cenario(TAM_CENARIO);
    cenario->Cria_Paredes();
    cenario->insere_paredes_grid();
    
    double cpu_time_used;
    clock_t start, end;
    start = clock();
    cenario->Visibility_Algorithm();
    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("%d\t%f\n", TAM_GRID, cpu_time_used);
    
    cenario->encontra_maxima_exposicao();
    
    fugitivo = new Fugitivo(cenario);
    fugitivo->gera_posicao_inicial(cenario);
    inimigo = (Inimigo**) malloc (sizeof(Inimigo*)*num_inimigos);
    for (int i = 0; i < num_inimigos; i++) {
		inimigo[i] = new Inimigo(cenario);
		inimigo[i]->gera_posicao_inicial(cenario, fugitivo);
	}
    
    visual = new Visual();
    
    cont_passos = 0;
}

void reinicia() {
	delete cenario;
	cenario = new Cenario(TAM_CENARIO);
	cenario->Cria_Paredes();
	cenario->insere_paredes_grid();
	cenario->Visibility_Algorithm();
	for (int i = 0; i < TAM_GRID; i++) {
		for (int j = 0; j  < TAM_GRID; j++) {
			//printf("grid[%d][%d].exp_ambiente = %d\n", i, j, cenario->grid[i][j].exp_ambiente);
		}
	}
	cenario->encontra_maxima_exposicao();
	
	fugitivo->restaura_valores(cenario);
	fugitivo->gera_posicao_inicial(cenario);
	for (int i = 0; i < num_inimigos; i++) {
		inimigo[i]->restaura_valores(cenario);
		inimigo[i]->gera_posicao_inicial(cenario, fugitivo);
	}
	
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
		
		visual->desenha_cenario(cenario, fugitivo, inimigo, num_inimigos);
		
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
		float metade = (float)(cenario->tam_cell/(float)2.0);
		bool encontrado = false;
		for (int i = 0; i < TAM_GRID; i++) {
			for (int j = 0; j < TAM_GRID; j++) {
				if (p.x >= cenario->grid[i][j].x - metade && p.x <= cenario->grid[i][j].x + metade) {
					if (p.y >= cenario->grid[i][j].y - metade && p.y <= cenario->grid[i][j].y + metade) {
						if (cenario->grid[i][j].exp_ambiente != -1 && cenario->grid[i][j].exp_inimigo == 0) {
							encontrado = true;
							fugitivo->path.end.x = i;
							fugitivo->path.end.y = j;
							fugitivo->caminho = libera(fugitivo->caminho);
							break;
						}
					}
				}
			}
			if (encontrado)
				break;
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
		case 's':
		printf("para\n");
			if (fugitivo->path.velocidade > 0)
				fugitivo->altera_velocidade(false, inimigo, num_inimigos);
			else
				fugitivo->altera_velocidade(true, inimigo, num_inimigos);
				
			imune = !imune;
		break;
	}
}


///GLUT: MOUSE MOTION
void mouseMotion(int x, int y){
	
}

void idle () {
	int r;
	if (modo_exec == 1) {
		///Sucesso 
		if (fugitivo->path.terminou_caminho) {
			exec++;
			sucesso++;
			printf("Sucesso = %d (%d)\n", sucesso, exec);
			if (exec >= max_exec) {
				printf("sucesso = %d\n", sucesso);
				exit(0);
			}
			///Restart
			reinicia();
		}
		///Fracasso
		if (!imune && cenario->grid[(int)fugitivo->path.atual.x][(int)fugitivo->path.atual.y].exp_inimigo > 0) {
			exec++;
			printf("Sucesso = %d (%d)\n", sucesso, exec);
			if (exec >= max_exec) {
				printf("sucesso = %d\n", sucesso);
				exit(0);
			}
			///Restart
			reinicia();
		}
	}
	
	for (int i = 0; i < num_inimigos; i++) {
		r = inimigo[i]->recalcula;
		inimigo[i]->define_novo_destino(cenario);
		inimigo[i]->caminha(cenario);
		if (r == 0) {
			cenario->reinicia_visao_inimigo();
			for (int j = 0; j < num_inimigos; j++) 
				inimigo[j]->atualiza_campo_visao(cenario);
		}
	}
	
	if (cenario->recalcula == 0) {
		cenario->recalcula = TEMPO_RECALCULA;
		cenario->Dark_Path_Algortihm(cenario->grid, &fugitivo->path.end);
	}
	else
		cenario->recalcula--;
	
	if (!imune) {
		fugitivo->altera_velocidade(true, inimigo, num_inimigos);
		fugitivo->caminha(cenario);
	}
	
	cont_passos++;
	
	glutPostRedisplay();
}
