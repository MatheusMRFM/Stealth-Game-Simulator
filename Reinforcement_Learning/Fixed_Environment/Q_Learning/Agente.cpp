#include <stdio.h>
#include <stdlib.h>
#include <time.h> 
#include <math.h> 
#include "Agente.hpp"


Agente::Agente() {
	this->path.caminho_exato = new Bspline();
	this->path.direcao.z = 0;
	this->path.caminho_mesh = new ListaDE();
	this->path.velocidade = 0.0;
	this->path.estado_velocidade = ESTADO_PARADO;
	this->path.terminou_caminho = false;
}

Agente::~Agente() {
	
}
//----------------------------------------------------------------------
void Agente::gera_posicao_inicial (float tam_cenario) {
	
}

void Agente::gera_caminho_exato () {
	
}

Ponto_Mapa* Agente::encontra_ponto_controle (Ponto_Mapa *m1_p, int m2, int m3) {
	
	return NULL;
}

void Agente::restaura_valores () {
	
}
//----------------------------------------------------------------------
void Agente::inicializa_dados (bool limpa_pesos) {
	this->grafo->limpa_caminho_anterior(limpa_pesos);
	this->path.caminho_exato->limpa_bspline();
	delete this->path.caminho_mesh;
	this->path.caminho_mesh = new ListaDE();
	this->path.p_atual_bspline = this->path.caminho_exato->pontos_discretizacao - 1;
	this->path.entre_pontos_bspline = false;
}
//----------------------------------------------------------------------
Ponto_Mapa* Agente::gera_posicao (float tam_cenario, int id_inimigo) {
	srand(SEED_FIXA + id_inimigo);
	Ponto_Mapa *p = (Ponto_Mapa*) malloc (sizeof(Ponto_Mapa));
	int mesh;
	bool fim = false;
	while (!fim) {
		p->x = rand() % (int)tam_cenario;
		p->y = rand() % (int)tam_cenario;
		mesh = this->grafo->encontra_mesh(p);
		if (mesh != -1)
			fim = true;
	}
	
	return p;
}
//----------------------------------------------------------------------
bool Agente::gera_caminho_mesh () {
	bool existe_caminho = this->grafo->encontra_menor_caminho(&this->path.start, this->path.mesh_start, &this->path.end, this->path.mesh_end);
	if (existe_caminho) {
		///Agora o caminho está gravado no grafo. Basta seguir os pais dos nós a partir do nó final
		int i = this->path.mesh_end; 
		while (this->grafo->caminho_circular || i != this->path.mesh_start) {
			this->grafo->caminho_circular = false;
			this->path.caminho_mesh->insere_inicio(i, 0);
			this->grafo->nodo[i]->pertence_caminho = true;
			if (i < 0) {
				printf("ERRO: Caminho gravado errado!\n\ni = %d\n", i);
				exit(0);
			}
			i = this->grafo->nodo[i]->pai;
		}
		this->grafo->nodo[i]->pertence_caminho = true;
		this->path.caminho_mesh->insere_inicio(i, 0);
	}
	
	return existe_caminho;
}
//----------------------------------------------------------------------
///Gera-se a Bspline baseado no caminho de meshes (a bspline representa o caminho exato). Cada ponto da bspline é o centro da mesh
void Agente::gera_caminho_exato_basico () {
	int i;
	///Insere o primeiro ponto duas vezes para a Bspline começar no ponto inicial
	this->path.caminho_exato->insere_ponto(this->path.start.x, this->path.start.y, this->path.mesh_start);
	this->path.caminho_exato->insere_ponto(this->path.start.x, this->path.start.y, this->path.mesh_start);
	
	Item *p = this->path.caminho_mesh->first;
	if (p) {
		p = p->prox;
		for (p = p; p != NULL; p = p->prox) {
			i = p->id;
			this->path.caminho_exato->insere_ponto(this->grafo->nodo[i]->x, this->grafo->nodo[i]->y, p->id);
		}
	}
	///Insere o último ponto duas vezes para a Bspline terminar no ponto final
	this->path.caminho_exato->insere_ponto(this->path.end.x, this->path.end.y, this->path.mesh_end);
	this->path.caminho_exato->insere_ponto(this->path.end.x, this->path.end.y, this->path.mesh_end);
	
	///Atualiza B-Spline
	this->path.caminho_exato->rearranjaNos();
	this->path.caminho_exato->atualizaBspline();
}
//----------------------------------------------------------------------
void Agente::gera_caminho () {
	
}
//----------------------------------------------------------------------
void Agente::atualiza_presenca_mesh () {
	
}
//----------------------------------------------------------------------
void Agente::altera_velocidade (bool anda) {
	
}
//----------------------------------------------------------------------
void Agente::atualiza_posicao () {
	float dx, dy, dx2, dy2;
	float total = 0.0, dist;
	double angulo;
	
	if (this->path.velocidade > 0.0 && ((this->path.mesh_objetivo != this->path.mesh_atual) || (this->path.mesh_objetivo == this->path.mesh_end))) {
		for (int i = this->path.p_atual_bspline; i > 0; i--) {
			if (this->path.entre_pontos_bspline) {
				dx = this->path.atual.x - this->path.caminho_exato->bspline[i-1].x;
				dy = this->path.atual.y - this->path.caminho_exato->bspline[i-1].y;
			}
			else {
				dx = this->path.caminho_exato->bspline[i].x - this->path.caminho_exato->bspline[i-1].x;
				dy = this->path.caminho_exato->bspline[i].y - this->path.caminho_exato->bspline[i-1].y;
			}
			dist = sqrt(dx*dx + dy*dy);
			if (total + dist <= this->path.velocidade) {
				this->path.entre_pontos_bspline = false;
				total += dist;
				this->path.p_atual_bspline--;
				if (this->path.p_atual_bspline == 0) {
					this->path.atual.x = this->path.end.x;
					this->path.atual.y = this->path.end.y;
				}
				else {
					this->path.atual.x = this->path.caminho_exato->bspline[this->path.p_atual_bspline].x;
					this->path.atual.y = this->path.caminho_exato->bspline[this->path.p_atual_bspline].y;
				}
			}
			else {
				dist = this->path.velocidade - total;
				if (dy < 0) {
					angulo = atan2(fabs(dy), fabs(dx));
					dx2 = dist*cos(angulo);
					dy2 = dist*sin(angulo);
				}
				else {
					angulo = atan2(fabs(dx), fabs(dy));
					dx2 = dist*sin(angulo);
					dy2 = dist*cos(angulo);
				}
				if (dx > 0)
					this->path.atual.x -= dx2;
				else
					this->path.atual.x += dx2;
				if (dy > 0)
					this->path.atual.y -= dy2;
				else
					this->path.atual.y += dy2;
				
				this->path.entre_pontos_bspline = true;
				break;
			}
		}
		
		if (this->path.atual.x == this->path.start.x && this->path.atual.y == this->path.start.y)
			this->path.terminou_caminho = true;
		else {
			this->path.direcao.x = this->path.atual.x - this->path.start.x;
			this->path.direcao.y = this->path.atual.y - this->path.start.y;
		}
		this->path.start.x = this->path.atual.x;
		this->path.start.y = this->path.atual.y;
	}
	
	this->atualiza_presenca_mesh();
}
//----------------------------------------------------------------------
Ponto_Mapa* Agente::caminho_atravessa_parede (Cenario *cenario) {
	Ponto_Mapa *ponto_encontro = (Ponto_Mapa*) malloc (sizeof(Ponto_Mapa));
	
	for (int i = this->path.p_atual_bspline; i >= 0; i--) {
		for (int j = 0; j < cenario->num_parede; j++) {
			ponto_encontro->x = this->path.caminho_exato->bspline[i].x;
			ponto_encontro->y = this->path.caminho_exato->bspline[i].y;
			if (verifica_dentro_poligono(cenario->paredes[j]->pontos, ponto_encontro)) 
				return ponto_encontro;
		}
	}
	
	free(ponto_encontro);
	
	return NULL;
}
//----------------------------------------------------------------------
Ponto_Mapa* Agente::verifica_caminho_valido (Cenario *cenario) {
	
	return NULL;
}
//----------------------------------------------------------------------
int Agente::mesh_caminho_mais_proxima (Ponto_Mapa *p) {
	Lista *aux;
	int m, temp;
	float menor_dist = -1;
	float dx, dy, dist;
	
	m = this->grafo->encontra_mesh(p);
	///Se m = -1, então trata-se de um ponto de colisão com uma parede (assim, o ponto pertence dentro da parede)
	if (m != -1 && this->grafo->nodo[m]->pertence_caminho)
		return m;
	
	if (this->path.caminho_exato->pontos) {
		///Percorre os pontos de controle de trás pra frente
		for (aux = this->path.caminho_exato->pontos; aux->prox != NULL; aux = aux->prox) {}
		///Percorre todos os pontos de controle da Bspline
		for (Lista *l = aux; l != NULL; l = l->ant) {
			dx = l->x - p->x;
			dy = l->y - p->y;
			dist = sqrt(dx*dx + dy*dy);
			temp = l->num;	///temp = mesh correspondente do ponto de controle l
			if ((menor_dist == -1 || dist <= menor_dist) && this->grafo->nodo[temp]->pertence_caminho) {
				menor_dist = dist;
				m = temp;
			}
		}
	}
	else {
		//printf("Caminho VAZIO!\n");
	}
	
	return m;
}

























