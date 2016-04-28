#include <stdio.h>
#include <stdlib.h>
#include <time.h> 
#include <math.h>       
#include "Cenario.hpp"

int SEED_FIXA;

bool impressao;

float minimo (float a1, float a2, float a3, float a4, float a5) {
	float m = a1;
	if (m > a2) 
		m = a2;
	if (m > a3) 
		m = a3;
	if (m > a4) 
		m = a4;
	if (m > a5) 
		m = a5;
		
	return m;
}
//**********************************************************************
//**********************************************************************
//**********************************************************************
void Poligono::Print_pontos() {
	for (int i = 0; i < 4; i++) {
		printf("(%f, %f)\t", this->pontos[i].x, this->pontos[i].y);
	}
	printf("\n");
}
//**********************************************************************
//**********************************************************************
//**********************************************************************
Parede::Parede(float x, float y, float dimx, float dimy, float rot) {
	this->x = x;
	this->y = y;
	this->dx = dimx;
	this->dy = dimy;
	this->rotacao = rot; ///Em graus
	
	float dx1 = cos(this->rotacao * PI / 180.0) * dx;
	float dx2 = sin(this->rotacao * PI / 180.0) * dx;
	float dy1 = sin(this->rotacao * PI / 180.0) * dy;
	float dy2 = cos(this->rotacao * PI / 180.0) * dy;
	
	float metade_lado_1 = (dx1 + dy1) / 2.0;
	float metade_lado_2 = (dx2 + dy2) / 2.0;

	pontos[0].x = x - metade_lado_1;	pontos[0].y = y - metade_lado_2 + dy2;
	pontos[1].x = pontos[0].x + dx1;	pontos[1].y = y + metade_lado_2;
	pontos[2].x = pontos[0].x + dy1;	pontos[2].y = y - metade_lado_2;
	pontos[3].x = x + metade_lado_1;	pontos[3].y = pontos[2].y + dx2;
}
//----------------------------------------------------------------------
Parede::~Parede() {
	
}
//----------------------------------------------------------------------
bool Parede::Verifica_valido (float tam) {
	float thresh = 0.05*tam;

	for (int i = 0; i < 4; i++) {
		if (pontos[i].x - thresh < 0 || pontos[i].x + thresh > tam) {
			//printf("FALHA NA PAREDE:\n");
			//this->Print_pontos();
			return false;
		}
		if (pontos[i].y - thresh < 0 || pontos[i].y + thresh > tam) {
			//printf("FALHA NA PAREDE:\n");
			//this->Print_pontos();
			return false;
		}
	}
	return true;
}
//**********************************************************************
//**********************************************************************
//**********************************************************************
Cenario::Cenario(float tam) {
	srand (SEED_FIXA);
	impressao = true;
	this->tamanho = tam;
	this->tam_cell = (float)this->tamanho / (float)TAM_GRID;
	this->grid = (Grid_Cell**) malloc (sizeof(Grid_Cell*)*TAM_GRID);
	for (int i = 0; i < TAM_GRID; i++) {
		this->grid[i] = (Grid_Cell*) malloc (sizeof(Grid_Cell)*TAM_GRID);
		for (int j = 0; j  < TAM_GRID; j++) {
			this->grid[i][j].x = j*this->tam_cell + (float)(this->tam_cell/(float)2.0);
			this->grid[i][j].y = i*this->tam_cell + (float)(this->tam_cell/(float)2.0);
			this->grid[i][j].exp_ambiente = 0;
			this->grid[i][j].exp_inimigo = 0;
			this->grid[i][j].dt = 0.0;
			this->grid[i][j].dp = 0.0;
		}
	}
	
	this->num_parede = MIN_PAREDE + (rand() % (int)(this->tamanho * (float) PCT_NUM_PAREDES));
	paredes = (Parede**) malloc (sizeof(Parede*)*this->num_parede);
	for (int i = 0; i < this->num_parede; i++)
		paredes[i] = NULL;
		
	this->recalcula = 0;
}
//----------------------------------------------------------------------
Cenario::~Cenario() {
	for (int i = 0; i < this->num_parede; i++)
		delete this->paredes[i];
	free(this->paredes);
	for (int i = 0; i < TAM_GRID; i++) 
		free(this->grid[i]);
	free(this->grid);
}
//----------------------------------------------------------------------
void Cenario::Cria_Paredes() {
	float x, y, dx, dy, rot;
	int j = 0;
	srand (SEED_FIXA);
	
	for (int i = 0; i < this->num_parede; i++) {
		dx = MIN_LARGURA_PAREDE + (rand() % (int)(this->tamanho * PCT_MAX_PAREDE));
		dy = MIN_LARGURA_PAREDE + (rand() % (int)(this->tamanho * PCT_MAX_PAREDE));
		rot = rand() % 91;
		if (rot < 30 || rot > 60)
			rot = 0;
		x = dx/2.0 + (rand() % (int)(this->tamanho - dx));
		y = dy/2.0 + (rand() % (int)(this->tamanho - dy));
		
		paredes[j] = new Parede(x, y, dx, dy, rot);
		if (!this->Verifica_intersecao() || !paredes[j]->Verifica_valido(this->tamanho))  {
			delete paredes[j];
			paredes[j] = NULL;
		}
		else {
			//paredes[j]->Print_pontos();
			j++;
		}
	}
	
	this->num_parede = j;
}
//----------------------------------------------------------------------
void Cenario::insere_paredes_grid () {
	float xmin, xmax, ymin, ymax, x, y;
	for (int p = 0; p < this->num_parede; p++) {
		///Encontra os pontos extremos de uma parede
		xmin = this->tamanho; ymin = this->tamanho; xmax = 0; ymax = 0;
		for (int j = 0; j < 4; j++) {
			if (paredes[p]->pontos[j].x >= xmax) 
				xmax = paredes[p]->pontos[j].x;
			if (paredes[p]->pontos[j].x <= xmin) 
				xmin = paredes[p]->pontos[j].x;
			if (paredes[p]->pontos[j].y >= ymax) 
				ymax = paredes[p]->pontos[j].y;
			if (paredes[p]->pontos[j].y <= ymin) 
				ymin = paredes[p]->pontos[j].y;
		}
		///Identifica quais células da grid pertencem à parede p
		for (int i = 0; i < TAM_GRID; i++) {
			y = i*this->tam_cell + (float)(this->tam_cell/(float)2.0); 
			if (y >= ymin && y <= ymax) {
				for (int j = 0; j < TAM_GRID; j++) {
					x = j*this->tam_cell + (float)(this->tam_cell/(float)2.0); 
					if (x >= xmin && x <= xmax) {
						Ponto_Mapa centro_celula;
						centro_celula.x = x; centro_celula.y = y;
						if (verifica_dentro_poligono(paredes[p]->pontos, &centro_celula)) {
							this->grid[i][j].exp_ambiente = -1;
						}
					}
				}
			}
		}
	}
}
//----------------------------------------------------------------------
bool Cenario::Verifica_intersecao () {
	for (int indice_parede = 0; indice_parede < this->num_parede && this->paredes[indice_parede] != NULL; indice_parede++) {
		for (int i = 0; i < this->num_parede && this->paredes[i] != NULL; i++) {
			float delta = 7.5, minX_1 = this->tamanho, minY_1 = this->tamanho, maxX_1 = 0, maxY_1 = 0;
			float minX_2 = this->tamanho, minY_2 = this->tamanho, maxX_2 = 0, maxY_2 = 0;
			if (i != indice_parede) {
				///Encontra os pontos extremos de uma parede
				for (int j = 0; j < 4; j++) {
					if (paredes[i]->pontos[j].x >= maxX_1) 
						maxX_1 = paredes[i]->pontos[j].x;
					if (paredes[i]->pontos[j].x <= minX_1) 
						minX_1 = paredes[i]->pontos[j].x;
					if (paredes[i]->pontos[j].y >= maxY_1) 
						maxY_1 = paredes[i]->pontos[j].y;
					if (paredes[i]->pontos[j].y <= minY_1) 
						minY_1 = paredes[i]->pontos[j].y;
						
					if (paredes[indice_parede]->pontos[j].x >= maxX_2) 
						maxX_2 = paredes[indice_parede]->pontos[j].x;
					if (paredes[indice_parede]->pontos[j].x <= minX_2) 
						minX_2 = paredes[indice_parede]->pontos[j].x;
					if (paredes[indice_parede]->pontos[j].y >= maxY_2) 
						maxY_2 = paredes[indice_parede]->pontos[j].y;
					if (paredes[indice_parede]->pontos[j].y <= minY_2) 
						minY_2 = paredes[indice_parede]->pontos[j].y;
				}
				
				///Verifica se a parede atual está dentro de outra
				if ((minX_2-delta >= minX_1-delta && minX_2-delta <= maxX_1+delta) || (maxX_2+delta >= minX_1-delta && maxX_2+delta <= maxX_1+delta)) {
					if ((minY_2-delta >= minY_1-delta && minY_2-delta <= maxY_1+delta) || (maxY_2+delta >= minY_1-delta && maxY_2+delta <= maxY_1+delta)) {
						//printf("INTERSECAO:\n");
						return false;
					}
					else if ((minY_1-delta >= minY_2-delta && minY_1-delta <= maxY_2+delta) || (maxY_1+delta >= minY_2-delta && maxY_1+delta <= maxY_2+delta)) {
						return false;
					}
				}
			}
		}
	}
	
	return true;
}
//----------------------------------------------------------------------
void Cenario::encontra_maxima_exposicao () {
	this->max_exposicao = 0;
	for (int i = 0; i < TAM_GRID; i++) {
		for (int j = 0; j < TAM_GRID; j++) {
			if (this->grid[i][j].exp_ambiente > this->max_exposicao)
				this->max_exposicao = this->grid[i][j].exp_ambiente;
		}
	}
}
//----------------------------------------------------------------------
void Cenario::reinicia_visao_inimigo () {
	for (int i = 0; i < TAM_GRID; i++) {
		for (int j = 0; j  < TAM_GRID; j++) {
			this->grid[i][j].exp_inimigo = 0;
		}
	}
}
//----------------------------------------------------------------------
void Cenario::Distance_Transform (Grid_Cell **g) {
	bool alterado = true;
	float m;
	while (alterado) {
		alterado = false;
		///Forward Pass
		for (int i = 1; i < TAM_GRID; i++) {
			for (int j = 1; j < TAM_GRID; j++) {
				if (g[i][j].exp_ambiente != -1) {
					if (j < TAM_GRID - 1) {
						m = minimo(	g[i][j-1].dt + CUSTO_RETO, 
									g[i-1][j-1].dt + CUSTO_DIAGONAL, 
									g[i-1][j].dt + CUSTO_RETO, 
									g[i-1][j+1].dt + CUSTO_DIAGONAL, 
									g[i][j].dt);
					}
					else {
						m = minimo(	g[i][j-1].dt + CUSTO_RETO, 
									g[i-1][j-1].dt + CUSTO_DIAGONAL, 
									g[i-1][j].dt + CUSTO_RETO, 
									g[i][j].dt, 
									g[i][j].dt);
					}
					if (m != g[i][j].dt) {
						alterado = true;
						g[i][j].dt = m;
					}
				}
			}
		}
		///Backward Pass
		for (int i = TAM_GRID-2; i >= 0; i--) {
			for (int j = TAM_GRID-2; j >= 0; j--) {
				if (g[i][j].exp_ambiente != -1) {
					if (j > 0) {
						m = minimo(	g[i][j+1].dt + CUSTO_RETO, 
									g[i+1][j+1].dt + CUSTO_DIAGONAL, 
									g[i+1][j].dt + CUSTO_RETO, 
									g[i+1][j-1].dt + CUSTO_DIAGONAL, 
									g[i][j].dt);
					}
					else {
						m = minimo(	g[i][j+1].dt + CUSTO_RETO, 
									g[i+1][j+1].dt + CUSTO_DIAGONAL, 
									g[i+1][j].dt + CUSTO_RETO, 
									g[i][j].dt, 
									g[i][j].dt);
					}
					if (m != g[i][j].dt) {
						alterado = true;
						g[i][j].dt = m;
					}
				}
			}
		}
	}
}
//----------------------------------------------------------------------
void Cenario::inicializa_grids (Grid_Cell **g1, Grid_Cell **g2, int a, int b) {
	for (int i = 0; i < TAM_GRID; i++) {
		for (int j = 0; j < TAM_GRID; j++) {
			if (a == i && b == j) {
				g1[i][j].dt = 0.0;
				g2[i][j].dt = 0.0;
			}
			else {
				g1[i][j].dt = TAM_GRID*TAM_GRID + VALOR_GRANDE;
				g2[i][j].dt = TAM_GRID*TAM_GRID + VALOR_GRANDE;
			}
		}
	}
}
//----------------------------------------------------------------------
void Cenario::inicializa_grid_principal (int a, int b) {
	for (int i = 0; i < TAM_GRID; i++) {
		for (int j = 0; j < TAM_GRID; j++) {
			if (a == i && b == j) 
				this->grid[i][j].dp = this->grid[i][j].exp_ambiente;
			else 
				this->grid[i][j].dp = TAM_GRID*TAM_GRID + VALOR_GRANDE;
		}
	}
}
//----------------------------------------------------------------------
void Cenario::inicializa_grid_inimigo (Grid_Cell **g, int a, int b) {
	for (int i = 0; i < TAM_GRID; i++) {
		for (int j = 0; j < TAM_GRID; j++) {
			if (a == i && b == j) 
				g[i][j].dt = 0;
			else 
				g[i][j].dt = TAM_GRID*TAM_GRID + VALOR_GRANDE;
		}
	}
}
//----------------------------------------------------------------------
void Cenario::Visibility_Algorithm () {
	Grid_Cell **g1, **g2;
	g1 = (Grid_Cell**) malloc (sizeof(Grid_Cell*)*TAM_GRID);
	g2 = (Grid_Cell**) malloc (sizeof(Grid_Cell*)*TAM_GRID);
	for (int i = 0; i < TAM_GRID; i++) {
		g1[i] = (Grid_Cell*) malloc (TAM_GRID*sizeof(Grid_Cell));
		g2[i] = (Grid_Cell*) malloc (TAM_GRID*sizeof(Grid_Cell));
		for (int j = 0; j < TAM_GRID; j++) {
			g1[i][j].exp_ambiente = this->grid[i][j].exp_ambiente;
			g2[i][j].exp_ambiente = 0;
		}
	}
	
	for (int i = 0; i < TAM_GRID; i++) {
		//printf("Calculando para i = %d\n", i);
		for (int j = 0; j < TAM_GRID; j++) {
			if (this->grid[i][j].exp_ambiente != -1) {
				///Inicializa as distâncias para TAM_GRID*TAM_GRID + VALOR_GRANDE e o destino para 0
				this->inicializa_grids(g1, g2, i, j);
				///Calcula as distâncias de todos os pontos ao ponto (i,j) considerando obstáculos (g1) e sem obstáculos (g2)
				this->Distance_Transform(g1);
				this->Distance_Transform(g2);
				///Compara as grids g1 e g2 para ver quantos pontos podem ver o ponto (i, j)
				for (int a = 0; a < TAM_GRID; a++) {
					for (int b = 0; b < TAM_GRID; b++) {
						//printf("%f\n", g2[a][b].dt / g1[a][b].dt);
						if (g2[a][b].dt / g1[a][b].dt > COEF_VISIBILIDADE) 
							this->grid[i][j].exp_ambiente++;
					}
				}
			}
		}
	}
	
	for (int i = 0; i < TAM_GRID; i++) {
		free(g1[i]);
		free(g2[i]);
	}
	free(g1);
	free(g2);
}
//----------------------------------------------------------------------
void Cenario::Dark_Path_Algortihm (Grid_Cell **g, Ponto_Mapa *destino) {
	bool alterado = true;
	float m, v, dist;
	
	this->inicializa_grid_principal((int)destino->x, (int)destino->y);
	
	while (alterado) {
		alterado = false;
		for (int i = 0; i < TAM_GRID; i++) {
			for (int j = 0; j < TAM_GRID; j++) {
				///Analisa apenas células livres
				if (g[i][j].exp_ambiente != -1 && g[i][j].exp_inimigo == 0) {
					///Analisa os 8 vizinhos
					m = g[i][j].dp;
					for (int y = -1; y <= 1; y++) {
						if (i+y >= 0 && i+y < TAM_GRID) {
							for (int x = -1; x <= 1; x++) {
								if (j+x >= 0 && j+x < TAM_GRID) {
									///Obtém o valor de visualização do vizinho (x, y)
									if (g[i+y][j+x].exp_ambiente == -1 || g[i+y][j+x].exp_inimigo > 0)
										v = TAM_GRID*TAM_GRID + VALOR_GRANDE;
									else
										v = g[i+y][j+x].exp_ambiente;
									///Identifica a distância do vizinho (x, y) com a célula atual
									if (x == 0 || y == 0)
										dist = CUSTO_RETO;
									else
										dist = CUSTO_DIAGONAL;
									///Obtém o menor valor
									if (m > g[i+y][j+x].dp + COEF_DIST*dist + COEF_FURTIVIDADE*v) {
										m = g[i+y][j+x].dp + COEF_DIST*dist + COEF_FURTIVIDADE*v;
									}
								}
							}
						}
					}
					if (m != g[i][j].dp) {
						alterado = true;
						g[i][j].dp = m;
					}
				}
			}
		}
	}
}



































