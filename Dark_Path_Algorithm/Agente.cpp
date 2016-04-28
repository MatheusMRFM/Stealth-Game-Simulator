#include <stdio.h>
#include <stdlib.h>
#include <time.h> 
#include <math.h> 
#include "Agente.hpp"

extern int SEED_FIXA;

Agente::Agente() {
	this->path.direcao.z = 0;
	this->path.velocidade = 0.0;
	this->path.estado_velocidade = ESTADO_PARADO;
	this->path.terminou_caminho = false;
	this->recalcula = 0;
}

Agente::~Agente() {
	
}
//----------------------------------------------------------------------
void Agente::gera_posicao_inicial (Cenario *cenario) {
	
}

void Agente::restaura_valores () {
	
}
//----------------------------------------------------------------------
void Agente::inicializa_dados (bool limpa_pesos) {
	
}
//----------------------------------------------------------------------
Ponto_Mapa* Agente::gera_posicao (Cenario *cenario) {
	srand (SEED_FIXA);
	Ponto_Mapa *p = (Ponto_Mapa*) malloc (sizeof(Ponto_Mapa));
	bool fim = false;
	while (!fim) {
		p->x = rand() % TAM_GRID;
		p->y = rand() % TAM_GRID;
		if (cenario->grid[(int)p->x][(int)p->y].exp_ambiente != -1)
			fim = true;
	}
	
	return p;
}
//----------------------------------------------------------------------
Ponto_Mapa* Agente::encontra_prox_ponto (Cenario *cenario) {
	return NULL;
}
//----------------------------------------------------------------------
void Agente::caminha (Cenario *cenario) {
	if (this->recalcula == 0) {
		Ponto_Mapa *p = encontra_prox_ponto(cenario);
		if (this->path.atual.x == p->x && this->path.atual.y == p->y)
			this->path.terminou_caminho = true;
		else {
			this->path.direcao.x = cenario->grid[(int)p->x][(int)p->y].x - cenario->grid[(int)this->path.atual.x][(int)this->path.atual.y].x;
			this->path.direcao.y = cenario->grid[(int)p->x][(int)p->y].y - cenario->grid[(int)this->path.atual.x][(int)this->path.atual.y].y;
		}
		this->path.atual.x = p->x;
		this->path.atual.y = p->y;
		free(p);
		if (this->path.velocidade != 0)
			this->recalcula = cenario->tam_cell / (float)this->path.velocidade;
		else
			this->recalcula = cenario->tam_cell / MODULO_VELOCIDADE_ANDA;
	}
	else
		this->recalcula--;
}
//----------------------------------------------------------------------
void Agente::altera_velocidade (bool anda) {
	
}
//----------------------------------------------------------------------
void Agente::atualiza_posicao () {
	
}

























