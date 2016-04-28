#include <stdio.h>
#include <stdlib.h>
#include <math.h> 
#include <unistd.h>
#include <GL/glut.h>
#include "Fugitivo.hpp"

Fugitivo::Fugitivo(Cenario *cenario) {
	this->path.velocidade = MODULO_VELOCIDADE_ANDA;
	this->recalcula = cenario->tam_cell / (float)this->path.velocidade;
	this->path.terminou_caminho = false;
	this->caminho = inicializa();
}

Fugitivo::~Fugitivo() {
	
}

void Fugitivo::restaura_valores (Cenario* cenario) {
	this->path.velocidade = MODULO_VELOCIDADE_ANDA;
	this->recalcula = cenario->tam_cell / (float)this->path.velocidade;
	this->path.terminou_caminho = false;
	this->caminho = libera(this->caminho);
}
//----------------------------------------------------------------------
void Fugitivo::gera_posicao_inicial (Cenario *cenario) {
	Ponto_Mapa *p;
	p = this->gera_posicao(cenario);
	this->path.start.x = p->x;
	this->path.start.y = p->y;
	free(p);
	
	p = this->gera_posicao(cenario);
	this->path.end.x = p->x;
	this->path.end.y = p->y;
	free(p);
	
	this->p_destino.x = this->path.end.x;
	this->p_destino.y = this->path.end.y;
	this->path.atual.x = this->path.start.x;
	this->path.atual.y = this->path.start.y;
}
//----------------------------------------------------------------------
void Fugitivo::define_destino (Cenario *cenario) {
	Ponto_Mapa *p = this->gera_posicao(cenario);
	this->path.end.x = p->x;
	this->path.end.y = p->y;
	free(p);
}
//----------------------------------------------------------------------
void Fugitivo::altera_velocidade (bool anda, Inimigo **inimigo, int num_inimigos) {
	if (anda) {
		float dx, dy, dist;
		this->path.estado_velocidade = ESTADO_CORRE;
		this->path.velocidade = MODULO_VELOCIDADE_CORRE;
		
		for (int i = 0; i < num_inimigos; i++) {
			dx = this->path.atual.x - inimigo[i]->path.atual.x;
			dy = this->path.atual.y - inimigo[i]->path.atual.y;
			dist = sqrt(dx*dx + dy*dy);
			if (dist < RAIO_SOM_SNEAK) {
				//printf("Espera!\n");
				this->path.estado_velocidade = ESTADO_PARADO;
			}
			else if (dist < RAIO_SOM_ANDA && this->path.estado_velocidade >= ESTADO_SNEAK) {
				//printf("Sneak!\n");
				this->path.estado_velocidade = ESTADO_SNEAK;
				this->path.velocidade = MODULO_VELOCIDADE_SNEAK;
			}
			else if (dist < RAIO_SOM_CORRE && this->path.estado_velocidade >= ESTADO_ANDA) {
				//printf("\tAnda!\n");
				this->path.estado_velocidade = ESTADO_ANDA;
				this->path.velocidade = MODULO_VELOCIDADE_ANDA;
			}
			else if (this->path.estado_velocidade == ESTADO_CORRE) {
				//printf("\t\tCorre!\n");
				this->path.estado_velocidade = ESTADO_CORRE;
				this->path.velocidade = MODULO_VELOCIDADE_CORRE;
			}
		}
	}
	else {
		this->path.estado_velocidade = ESTADO_PARADO;
		this->path.velocidade = 0.0;
	}
}
//----------------------------------------------------------------------
Ponto_Mapa* Fugitivo::encontra_prox_ponto (Cenario *cenario) {
	int y = (int)this->path.atual.x;
	int x = (int)this->path.atual.y;
	float valor = -1;
	Ponto_Mapa *p = (Ponto_Mapa*) malloc (sizeof(Ponto_Mapa));
	
	//printf("%f ----------------> ",  cenario->grid[y][x].dp);
					
	for (int i = -1; i <= 1; i++) {
		if (y+i >= 0 && y+i < TAM_GRID) {
			for (int j = -1; j <= 1; j++) {
				if (x+j >= 0 && x+j < TAM_GRID) {
					//printf("%f - ", cenario->grid[y+j][x+i].dt);
					if (valor > cenario->grid[y+i][x+j].dp || valor == -1) {
						valor = cenario->grid[y+i][x+j].dp;
						p->y = x+j;
						p->x = y+i;
					}
				}
			}
		}
	}
	//printf("\n");
	//printf("%f    (%d, %d)\n",  cenario->grid[(int)p->x][(int)p->y].dp, cenario->grid[(int)p->x][(int)p->y].exp_ambiente, cenario->grid[(int)p->x][(int)p->y].exp_inimigo);
	
	this->caminho = insere(this->caminho, 0, p->x, p->y);
	
	return p;
}
//----------------------------------------------------------------------
Ponto_Mapa* Fugitivo::verifica_visao_inimigos (Cenario *cenario, Inimigo **inimigo, int num_inimigos, bool check_path) {
	return NULL;
}
//----------------------------------------------------------------------
void Fugitivo::faz_barulho_passo (Cenario *cenario, Inimigo **inimigo, int num_inimigos) {
	float dx, dy, dist;
	int raio_som;
	
	if (this->path.estado_velocidade == ESTADO_PARADO)
		raio_som = 0;
	else if (this->path.estado_velocidade == ESTADO_SNEAK) {
		raio_som = RAIO_SOM_SNEAK;
	}
	else if (this->path.estado_velocidade == ESTADO_ANDA) {
		raio_som = RAIO_SOM_ANDA;
	}
	else {
		raio_som = RAIO_SOM_CORRE;
	}
		
	if (raio_som > 0) {
		for (int i = 0; i < num_inimigos; i++) {
			dx = this->path.atual.x - inimigo[i]->path.atual.x;
			dy = this->path.atual.y - inimigo[i]->path.atual.y;
			dist = sqrt(dx*dx + dy*dy);
			if (dist <= raio_som && inimigo[i]->estado_visao != VISUALIZA_AGENTE) {
				printf("Barulho!\n");
				inimigo[i]->estado_visao = VERIFICA_BARULHO;
				//inimigo[i]->verifica_pertubacao(cenario, &this->path.atual, this->path.mesh_atual);
			}
		}
	}
}














