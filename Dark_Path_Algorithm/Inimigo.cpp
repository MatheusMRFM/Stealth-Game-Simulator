#include <stdio.h>
#include <stdlib.h>
#include <math.h> 
#include "Inimigo.hpp"

#define TAM_VETOR_DIRECAO 10

Inimigo::Inimigo(Cenario *cenario) {
	///Gera valores entre -1 e 1 para o x e y da direcao
	float x, y, norma;
	x = rand() % 199 + 1;
	y = rand() % 200;
	x = (float)(x/100.0) - 1;
	y = (float)(y/100.0) - 1;
	norma = sqrt(x*x + y*y);
	x = x/(float)norma * TAM_VETOR_DIRECAO;
	y = y/(float)norma * TAM_VETOR_DIRECAO;
	
	this->path.direcao.x = x;
	this->path.direcao.y = y;
	this->tam_visao = TAM_VISAO_INICIAL;
	this->angulo_visao = ANGULO_VISAO;
	this->estado_visao = NEUTRO;
	this->path.velocidade = MODULO_VELOCIDADE_PATRULHA;
	this->recalcula = cenario->tam_cell / (float)this->path.velocidade;
	this->espera = 0;
	this->path.terminou_caminho = true;
	///Inicializa a grid
	this->grid = (Grid_Cell**) malloc (sizeof(Grid_Cell*)*TAM_GRID);
	for (int i = 0; i < TAM_GRID; i++) {
		this->grid[i] = (Grid_Cell*) malloc (sizeof(Grid_Cell)*TAM_GRID);
		for (int j = 0; j < TAM_GRID; j++) {
			this->grid[i][j].exp_ambiente = cenario->grid[i][j].exp_ambiente;
			this->grid[i][j].dt = 0.0;
		}
	}
}

Inimigo::~Inimigo() {
	
}

