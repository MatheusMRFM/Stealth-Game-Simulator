#include <stdio.h>
#include <stdlib.h>
#include <math.h> 
#include <time.h> 
#include "Inimigo.hpp"

#define TAM_VETOR_DIRECAO 10

extern int SEED_FIXA;

Inimigo::Inimigo(Cenario *cenario, int id_inimigo) {
	srand(SEED_FIXA+id_inimigo);
	this->id = id_inimigo;
	this->num_percursos = 0;
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
	this->existe_caminho = true;
	this->morto = false;
	this->alerta = false;
	this->encontrou_corpo = false;
	this->grafo = new Grafo(cenario->nav_mesh);
}

Inimigo::~Inimigo() {
	
}

void Inimigo::restaura_valores (Cenario* cenario) {
	srand(SEED_FIXA + this->id);
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
	
	this->path.terminou_caminho = true;
	this->path.velocidade = 0;
	this->num_percursos = 0;
	this->espera = 0;
	this->tam_visao = TAM_VISAO_INICIAL;
	this->angulo_visao = ANGULO_VISAO;
	this->estado_visao = NEUTRO;
	this->existe_caminho = true;
	this->morto = false;
	this->alerta = false;
	this->encontrou_corpo = false;
	delete this->grafo;
	this->grafo = new Grafo(cenario->nav_mesh);
}
//**********************************************************************
void Inimigo::gera_posicao_inicial (float tam_cenario, Agente *fugitivo, Parede **parede, int num_parede) {
	srand(SEED_FIXA + this->id);
	bool fim = false;
	float dx, dy, dist;
	while (!fim) {
		this->path.start.x = rand() % (int)tam_cenario;
		this->path.start.y = rand() % (int)tam_cenario;
		this->path.atual.x = this->path.start.x;
		this->path.atual.y = this->path.start.y;
		this->path.mesh_start = this->grafo->encontra_mesh(&this->path.start);
		this->path.mesh_atual = this->path.mesh_start;
		this->atualiza_campo_visao();
		dx = this->path.atual.x - fugitivo->path.atual.x;
		dy = this->path.atual.y - fugitivo->path.atual.y;
		dist = sqrt(dx*dx + dy*dy);
		if (this->path.mesh_start != -1 && dist > DIST_AGENTE_INICIAL && dist < DIST_MAX_AGENTE_INICIAL)
			fim = true;
	}
	
	this->path.atual.x = this->path.start.x;
	this->path.atual.y = this->path.start.y;
	this->path.end.x = this->path.start.x;
	this->path.end.y = this->path.start.y;
	this->path.mesh_atual = this->path.mesh_start;
	this->grafo->nodo[this->path.mesh_atual]->existe_inimigo += 1;
}
//**********************************************************************
void Inimigo::atualiza_presenca_mesh () {
	int mesh_antiga = this->path.mesh_atual;
	this->path.mesh_atual = this->grafo->encontra_mesh(&this->path.atual);
	
	///Se o inimigo sair de uma mesh m1 e entrar em uma m2, 
	///indica que a m1 possui 1 inimigo a menos e que m2 tem 1 inimigo a mais
	if (this->path.mesh_atual != mesh_antiga) {
		if (this->path.mesh_atual == -1)
			this->path.mesh_atual = mesh_antiga;
		else {
			this->grafo->nodo[this->path.mesh_atual]->existe_inimigo++;
			this->grafo->nodo[mesh_antiga]->existe_inimigo--;
		}
	}
	
	this->path.mesh_start = this->path.mesh_atual;
}
//**********************************************************************
void Inimigo::altera_velocidade (bool anda) {
	if (anda) {
		this->path.estado_velocidade = ESTADO_SNEAK;
		if (this->alerta)
			this->path.velocidade = MODULO_VELOCIDADE_ALERTA;
		else
			this->path.velocidade = MODULO_VELOCIDADE_PATRULHA;
	}
	else {
		this->path.estado_velocidade = ESTADO_PARADO;
		this->path.velocidade = 0.0;
	}
}
//**********************************************************************
void Inimigo::atualiza_campo_visao () {
	Vetor3D v1, v2;
	v1.x = this->path.direcao.x;
	v1.y = this->path.direcao.y;
	v1.z = 0;
	v2.x = 1;
	v2.y = 0;
	v2.z = 0;
	
	float phi = angulo_entre_vetores(&v1, &v2);
	float beta = phi - this->angulo_visao/2.0;
	if (this->path.direcao.y < 0) {
		beta = phi - PI/2.0 - this->angulo_visao/2.0;
	}
	float alpha = beta + this->angulo_visao;
	
	this->visao.pontos[0].x = this->path.atual.x;											this->visao.pontos[0].y = this->path.atual.y;
	if (this->path.direcao.y >= 0) {
		this->visao.pontos[1].x = this->visao.pontos[0].x + this->tam_visao*cos(beta);		this->visao.pontos[1].y = this->visao.pontos[0].y + this->tam_visao*sin(beta);
		this->visao.pontos[2].x = this->visao.pontos[0].x + this->tam_visao*cos(alpha);		this->visao.pontos[2].y = this->visao.pontos[0].y + this->tam_visao*sin(alpha);
	}
	else {
		this->visao.pontos[1].x = this->visao.pontos[0].x - this->tam_visao*sin(beta);		this->visao.pontos[1].y = this->visao.pontos[0].y - this->tam_visao*cos(beta);
		this->visao.pontos[2].x = this->visao.pontos[0].x - this->tam_visao*sin(alpha);		this->visao.pontos[2].y = this->visao.pontos[0].y - this->tam_visao*cos(alpha);
	}
}
//**********************************************************************
bool Inimigo::enxerga_ponto (Ponto_Mapa *p, Parede **parede, int num_parede, bool ilimitado) {
	bool valida, l1, l2, enxerga = true;
	///Verifica se o agente está dentro do campo de visão do inimigo
	if (this->morto)
		printf("-----------Estou morto!!!\n");
	if (verifica_dentro_triangulo(this->visao.pontos, p) || ilimitado) {
		///Verifica todas as paredes
		for (int i = 0; i < num_parede; i++) {
			valida = false;
			///Verifica se a parede em questão pode bloquear o inimigo
			if (!ilimitado) {
				for (int j = 0; j < 4; j++) {
					if (verifica_dentro_triangulo(this->visao.pontos, &parede[i]->pontos[j])) {
						valida = true;
						break;
					}
				}
			}
			else
				valida = true;
		
			if (valida) {
				///Verifica se a parede em questão encontra-se entre o agente e o inimigo
				l1 = linhas_se_cruzam(&parede[i]->pontos[0], &parede[i]->pontos[3], p, &this->path.atual);
				if (l1)		return false;
				l2 = linhas_se_cruzam(&parede[i]->pontos[1], &parede[i]->pontos[2], p, &this->path.atual);
				if (l2)		return false;
				
				enxerga = true;
			}
		}
	}
	else {
		enxerga = false;
	}
	
	return enxerga;
}
//**********************************************************************
bool Inimigo::enxerga_ponto_visao_ilimitada (Ponto_Mapa *p, Parede **parede, int indice_parede) {
	bool l1, l2;

	///Verifica se a parede em questão encontra-se entre o agente e o inimigo
	l1 = linhas_se_cruzam(&parede[indice_parede]->pontos[0], &parede[indice_parede]->pontos[3], p, &this->path.atual);
	if (l1)		return false;
	l2 = linhas_se_cruzam(&parede[indice_parede]->pontos[1], &parede[indice_parede]->pontos[2], p, &this->path.atual);
	if (l2)		return false;
	
	return true;
}
//**********************************************************************
void Inimigo::gera_caminho_exato () {
	
}
//**********************************************************************
Ponto_Mapa* Inimigo::encontra_ponto_controle (Ponto_Mapa *m1_p, int m2, int m3) {
	
	return NULL;
}
//**********************************************************************
void Inimigo::gera_caminho (Cenario *cenario) {
	Ponto_Mapa *p = NULL;
	int mesh_penalizada;
	
	this->grafo->set_modo(false);
	this->inicializa_dados(true);
	
	this->existe_caminho = this->gera_caminho_mesh();
	if (this->existe_caminho) {
		this->gera_caminho_exato_basico();
		p = this->verifica_caminho_valido(cenario);
	
		if (p != NULL) {
			mesh_penalizada = this->mesh_caminho_mais_proxima (p);
			if (mesh_penalizada == -1)
				printf("Merda no Inimigo...\n");
			this->grafo->penaliza_transicao_mesh (mesh_penalizada, this->path.mesh_end);
			this->inicializa_dados(false);
			this->existe_caminho = this->gera_caminho_mesh();
			if (this->existe_caminho) {
				this->gera_caminho_exato_basico();
				free(p);
				p = this->verifica_caminho_valido(cenario);
			}
			else {
				free(p);
				p = NULL;
			}
		}
	
		this->path.mesh_objetivo = this->path.mesh_end;
	}
}
//**********************************************************************
Ponto_Mapa* Inimigo::verifica_caminho_valido (Cenario *cenario) {
	return caminho_atravessa_parede(cenario);
}
//**********************************************************************
void Inimigo::define_novo_destino (Cenario *cenario, Ponto_Mapa *d) {
	srand(SEED_FIXA);
	if (this->path.terminou_caminho) {
		///O inimigo já terminou seu caminho anterior e está pronto para caminhar novamente
		if (this->espera == 0) {
			this->estado_visao = NEUTRO;
			if (this->alerta)
				this->espera = 0;
			else
				this->espera = rand() % NUM_FRAMES_ESPERA;
			
			Ponto_Mapa *p;
			float dist = -1, dx, dy;
			srand(time(NULL));
			int chance = rand() % 100;
			
			if (chance < CHANCE_PROX_DESTINO) {
				int cont = 0;
				while (dist == -1 || dist > DIST_MAX_DESTINO) {
					p = this->gera_posicao(cenario->tamanho, cont);
					dx = p->x - d->x;
					dy = p->y - d->y;
					dist = sqrt(dx*dx + dy*dy);
					cont++;
					if (cont > 1000) {
						printf("d = (%f, %f)\n", p->x, p->y);
					}
				}
			}
			else {
				p = this->gera_posicao(cenario->tamanho, this->id + this->num_percursos);
			}
			this->num_percursos++;
			
			
			this->path.mesh_end = grafo->encontra_mesh(p);
			this->path.end.x = p->x;
			this->path.end.y = p->y;
			this->gera_caminho(cenario);
			free(p);
			if (this->existe_caminho) {
				this->path.terminou_caminho = false;
				this->altera_velocidade(true);
			}
		}
		///O inimigo ainda vai ficar parado
		else if (this->espera > 0) {
			this->altera_velocidade(false);
			this->espera--;
		}
	}
}
//**********************************************************************
void Inimigo::verifica_pertubacao (Cenario *cenario, Ponto_Mapa *p, int mesh) {
	this->espera = NUM_FRAMES_ESPERA;
	this->path.mesh_end = mesh;
	if (this->path.mesh_end == -1)
		printf("Ponto de distração sem mesh!!\n");
	this->path.end.x = p->x;
	this->path.end.y = p->y;
	this->gera_caminho(cenario);
	if (this->existe_caminho) {
		this->path.terminou_caminho = false;
		this->altera_velocidade(true);
	}
}
//**********************************************************************
void Inimigo::entra_alerta() {
	this->alerta = true;
	this->altera_velocidade(true);
}
//**********************************************************************
void Inimigo::ve_inimigo_morto (Cenario* cenario, Inimigo** inimigo, int num_inimigos) {
	Ponto_Mapa p;
	bool a = false;
	
	for (int i = 0; i < num_inimigos; i++) {
		if (inimigo[i]->morto) {
			p.x = inimigo[i]->path.atual.x;
			p.y = inimigo[i]->path.atual.y;
			if (this->enxerga_ponto(&p, cenario->paredes, cenario->num_parede, false)) {
				a = true;
				break;
			}
		}
	}
	
	if (a) {
		for (int i = 0; i < num_inimigos; i++) {
			inimigo[i]->alerta = true;
			inimigo[i]->encontrou_corpo = true;
		}
	}
}
//**********************************************************************


























