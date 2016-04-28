#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include "Visual.hpp"

Visual::Visual() {
	this->visualiza_grafo = false;
	this->visualiza_mesh = false;
	this->visualiza_caminho_mesh = false;
	this->visualiza_caminho_exato = true;
	this->visualiza_pontos_controle = false;
	this->visualiza_linhas_controle = false;
	this->visualiza_nodos_fechados = false;
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
		glBegin(GL_QUADS);
			glVertex2f (cenario->paredes[i]->pontos[0].x, cenario->paredes[i]->pontos[0].y);
			glVertex2f (cenario->paredes[i]->pontos[1].x, cenario->paredes[i]->pontos[1].y);
			glVertex2f (cenario->paredes[i]->pontos[3].x, cenario->paredes[i]->pontos[3].y);
			glVertex2f (cenario->paredes[i]->pontos[2].x, cenario->paredes[i]->pontos[2].y);
		glEnd();
	}
}
//----------------------------------------------------------------------
void Visual::desenha_meshes(Cenario *cenario) {
	glLineWidth((GLfloat)3.5);
	glColor3f (0.5, 0.5, 1.0);
	for (int i = 0; i < cenario->nav_mesh->num_meshes; i++) {
		/*if (cenario->nav_mesh->meshes[i]->tipo == 0)
			glColor3f (0.0, 0.0, 1.0);
		else if (cenario->nav_mesh->meshes[i]->tipo == 1)
			glColor3f (0.0, 1.0, 0.0);
		else if (cenario->nav_mesh->meshes[i]->tipo == 2)
			glColor3f (1.0, 0.0, 0.0);
		else
			glColor3f (0.0, 1.0, 1.0);*/
		if (cenario->nav_mesh->meshes[i]->pontos[3].x != -1) {
			glBegin(GL_LINE_LOOP);
			//glBegin(GL_QUADS);
				glVertex2f (cenario->nav_mesh->meshes[i]->pontos[0].x, cenario->nav_mesh->meshes[i]->pontos[0].y);
				glVertex2f (cenario->nav_mesh->meshes[i]->pontos[1].x, cenario->nav_mesh->meshes[i]->pontos[1].y);
				glVertex2f (cenario->nav_mesh->meshes[i]->pontos[3].x, cenario->nav_mesh->meshes[i]->pontos[3].y);
				glVertex2f (cenario->nav_mesh->meshes[i]->pontos[2].x, cenario->nav_mesh->meshes[i]->pontos[2].y);
			glEnd();
		}
		else {
			glBegin(GL_LINE_LOOP);
			//glBegin(GL_QUADS);
				glVertex2f (cenario->nav_mesh->meshes[i]->pontos[0].x, cenario->nav_mesh->meshes[i]->pontos[0].y);
				glVertex2f (cenario->nav_mesh->meshes[i]->pontos[1].x, cenario->nav_mesh->meshes[i]->pontos[1].y);
				glVertex2f (cenario->nav_mesh->meshes[i]->pontos[2].x, cenario->nav_mesh->meshes[i]->pontos[2].y);
			glEnd();
		}
	}
} 
//----------------------------------------------------------------------
void Visual::desenha_grafo(Grafo *grafo) {
	glColor3f (1.0, 0.0, 0.0);
	glPointSize((GLfloat)10.0f);
	glLineWidth((GLfloat)1.0);
	for (int i = 0; i < grafo->num_no-1; i++) {
		glBegin(GL_POINTS);
			glVertex2f(grafo->nodo[i]->x, grafo->nodo[i]->y);
		glEnd();
		for (int j = i+1; j < grafo->num_no; j++) {
			if (grafo->m_adj[i][j] != -1) {
				glBegin(GL_POINTS);
					glVertex2f(grafo->nodo[i]->x, grafo->nodo[i]->y);
					glVertex2f(grafo->nodo[j]->x, grafo->nodo[j]->y);
				glEnd();
				if (grafo->nodo[i]->mesh->tipo != 0 && grafo->nodo[j]->mesh->tipo != 0) {
					glBegin(GL_LINES);
						glVertex2f(grafo->nodo[i]->x, grafo->nodo[i]->y);
						glVertex2f(grafo->nodo[j]->x, grafo->nodo[j]->y);
					glEnd();
				}
				else {
					if (grafo->nodo[i]->mesh->tipo != 0 || grafo->nodo[j]->mesh->tipo != 0) {
						glBegin(GL_LINES);
							glVertex2f(grafo->nodo[i]->x, grafo->nodo[i]->y);
							glVertex2f(grafo->nodo[j]->x, grafo->nodo[j]->y);
						glEnd();
					}
					else if (grafo->nodo[j]->y <= grafo->nodo[i]->max_y && grafo->nodo[j]->y >= grafo->nodo[i]->min_y && (grafo->nodo[i]->max_x == grafo->nodo[j]->min_x || grafo->nodo[j]->max_x == grafo->nodo[i]->min_x)) {
						glBegin(GL_LINE_STRIP);
							glVertex2f(grafo->nodo[i]->x, grafo->nodo[i]->y);
							glVertex2f(grafo->nodo[i]->x, grafo->nodo[j]->y);
							glVertex2f(grafo->nodo[j]->x, grafo->nodo[j]->y);
						glEnd();
					}
					else {
						glBegin(GL_LINE_STRIP);
							glVertex2f(grafo->nodo[i]->x, grafo->nodo[i]->y);
							glVertex2f(grafo->nodo[j]->x, grafo->nodo[i]->y);
							glVertex2f(grafo->nodo[j]->x, grafo->nodo[j]->y);
						glEnd();
					}
				}
			}
		}
	}
} 
//----------------------------------------------------------------------
void Visual::desenha_front_walls(Cenario *cenario) {
	glColor3f (1.0, 0.0, 0.0);
	glLineWidth((GLfloat)3.5);
	for (Elemento *aux = cenario->aberto->topo; aux != NULL; aux = aux->prox) {
		glBegin(GL_LINES);
			glVertex2f (aux->p1.x, aux->p1.y);
			glVertex2f (aux->p2.x, aux->p2.y);
		glEnd();
	}
	glColor3f (1.0, 1.0, 0.0);
	for (Elemento *aux = cenario->frontWall->topo; aux != NULL; aux = aux->prox) {
		glBegin(GL_LINES);
			glVertex2f (aux->p1.x, aux->p1.y);
			glVertex2f (aux->p2.x, aux->p2.y);
		glEnd();
	}
}
//----------------------------------------------------------------------
void Visual::desenha_caminho_mesh(Grafo *grafo, Agente *agente) {
	glPointSize((GLfloat)10.0f);
	int i;
	
	///Mostra todos os nodos visitados pelo A*
	if (this->visualiza_nodos_fechados) {
		glColor3f (1.0, 0.0, 0.0);
		for (int j = 0; j < grafo->num_no; j++) {
			if (grafo->nodo[j]->estado == -1) {
				i = j;
				if (grafo->nodo[i]->mesh->tipo != 3) {
					glBegin(GL_QUADS);
						glVertex2f (grafo->nodo[i]->mesh->pontos[0].x, grafo->nodo[i]->mesh->pontos[0].y);
						glVertex2f (grafo->nodo[i]->mesh->pontos[1].x, grafo->nodo[i]->mesh->pontos[1].y);
						glVertex2f (grafo->nodo[i]->mesh->pontos[3].x, grafo->nodo[i]->mesh->pontos[3].y);
						glVertex2f (grafo->nodo[i]->mesh->pontos[2].x, grafo->nodo[i]->mesh->pontos[2].y);
					glEnd();
				}
				else {
					glBegin(GL_TRIANGLES);
						glVertex2f (grafo->nodo[i]->mesh->pontos[0].x, grafo->nodo[i]->mesh->pontos[0].y);
						glVertex2f (grafo->nodo[i]->mesh->pontos[1].x, grafo->nodo[i]->mesh->pontos[1].y);
						glVertex2f (grafo->nodo[i]->mesh->pontos[2].x, grafo->nodo[i]->mesh->pontos[2].y);
					glEnd();
				}
			}
		}
	}
	
	///Mostra o caminho final
	glColor3f (0.5, 1.0, 0.5);
	if (agente->path.caminho_mesh->first) {
		for (Item *p = agente->path.caminho_mesh->first; p != NULL; p = p->prox) {
			i = p->id;
			if (grafo->nodo[i]->mesh->tipo != 3) {
				glBegin(GL_QUADS);
					glVertex2f (grafo->nodo[i]->mesh->pontos[0].x, grafo->nodo[i]->mesh->pontos[0].y);
					glVertex2f (grafo->nodo[i]->mesh->pontos[1].x, grafo->nodo[i]->mesh->pontos[1].y);
					glVertex2f (grafo->nodo[i]->mesh->pontos[3].x, grafo->nodo[i]->mesh->pontos[3].y);
					glVertex2f (grafo->nodo[i]->mesh->pontos[2].x, grafo->nodo[i]->mesh->pontos[2].y);
				glEnd();
			}
			else {
				glBegin(GL_TRIANGLES);
					glVertex2f (grafo->nodo[i]->mesh->pontos[0].x, grafo->nodo[i]->mesh->pontos[0].y);
					glVertex2f (grafo->nodo[i]->mesh->pontos[1].x, grafo->nodo[i]->mesh->pontos[1].y);
					glVertex2f (grafo->nodo[i]->mesh->pontos[2].x, grafo->nodo[i]->mesh->pontos[2].y);
				glEnd();
			}
		}
	}
	
	///Mostra a mesh atual
	glColor3f (0.5, 0.5, 1.0);
	i = agente->path.mesh_atual;
	if (i == -1)
		printf("Mesh atual = -1 na classe 'visual'....\n");
	if (grafo->nodo[i]->mesh->tipo != 3) {
		glBegin(GL_QUADS);
			glVertex2f (grafo->nodo[i]->mesh->pontos[0].x, grafo->nodo[i]->mesh->pontos[0].y);
			glVertex2f (grafo->nodo[i]->mesh->pontos[1].x, grafo->nodo[i]->mesh->pontos[1].y);
			glVertex2f (grafo->nodo[i]->mesh->pontos[3].x, grafo->nodo[i]->mesh->pontos[3].y);
			glVertex2f (grafo->nodo[i]->mesh->pontos[2].x, grafo->nodo[i]->mesh->pontos[2].y);
		glEnd();
	}
	else {
		glBegin(GL_TRIANGLES);
			glVertex2f (grafo->nodo[i]->mesh->pontos[0].x, grafo->nodo[i]->mesh->pontos[0].y);
			glVertex2f (grafo->nodo[i]->mesh->pontos[1].x, grafo->nodo[i]->mesh->pontos[1].y);
			glVertex2f (grafo->nodo[i]->mesh->pontos[2].x, grafo->nodo[i]->mesh->pontos[2].y);
		glEnd();
	}
	/*glColor3f (1.0, 0.0, 0.0);
	for (int j = 0; j < grafo->num_no; j++) {
		for (int k = 0; k < grafo->num_no; k++)
			if (grafo->peso[j][k]) {
				glBegin(GL_LINES);
					glVertex2f (grafo->nodo[j]->x, grafo->nodo[j]->y);
					glVertex2f (grafo->nodo[k]->x, grafo->nodo[k]->y);
				glEnd();
			}
	}*/
	
	glBegin(GL_POINTS);
		//glColor3f (1.0, 0.5, 0.5);
		//glVertex2f(fugitivo->path.start.x, fugitivo->path.start.y);
		glColor3f (0.5, 0.0, 0.0);
		glVertex2f(agente->path.end.x, agente->path.end.y);
	glEnd();
}
//----------------------------------------------------------------------
void Visual::desenha_Bspline (Agente *agente) {
	Lista *p; 
    glPointSize(6.0f);

    ///Desenha pontos de controle
	if (this->visualiza_pontos_controle) {
		glBegin(GL_POINTS);
			for(p = agente->path.caminho_exato->pontos; p != NULL; p = p->prox)	{
				glColor3f(1.0, 0.0, 0.0);
				glVertex2f(p->x, p->y);
			}
		glEnd();
	}

	///Grafo de Controle: desenha linha poligonal que conecta os pontos de controle em ordem
	if (this->visualiza_linhas_controle) {
		if (agente->path.caminho_exato->mostraLinhas == 1) 	{
			glLineWidth((GLfloat)0.5);
			glBegin(GL_LINE_STRIP);
				glColor3f(0.8, 0.8, 1.0); 
				for(p = agente->path.caminho_exato->pontos; p != NULL; p = p->prox)
					glVertex2f(p->x, p->y);
			glEnd();
		}
	}

    ///Desenha B-Spline
    if (this->visualiza_caminho_exato) {
		Fugitivo *aux = dynamic_cast<Fugitivo*>(agente);
		if (aux)
			glColor3f(0.0, 0.0, 0.0);
		else
			glColor3f(0.7, 0.0, 0.0);
		///Número mínimo de pontos de controle estipulados para a geração da curva
		if (agente->path.caminho_exato->cont_pontos > 3) { 
			glLineWidth((GLfloat)3);
			glBegin(GL_LINE_STRIP);
				if(agente->path.caminho_exato->bspline) {
					for(int i = agente->path.p_atual_bspline; i >= 0; i--) {
						glVertex2f(agente->path.caminho_exato->bspline[i].x, agente->path.caminho_exato->bspline[i].y);
					}
				}
			glEnd();
		}
	}
}
//----------------------------------------------------------------------
void Visual::desenha_fugitivo (Fugitivo *fugitivo) {
	glPointSize((GLfloat)10.0f);
	glColor3f (1.0, 0.5, 0.5);
	glBegin(GL_POINTS);
		glVertex2f(fugitivo->path.atual.x, fugitivo->path.atual.y);
	glEnd();
}
//----------------------------------------------------------------------
void Visual::desenha_inimigos (Inimigo **inimigo, int num_inimigos) {
	glPointSize((GLfloat)10.0f);
	

	for (int i = 0; i < num_inimigos; i++) {
		if (!inimigo[i]->morto) {
			glColor3f (0.0, 0.0, 0.7);
			glBegin(GL_POINTS);
				glVertex2f(inimigo[i]->path.atual.x, inimigo[i]->path.atual.y);
			glEnd();
			glColor3f (0.7, 0.0, 0.0);
			glBegin(GL_LINE_STRIP);
				glVertex2f(inimigo[i]->path.atual.x, inimigo[i]->path.atual.y);
				glVertex2f(inimigo[i]->path.atual.x + inimigo[i]->path.direcao.x, inimigo[i]->path.atual.y + inimigo[i]->path.direcao.y);
			glEnd();
		
			if (inimigo[i]->estado_visao == VISUALIZA_CAMINHO) {
				glLineWidth((GLfloat)3.0);
				glColor3f (0.8, 0.4, 0.4);
			}
			else if (inimigo[i]->estado_visao == VISUALIZA_AGENTE) {
				glLineWidth((GLfloat)4.5);
				glColor3f (1.0, 0.0, 0.0);
			}
			else if (inimigo[i]->alerta) {
				if (inimigo[i]->estado_visao == VERIFICA_BARULHO) {
					glLineWidth((GLfloat)4.5);
					glColor3f (0.7, 0.7, 0.7);
				}
				else {
					glLineWidth((GLfloat)4.5);
					glColor3f (0.7, 0.7, 0.3);
				}
			}
			else if (inimigo[i]->estado_visao == NEUTRO) {
				glLineWidth((GLfloat)2.0);
				glColor3f (0.0, 0.0, 0.7);
			}
			else {
				glLineWidth((GLfloat)4.5);
				glColor3f (0.3, 1.0, 0.3);
			}
			glBegin(GL_LINE_STRIP);
				glVertex2f (inimigo[i]->visao.pontos[0].x, inimigo[i]->visao.pontos[0].y);
				glVertex2f (inimigo[i]->visao.pontos[1].x, inimigo[i]->visao.pontos[1].y);
				glVertex2f (inimigo[i]->visao.pontos[2].x, inimigo[i]->visao.pontos[2].y);
				glVertex2f (inimigo[i]->visao.pontos[0].x, inimigo[i]->visao.pontos[0].y);
			glEnd();
		}
		else  {
			glColor3f (0.9, 0.0, 0.0);
			glBegin(GL_POINTS);
				glVertex2f(inimigo[i]->path.atual.x, inimigo[i]->path.atual.y);
			glEnd();
		}
	}
}
//----------------------------------------------------------------------
void Visual::desenha_cenario(Cenario *cenario, Grafo *grafo, Fugitivo *fugitivo, Inimigo **inimigo, int num_inimigos) {
	if (this->visualiza_caminho_mesh) {
		this->desenha_caminho_mesh(grafo, fugitivo);
		for (int i = 0; i < num_inimigos; i++) {
			if (!inimigo[i]->morto)
				this->desenha_caminho_mesh(grafo, inimigo[i]);
		}
	}
		
	this->desenha_bordas(cenario);
	this->desenha_parede(cenario);
	
	if (this->visualiza_mesh)
		this->desenha_meshes(cenario);
		
	if (this->visualiza_grafo)
		this->desenha_grafo(grafo);

	
	this->desenha_Bspline(fugitivo);
	for (int i = 0; i < num_inimigos; i++) {
		if (!inimigo[i]->morto)
			this->desenha_Bspline(inimigo[i]);
	}
	
	this->desenha_fugitivo(fugitivo);
	this->desenha_inimigos(inimigo, num_inimigos);
}








