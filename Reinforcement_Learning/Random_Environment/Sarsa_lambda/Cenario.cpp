#include <stdio.h>
#include <stdlib.h>
#include <time.h> 
#include <math.h>       
#include "Cenario.hpp"


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

Parede::~Parede() {
	
}

bool Parede::Verifica_valido (float tam) {
	float thresh = 2*LARGURA_MESH_COVER;

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
Mesh::Mesh(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, int t, int parede_pai) {
	this->tipo = t;
	this->pontos[0].x = (int)(10*x1) / 10.0;		this->pontos[0].y = (int)(10*y1) / 10.0;
	this->pontos[1].x = (int)(10*x2) / 10.0;		this->pontos[1].y = (int)(10*y2) / 10.0;
	this->pontos[2].x = (int)(10*x3) / 10.0;		this->pontos[2].y = (int)(10*y3) / 10.0;
	this->pontos[3].x = (int)(10*x4) / 10.0;		this->pontos[3].y = (int)(10*y4) / 10.0;
	this->parede_pertencente = parede_pai;
}

Mesh::Mesh(float x1, float y1, float x2, float y2, float x3, float y3, int t, int parede_pai) {
	this->tipo = t;
	this->pontos[0].x = (int)(10*x1) / 10.0;		this->pontos[0].y = (int)(10*y1) / 10.0;
	this->pontos[1].x = (int)(10*x2) / 10.0;		this->pontos[1].y = (int)(10*y2) / 10.0;
	this->pontos[2].x = (int)(10*x3) / 10.0;		this->pontos[2].y = (int)(10*y3) / 10.0;
	this->pontos[3].x = -1;		this->pontos[3].y = -1;
	this->parede_pertencente = parede_pai;
}
//**********************************************************************
//**********************************************************************
//**********************************************************************
NavMesh::NavMesh() {
	this->num_meshes = 0;
	this->meshes = (Mesh**) malloc (sizeof(Mesh*)*NUM_MESH_DEFAULT);
	for (int i = 0; i < NUM_MESH_DEFAULT; i++) 
		this->meshes[i] = NULL;
}

NavMesh::~NavMesh() {
	for (int i = 0; i < this->num_meshes; i++) {
		if (this->meshes[i] != NULL) 
			free(this->meshes[i]);
	}
			
	free(this->meshes);
}
//**********************************************************************
//**********************************************************************
//**********************************************************************
Cenario::Cenario(float tam) {
	frontWall = new Pilha();
	aberto = new Pilha();
	this->tamanho = tam;
	this->num_parede = MIN_PAREDE + (rand() % (int)(this->tamanho * (float) PCT_NUM_PAREDES));
	paredes = (Parede**) malloc (sizeof(Parede*)*this->num_parede);
	for (int i = 0; i < this->num_parede; i++)
		paredes[i] = NULL;
		
	this->erro = false;
}

Cenario::~Cenario() {
	for (int i = 0; i < this->num_parede; i++)
		delete paredes[i];
	free(paredes);
	
	delete nav_mesh;
	delete frontWall;
	delete aberto;
}

void Cenario::Cria_Paredes() {
	float x, y, dx, dy, rot;
	int j = 0;
	
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

bool Cenario::Verifica_intersecao () {
	for (int indice_parede = 0; indice_parede < this->num_parede && this->paredes[indice_parede] != NULL; indice_parede++) {
		for (int i = 0; i < this->num_parede && this->paredes[i] != NULL; i++) {
			float delta = 1.5*LARGURA_MESH_COVER, minX_1 = this->tamanho, minY_1 = this->tamanho, maxX_1 = 0, maxY_1 = 0;
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

void Cenario::Cria_Mesh_Cobertura () {
	int j = 0;	///define qual mesh esta sendo criada
	float dx1, dx2, dy1, dy2;
	for (int i = 0; i < this->num_parede && this->paredes[i] != NULL; i++) {
		if (this->nav_mesh->num_meshes + 8 >= NUM_MESH_DEFAULT) {
			this->erro = true;
			this->nav_mesh->num_meshes = NUM_MESH_DEFAULT;
			return;
		}
		float x1 = this->paredes[i]->pontos[0].x;
		float y1 = this->paredes[i]->pontos[0].y;
		float x2 = this->paredes[i]->pontos[1].x;
		float y2 = this->paredes[i]->pontos[1].y;
		float x3 = this->paredes[i]->pontos[2].x;
		float y3 = this->paredes[i]->pontos[2].y;
		float x4 = this->paredes[i]->pontos[3].x;
		float y4 = this->paredes[i]->pontos[3].y;
		
		dx1 = cos(this->paredes[i]->rotacao * PI / 180.0) * LARGURA_MESH_COVER;
		dx2 = sin(this->paredes[i]->rotacao * PI / 180.0) * LARGURA_MESH_COVER;
		dy1 = sin(this->paredes[i]->rotacao * PI / 180.0) * LARGURA_MESH_COVER;
		dy2 = cos(this->paredes[i]->rotacao * PI / 180.0) * LARGURA_MESH_COVER;
		
		this->nav_mesh->meshes[j] = new Mesh(	x1 - dx1 - dx2, 	y1 - dy1 + dy2, 
												x1 - dx2, 			y1 + dy2, 
												x1 - dx1, 			y1 - dy1, 
												x1, 				y1, 2, i);		
		j++;
		this->nav_mesh->num_meshes++;
		this->nav_mesh->meshes[j] = new Mesh(	x1 - dx2, 			y1 + dy2, 
												x2 - dx2, 			y2 + dy2, 
												x1, 				y1, 
												x2, 				y2, 1, i);
		j++;
		this->nav_mesh->num_meshes++;
		this->nav_mesh->meshes[j] = new Mesh(	x2 - dx2, 			y2 + dy2, 
												x2 - dx2 + dx1, 	y2 + dy1 + dy2, 
												x2, 				y2, 
												x2 + dx1, 			y2 + dy1, 2, i);
		j++;
		this->nav_mesh->num_meshes++;
		this->nav_mesh->meshes[j] = new Mesh(	x2, 				y2, 
												x2 + dx1, 			y2 + dy1, 
												x4, 				y4,  
												x4 + dx1, 			y4 + dy1, 1, i);
		j++;
		this->nav_mesh->num_meshes++;
		this->nav_mesh->meshes[j] = new Mesh(	x4, 				y4,  
												x4 + dx1, 			y4 + dy1,
												x4 + dx2,			y4 - dy2, 
												x4 + dx1 + dx2, 	y4 - dy2 + dy1, 2, i);
		j++;
		this->nav_mesh->num_meshes++;
		this->nav_mesh->meshes[j] = new Mesh(	x3,					y3,
												x4, 				y4,  
												x3 + dx2, 			y3 - dy2,
												x4 + dx2,			y4 - dy2, 1, i);
		j++;
		this->nav_mesh->num_meshes++;
		this->nav_mesh->meshes[j] = new Mesh(	x3 - dx1,			y3 - dy1,
												x3,					y3,
												x3 - dx1 + dx2, 	y3 - dy1 - dy2,
												x3 + dx2,			y3 - dy2, 2, i);
		j++;
		this->nav_mesh->num_meshes++;
		this->nav_mesh->meshes[j] = new Mesh(	x1 - dx1, 			y1 - dy1, 
												x1,					y1,
												x3 - dx1,			y3 - dy1,
												x3,					y3, 1, i);
		j++;
		this->nav_mesh->num_meshes++;
		
		///Cria as meshes em formato de triangulo (somente se rot > 0)
		if (this->paredes[i]->rotacao != 0) {
			if (this->nav_mesh->num_meshes + 4 >= NUM_MESH_DEFAULT) {
				this->erro = true;
				this->nav_mesh->num_meshes = NUM_MESH_DEFAULT;
				return;
			}
			float D_x2 = sin(this->paredes[i]->rotacao * PI / 180.0) * (this->paredes[i]->dy + 2*LARGURA_MESH_COVER);
			float D_y1 = sin(this->paredes[i]->rotacao * PI / 180.0) * (this->paredes[i]->dx + 2*LARGURA_MESH_COVER);

			x1 = x1 - dx1 - dx2;
			y1 = y1 - dy1 + dy2;
			x2 = x2 - dx2 + dx1;
			y2 = y2 + dy1 + dy2;
			x3 = x3 - dx1 + dx2;
			y3 = y3 - dy1 - dy2;
			x4 = x4 + dx1 + dx2;
			y4 = y4 + dy1 - dy2;

			this->nav_mesh->meshes[j] = new Mesh(	x1, 				y1, 
													x1,					y1 + D_y1,
													x2,					y2, 3, i);
			j++;
			this->nav_mesh->num_meshes++;
			this->nav_mesh->meshes[j] = new Mesh(	x2, 				y2, 
													x2 + D_x2,			y2,
													x4,					y4, 3, i);
			j++;
			this->nav_mesh->num_meshes++;
			this->nav_mesh->meshes[j] = new Mesh(	x4, 				y4, 
													x4,					y4 - D_y1,
													x3,					y3, 3, i);
			j++;
			this->nav_mesh->num_meshes++;
			this->nav_mesh->meshes[j] = new Mesh(	x3, 				y3, 
													x3 - D_x2,			y3,
													x1,					y1, 3, i);
			j++;
			this->nav_mesh->num_meshes++;
			
			frontWall->empilha(x1, y1 + D_y1, x3 - D_x2, y3);
			aberto->empilha(x2 + D_x2, y2, x4, y4 - D_y1);
		}
		else {
			frontWall->empilha(x1 - LARGURA_MESH_COVER, y1 + LARGURA_MESH_COVER, x3 - LARGURA_MESH_COVER, y3 - LARGURA_MESH_COVER);
			aberto->empilha(x2 + LARGURA_MESH_COVER, y2 + LARGURA_MESH_COVER, x4 + LARGURA_MESH_COVER, y4 - LARGURA_MESH_COVER);
		}
	}
}

void Cenario::cria_Mesh_quadrada (Elemento *fora, Elemento *aux) {
	Elemento temp1, temp2;
	float dx, dy;
	int n;
	///Cria Meshes sempre quadradas
	dx = aux->p1.x - fora->p1.x;
	dy = fora->p1.y - fora->p2.y;
	if (dx > DIFF_PERMETIDA*dy) {
		n = (int) (dx / (float)(DIFF_PERMETIDA*dy));
		if (this->nav_mesh->num_meshes + n + 1 >= NUM_MESH_DEFAULT) {
			this->erro = true;
			this->nav_mesh->num_meshes = NUM_MESH_DEFAULT;
			free(fora);
			return;
		}
		temp1.p1.x = fora->p1.x;	temp1.p1.y = fora->p1.y;
		temp1.p2.x = fora->p2.x;	temp1.p2.y = fora->p2.y;
		temp2.p1.y = temp1.p1.y;
		temp2.p2.y = temp1.p2.y;
		for (int i = 0; i < n; i++) {
			temp2.p1.x = temp1.p1.x + DIFF_PERMETIDA*dy;
			temp2.p2.x = temp2.p1.x;
			this->nav_mesh->meshes[this->nav_mesh->num_meshes] = new Mesh(	temp1.p2.x, 	temp1.p2.y,	
																			temp1.p1.x, 	temp1.p1.y,
																			temp2.p2.x,		temp2.p2.y,
																			temp2.p1.x,		temp2.p1.y, 0, -1);
			this->nav_mesh->num_meshes++;
			temp1.p1.x = temp2.p1.x;	temp1.p1.y = temp2.p1.y;
			temp1.p2.x = temp2.p2.x;	temp1.p2.y = temp2.p2.y;
		}
		if ((float)(n*DIFF_PERMETIDA*dy) != dx) { 
			this->nav_mesh->meshes[this->nav_mesh->num_meshes] = new Mesh(	temp2.p2.x, 	temp2.p2.y,
																			temp2.p1.x, 	temp2.p1.y,
																			aux->p1.x,		temp2.p2.y,	
																			aux->p1.x,		temp2.p1.y, 0, -1);
			this->nav_mesh->num_meshes++;
		}
		//else
			//printf("X - Não precisa de mais mesh!\n");
	}
	else if (dy > DIFF_PERMETIDA*dx) {
		n = (int) (dy / (float)(DIFF_PERMETIDA*dx));
		if (this->nav_mesh->num_meshes + n + 1 >= NUM_MESH_DEFAULT) {
			this->erro = true;
			this->nav_mesh->num_meshes = NUM_MESH_DEFAULT;
			free(fora);
			return;
		}
		temp1.p2.x = fora->p2.x;	temp1.p2.y = fora->p2.y;
		temp1.p1.x = fora->p1.x;	temp1.p1.y = temp1.p2.y + DIFF_PERMETIDA*dx;
		temp2.p1.x = aux->p1.x;		
		temp2.p2.x = aux->p2.x;	
		for (int i = 0; i < n; i++) {
			temp2.p1.y = temp1.p1.y;
			temp2.p2.y = temp1.p2.y;
			this->nav_mesh->meshes[this->nav_mesh->num_meshes] = new Mesh(	temp1.p2.x, 	temp1.p2.y,
																			temp1.p1.x, 	temp1.p1.y,
																			temp2.p2.x,		temp2.p2.y,	
																			temp2.p1.x,		temp2.p1.y, 0, -1);
			this->nav_mesh->num_meshes++;
			temp1.p2.y = temp1.p1.y;
			temp1.p1.y = temp1.p1.y + DIFF_PERMETIDA*dx;
		}
		if ((float)(n*DIFF_PERMETIDA*dx) != dy) {
			dx = dy - n*DIFF_PERMETIDA*dx;
			this->nav_mesh->meshes[this->nav_mesh->num_meshes] = new Mesh(	temp1.p2.x, 	temp1.p2.y,
																			fora->p1.x, 	fora->p1.y,
																			temp2.p1.x,		temp2.p1.y,	
																			aux->p1.x,		temp2.p1.y + dx, 0, -1);
			this->nav_mesh->num_meshes++;
		}
		//else
			//printf("Y - Não precisa de mais mesh!\n");
	}
	else {
		if (this->nav_mesh->num_meshes >= NUM_MESH_DEFAULT) {
			this->erro = true;
			this->nav_mesh->num_meshes = NUM_MESH_DEFAULT;
			free(fora);
			return;
		}
		this->nav_mesh->meshes[this->nav_mesh->num_meshes] = new Mesh(	fora->p2.x, 	fora->p2.y,
																		fora->p1.x, 	fora->p1.y,
																		aux->p1.x,		fora->p2.y,	
																		aux->p1.x,		fora->p1.y, 0, -1);
		this->nav_mesh->num_meshes++;
	}
}

void Cenario::Cria_Mesh_Restante () {
	///Empilha a linha principal
	this->aberto->empilha(0.0, this->tamanho, 0.0, 0.0);
	///Empilha o fundo do cenário como front wall
	this->frontWall->empilha(this->tamanho, this->tamanho, this->tamanho, 0.0);
	Elemento *aux, *fora;
	
	
	while (this->aberto->topo != NULL) {
		fora = this->aberto->desempilha();
		///Encontra parede mais próxima
		aux = this->frontWall->procura_menor_x(fora->p1.x, fora->p1.y, fora->p2.y);
		
		if (fora->p1.x < aux->p1.x) {
			///Cria a Mesh
			this->cria_Mesh_quadrada (fora, aux);
			if (this->erro)
				return;
			
			float valor_x, valor_y = fora->p1.y;
			///Empilha novas linhas para serem varridas
			while (aux) {
				if (aux->p1.y < valor_y) {
					this->aberto->empilha(aux->p1.x, valor_y, aux->p1.x, aux->p1.y);
				}
				valor_x = aux->p1.x;
				valor_y = aux->p2.y;
				aux = this->frontWall->procura_igual(fora->p1.x, fora->p1.y, fora->p2.y, valor_x);
			}
			this->frontWall->renova_pilha();
			if (valor_y > fora->p2.y) {
				this->aberto->empilha(valor_x, valor_y, valor_x, fora->p2.y);
			}
		}
		///Desempilha a linha analisada
		free(fora);
	}
}

void Cenario::Cria_NavMesh () {
	nav_mesh = new NavMesh();
	this->Cria_Mesh_Cobertura();
	if (!this->erro)
		this->Cria_Mesh_Restante();
	if (!this->erro)
		nav_mesh->meshes = (Mesh**) realloc (nav_mesh->meshes, sizeof(Mesh*)*nav_mesh->num_meshes);
}

