void Inimigo::restaura_valores (Cenario* cenario) {
	this->espera = 0;
	this->path.velocidade = MODULO_VELOCIDADE_PATRULHA;
	this->recalcula = cenario->tam_cell / (float)this->path.velocidade;
	this->path.terminou_caminho = true;
	for (int i = 0; i < TAM_GRID; i++) {
		for (int j = 0; j < TAM_GRID; j++) {
			this->grid[i][j].exp_ambiente = cenario->grid[i][j].exp_ambiente;
			this->grid[i][j].dt = 0.0;
		}
	}
}
//**********************************************************************
void Inimigo::gera_posicao_inicial (Cenario *cenario, Agente *fugitivo) {
	bool fim = false;
	float dx, dy, dist;
	while (!fim) {
		this->path.start.x = rand() % TAM_GRID;
		this->path.start.y = rand() % TAM_GRID;
		this->path.atual.x = this->path.start.x;
		this->path.atual.y = this->path.start.y;
		dx = cenario->grid[(int)this->path.atual.x][(int)this->path.atual.y].x - cenario->grid[(int)fugitivo->path.atual.x][(int)fugitivo->path.atual.y].x;
		dy = cenario->grid[(int)this->path.atual.x][(int)this->path.atual.y].y - cenario->grid[(int)fugitivo->path.atual.x][(int)fugitivo->path.atual.y].y;
		dist = sqrt(dx*dx + dy*dy);
		if (cenario->grid[(int)this->path.start.x][(int)this->path.start.y].exp_ambiente != -1 && dist > DIST_AGENTE_INICIAL)
			fim = true;
	}
	
	this->atualiza_campo_visao(cenario);
	this->path.atual.x = this->path.start.x;
	this->path.atual.y = this->path.start.y;
	this->path.end.x = this->path.start.x;
	this->path.end.y = this->path.start.y;
}
//**********************************************************************
Ponto_Mapa* Inimigo::encontra_prox_ponto (Cenario *cenario) {
	int y = (int)this->path.atual.x;
	int x = (int)this->path.atual.y;
	float valor = -1;
	Ponto_Mapa *p = (Ponto_Mapa*) malloc (sizeof(Ponto_Mapa));
	
	//printf("%f ----------------> ",  this->grid[y][x].dt);
	for (int i = -1; i <= 1; i++) {
		if (y+i >= 0 && y+i < TAM_GRID) {
			for (int j = -1; j <= 1; j++) {
				if (x+j >= 0 && x+j < TAM_GRID) {
					if (valor > this->grid[y+i][x+j].dt || valor == -1) {
						valor = this->grid[y+i][x+j].dt;
						p->y = x+j;
						p->x = y+i;
					}
				}
			}
		}
	}
	//printf("%f    (%d)\n",  this->grid[(int)p->x][(int)p->y].dt, this->grid[(int)p->x][(int)p->y].exp_ambiente);
	
	return p;
}
//**********************************************************************
void Inimigo::altera_velocidade (bool anda) {
	if (anda) {
		this->path.estado_velocidade = ESTADO_SNEAK;
		this->path.velocidade = MODULO_VELOCIDADE_PATRULHA;
	}
	else {
		this->path.estado_velocidade = ESTADO_PARADO;
		this->path.velocidade = 0.0;
	}
}
//**********************************************************************
void Inimigo::atualiza_campo_visao (Cenario *cenario) {
	Vetor3D v1, v2;
	v1.x = this->path.direcao.x;
	v1.y = this->path.direcao.y;
	v1.z = 0;
	v2.x = 1;
	v2.y = 0;
	v2.z = 0;
	
	///Gera o triângulo que representa a visão do inimigo
	float phi = angulo_entre_vetores(&v1, &v2);
	float beta = phi - this->angulo_visao/2.0;
	if (this->path.direcao.y < 0) {
		beta = phi - PI/2.0 - this->angulo_visao/2.0;
	}
	float alpha = beta + this->angulo_visao;
	
	this->visao.pontos[0].x = cenario->grid[(int)this->path.atual.x][(int)this->path.atual.y].x;
	this->visao.pontos[0].y = cenario->grid[(int)this->path.atual.x][(int)this->path.atual.y].y;
	if (this->path.direcao.y >= 0) {
		this->visao.pontos[1].x = this->visao.pontos[0].x + this->tam_visao*cos(beta);		this->visao.pontos[1].y = this->visao.pontos[0].y + this->tam_visao*sin(beta);
		this->visao.pontos[2].x = this->visao.pontos[0].x + this->tam_visao*cos(alpha);		this->visao.pontos[2].y = this->visao.pontos[0].y + this->tam_visao*sin(alpha);
	}
	else {
		this->visao.pontos[1].x = this->visao.pontos[0].x - this->tam_visao*sin(beta);		this->visao.pontos[1].y = this->visao.pontos[0].y - this->tam_visao*cos(beta);
		this->visao.pontos[2].x = this->visao.pontos[0].x - this->tam_visao*sin(alpha);		this->visao.pontos[2].y = this->visao.pontos[0].y - this->tam_visao*cos(alpha);
	}
	
	///Identifica quais pontos da grid o inimigo consegue enxergar com sua visão
	bool ve = true;
	for (int i = 0; i < TAM_GRID; i++) {
		for (int j = 0; j < TAM_GRID; j++) {
			Ponto_Mapa celula;
			celula.x = cenario->grid[j][i].x;
			celula.y = cenario->grid[j][i].y;
			if (verifica_dentro_triangulo(this->visao.pontos, &celula)) {
				ve = true;
				for (int a = 0; a < cenario->num_parede; a++) {
					if (!enxerga_ponto_visao_ilimitada(cenario, &celula, cenario->paredes, a)) {
						ve = false;
						break;
					}
				}
				if (ve)
					cenario->grid[j][i].exp_inimigo++;
			}
		}
	}
}
//**********************************************************************
void Inimigo::define_novo_destino (Cenario *cenario) {
	if (this->path.terminou_caminho) {
		///O inimigo já terminou seu caminho anterior e está pronto para caminhar novamente
		if (this->espera == 0) {
			this->estado_visao = NEUTRO;
			this->espera = rand() % NUM_FRAMES_ESPERA;
			
			Ponto_Mapa *p = this->gera_posicao(cenario);
			this->path.end.x = p->x;
			this->path.end.y = p->y;
			//printf("Destino => (%d, %d) - %d\n", (int)p->x, (int)p->y, cenario->grid[(int)p->x][(int)p->y].exp_inimigo);
			cenario->inicializa_grid_inimigo(this->grid, (int)this->path.end.x, (int)this->path.end.y);
			cenario->Distance_Transform(this->grid);
			
			/*printf("Grid[%d][%d].dt = %f\n", (int)this->path.end.x, (int)this->path.end.y, this->grid[(int)this->path.end.x][(int)this->path.end.y].dt);
			int y = (int)this->path.end.x;
			int x = (int)this->path.end.y;
			for (int i = -1; i <= 1; i++) {
				if (y+i >= 0 && y+i < TAM_GRID) {
					for (int j = -1; j <= 1; j++) {
						if (x+j >= 0 && x+j < TAM_GRID) {
							printf("grid[%d][%d] = %f (%d, %d)\n", y+i, x+j, this->grid[y+i][x+j].dt, cenario->grid[y+i][x+j].exp_ambiente, cenario->grid[y+i][x+j].exp_inimigo);
						}
					}
				}
			}*/
			
			free(p);
			this->path.terminou_caminho = false;
			this->altera_velocidade(true);
		}
		///O inimigo ainda vai ficar parado
		else if (this->espera > 0) {
			this->altera_velocidade(false);
			this->espera--;
		}
	}
}
//**********************************************************************
bool Inimigo::enxerga_ponto_visao_ilimitada (Cenario *cenario, Ponto_Mapa *p, Parede **parede, int indice_parede) {
	bool l1, l2;
	Ponto_Mapa a;
	a.x = cenario->grid[(int)this->path.atual.x][(int)this->path.atual.y].x;
	a.y = cenario->grid[(int)this->path.atual.x][(int)this->path.atual.y].y;

	///Verifica se a parede em questão encontra-se entre o agente e o inimigo
	l1 = linhas_se_cruzam(&parede[indice_parede]->pontos[0], &parede[indice_parede]->pontos[3], p, &a);
	if (l1)		return false;
	l2 = linhas_se_cruzam(&parede[indice_parede]->pontos[1], &parede[indice_parede]->pontos[2], p, &a);
	if (l2)		return false;
	
	return true;
}
//**********************************************************************
void Inimigo::verifica_pertubacao (Cenario *cenario, Ponto_Mapa *p) {
	
}






















