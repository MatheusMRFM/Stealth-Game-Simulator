#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include "Visual.hpp"

Visual::Visual() {
	
}

Visual::~Visual() {
	
}
//----------------------------------------------------------------------
void Visual::desenha_bordas(Cenario *cenario) {
	glLineWidth((GLfloat)3.5);
	glColor3f (0.0, 0.0, 0.0);
	glBegin(GL_LINE_LOOP);
		glVertex2f(0, 0);
		glVertex2f(0, cenario->tamanho);
		glVertex2f(cenario->tamanho, cenario->tamanho);
		glVertex2f(cenario->tamanho, 0);
	glEnd();
}
//----------------------------------------------------------------------
void Visual::desenha_parede(Cenario *cenario) {
	glColor3f (0.0, 0.0, 0.0);
	glLineWidth((GLfloat)3.5);
	for (int i = 0; i < cenario->num_parede && cenario->paredes[i] != NULL; i++) {
		glBegin(GL_LINE_LOOP);
			glVertex2f (cenario->paredes[i]->pontos[0].x, cenario->paredes[i]->pontos[0].y);
			glVertex2f (cenario->paredes[i]->pontos[1].x, cenario->paredes[i]->pontos[1].y);
			glVertex2f (cenario->paredes[i]->pontos[3].x, cenario->paredes[i]->pontos[3].y);
			glVertex2f (cenario->paredes[i]->pontos[2].x, cenario->paredes[i]->pontos[2].y);
		glEnd();
	}
}
//----------------------------------------------------------------------
void Visual::desenha_grid (Cenario *cenario) {
	float metade = (float)(cenario->tam_cell/(float)2.0);
	float cor;
	for (int i = 0; i < TAM_GRID; i++) {
		for (int j = 0; j < TAM_GRID; j++) {
			if (cenario->grid[i][j].exp_ambiente == -1)
				glColor3f (0.0, 0.0, 0.0);
			else if (cenario->grid[i][j].exp_inimigo > 0) {
				glColor3f (1.0, 0.6, 0.6);
			}
			else if (cenario->max_exposicao > 0) {
				cor = (float)cenario->grid[i][j].exp_ambiente / ((float) cenario->max_exposicao);
				glColor3f (cor, cor, cor);
			}
			else {
				glColor3f (1.0, 1.0, 1.0);
			}
			glBegin(GL_QUADS);
				glVertex2f (cenario->grid[i][j].x - metade, cenario->grid[i][j].y - metade);
				glVertex2f (cenario->grid[i][j].x - metade, cenario->grid[i][j].y + metade);
				glVertex2f (cenario->grid[i][j].x + metade, cenario->grid[i][j].y + metade);
				glVertex2f (cenario->grid[i][j].x + metade, cenario->grid[i][j].y - metade);
			glEnd();
		}
	}
}
//----------------------------------------------------------------------
void Visual::desenha_fugitivo (Fugitivo *fugitivo, Cenario *cenario) {
	///Desenha o agente
	float x = cenario->grid[(int)fugitivo->path.atual.x][(int)fugitivo->path.atual.y].x;
	float y = cenario->grid[(int)fugitivo->path.atual.x][(int)fugitivo->path.atual.y].y;
	glPointSize((GLfloat)10.0f);
	glColor3f (1.0, 0.5, 0.5);
	glBegin(GL_POINTS);
		glVertex2f(x, y);
	glEnd();
	///Desenha o destino do fugitivo
	x = cenario->grid[(int)fugitivo->path.end.x][(int)fugitivo->path.end.y].x;
	y = cenario->grid[(int)fugitivo->path.end.x][(int)fugitivo->path.end.y].y;
	glPointSize((GLfloat)10.0f);
	glColor3f (0.0, 0.5, 0.0);
	glBegin(GL_POINTS);
		glVertex2f(x, y);
	glEnd();
	///Desenha o caminho do agente
	glBegin(GL_LINE_STRIP);
		Lista *p = fugitivo->caminho;
		while (p != NULL) {
			glVertex2f (cenario->grid[(int)p->x][(int)p->y].x, cenario->grid[(int)p->x][(int)p->y].y);
			p = p->prox;
		}
	glEnd();
}
//----------------------------------------------------------------------
void Visual::desenha_inimigos (Inimigo **inimigo, int num_inimigos, Cenario *cenario) {
	glPointSize((GLfloat)10.0f);
	float x, y;
	
	for (int i = 0; i < num_inimigos; i++) {
		x = cenario->grid[(int)inimigo[i]->path.atual.x][(int)inimigo[i]->path.atual.y].x;
		y = cenario->grid[(int)inimigo[i]->path.atual.x][(int)inimigo[i]->path.atual.y].y;
		glColor3f (0.0, 0.0, 0.7);
		glBegin(GL_POINTS);
			glVertex2f(x, y);
		glEnd();
		
		glColor3f (0.7, 0.0, 0.0);
		glBegin(GL_LINE_STRIP);
			glVertex2f(x, y);
			glVertex2f(x + inimigo[i]->path.direcao.x, y + inimigo[i]->path.direcao.y);
		glEnd();
		
		/*x = cenario->grid[(int)inimigo[i]->path.end.x][(int)inimigo[i]->path.end.y].x;
		y = cenario->grid[(int)inimigo[i]->path.end.x][(int)inimigo[i]->path.end.y].y;
		glColor3f (0.0, 1.0, 0.0);
		glBegin(GL_POINTS);
			glVertex2f(x, y);
		glEnd();
	
		if (inimigo[i]->estado_visao == VISUALIZA_AGENTE) {
			glLineWidth((GLfloat)4.5);
			glColor3f (1.0, 0.0, 0.0);
		}
		else {
			glLineWidth((GLfloat)2.0);
			glColor3f (0.0, 0.0, 0.7);
		}
		glBegin(GL_LINE_STRIP);
			glVertex2f (inimigo[i]->visao.pontos[0].x, inimigo[i]->visao.pontos[0].y);
			glVertex2f (inimigo[i]->visao.pontos[1].x, inimigo[i]->visao.pontos[1].y);
			glVertex2f (inimigo[i]->visao.pontos[2].x, inimigo[i]->visao.pontos[2].y);
			glVertex2f (inimigo[i]->visao.pontos[0].x, inimigo[i]->visao.pontos[0].y);
		glEnd();*/
	}
}
//----------------------------------------------------------------------
void Visual::desenha_cenario(Cenario *cenario, Fugitivo *fugitivo, Inimigo **inimigo, int num_inimigos) {
	this->desenha_bordas(cenario);
	this->desenha_grid(cenario);
	//this->desenha_parede(cenario);
	
	this->desenha_fugitivo(fugitivo, cenario);
	this->desenha_inimigos(inimigo, num_inimigos, cenario);
}








