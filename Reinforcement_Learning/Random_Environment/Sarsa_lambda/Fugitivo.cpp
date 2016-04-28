#include <stdio.h>
#include <stdlib.h>
#include <math.h> 
#include <unistd.h>
#include "Fugitivo.hpp"

Fugitivo::Fugitivo(Cenario *cenario) {
	this->em_cobertura = false;
	this->existe_caminho = true;
	this->caminho_errado = false;
	this->path.velocidade = MODULO_VELOCIDADE_ANDA;
	this->tempo_recalcula_1 = TEMPO_RECALCULA_1;
	this->tempo_recalcula_2 = TEMPO_RECALCULA_2;
	this->entre_coberturas = true;
	this->paredes_proximas = (int*) malloc (sizeof(int)*cenario->num_parede);
	this->atualiza_distancias_parede(cenario);
	this->perseguindo = false;
	this->furtivo = true;
	this->inimigo_mais_prox = -1;
	this->matou_recente = SEM_TENTATIVA;
	this->grafo = new Grafo(cenario->nav_mesh);
}

Fugitivo::~Fugitivo() {
	
}

void Fugitivo::restaura_valores (Cenario* cenario) {
	this->path.terminou_caminho = false;
	this->existe_caminho = true;
	this->caminho_errado = false;
	this->path.velocidade = MODULO_VELOCIDADE_ANDA;
	this->tempo_recalcula_1 = TEMPO_RECALCULA_1;
	this->tempo_recalcula_2 = TEMPO_RECALCULA_2;
	this->encontrado = false;
	free(this->paredes_proximas);
	this->paredes_proximas = (int*) malloc (sizeof(int)*cenario->num_parede);
	delete this->grafo;
	this->grafo = new Grafo(cenario->nav_mesh);
}

void Fugitivo::restaura_caminho (Cenario* cenario, Inimigo** inimigo, int num_inimigos) {
	this->inicializa_dados(true);
	this->caminho_errado = false;
	this->existe_caminho = true;
	this->path.end.x = this->p_destino.x;
	this->path.end.y = this->p_destino.y;	
	this->path.mesh_end = this->grafo->encontra_mesh(&this->path.end);
	this->gera_caminho(cenario, inimigo, num_inimigos);
}
//----------------------------------------------------------------------
void Fugitivo::gera_posicao_inicial (float tam_cenario) {
	bool fim = false;
	//float dist, dx, dy, maior_dist = tam_cenario*sqrt(2); ///Maior distancia possivel é a diagonal do cenario
	while (!fim) {
		this->path.start.x = rand() % (int)tam_cenario;
		this->path.start.y = rand() % (int)tam_cenario;
		this->path.mesh_start = this->grafo->encontra_mesh(&this->path.start);
		if (this->path.mesh_start != -1)
			fim = true;
	}
	
	fim = false;
	while (!fim) {
		this->path.end.x = rand() % (int)tam_cenario;
		this->path.end.y = rand() % (int)tam_cenario;
		this->path.mesh_end = this->grafo->encontra_mesh(&this->path.end);
		if (this->path.mesh_end != -1) {
			/*///Faz com que o destino esteja sempre em um intervalo de distancia do agente (evita valores muito pequenos ou muito grandes)
			dx = this->path.end.x - this->path.start.x;
			dy = this->path.end.y - this->path.start.y;
			dist = sqrt(dx*dx + dy*dy);
			if (dist <= PCT_MAX_DIST_DESTINO*maior_dist && dist >= PCT_MIN_DIST_DESTINO*maior_dist)*/
				fim = true;
		}
	}
	
	
	this->p_destino.x = this->path.end.x;
	this->p_destino.y = this->path.end.y;
	this->path.atual.x = this->path.start.x;
	this->path.atual.y = this->path.start.y;
	this->path.mesh_atual = this->path.mesh_start;
}
//----------------------------------------------------------------------
void Fugitivo::define_destino (float tam_cenario) {
	Ponto_Mapa *p = this->gera_posicao(tam_cenario);
	this->path.end.x = p->x;
	this->path.end.y = p->y;
	this->path.mesh_end = this->grafo->encontra_mesh(p);
	free(p);
}
//----------------------------------------------------------------------
void Fugitivo::atualiza_presenca_mesh () {
	int mesh_antiga = this->path.mesh_atual;
	this->path.mesh_atual = this->grafo->encontra_mesh(&this->path.atual);
	if (this->path.mesh_atual == -1)
		this->path.mesh_atual = mesh_antiga;
		
	this->path.mesh_start = this->path.mesh_atual;
	if (this->path.mesh_start == -1) {
		//printf("ERRO: Mesh start = -1\n");
		exit(0);
	}
	
	if (this->grafo->nodo[this->path.mesh_atual]->mesh->tipo == 0 || this->grafo->nodo[this->path.mesh_atual]->mesh->tipo == 3) 
		this->em_cobertura = false;
	else
		this->em_cobertura = true;
}
//----------------------------------------------------------------------
void Fugitivo::altera_velocidade (bool anda, Inimigo **inimigo, int num_inimigos) {
	if (anda) {
		float dx, dy, dist;
		if (this->em_cobertura) {
			this->path.estado_velocidade = ESTADO_ANDA;
			this->path.velocidade = MODULO_VELOCIDADE_ANDA;
		}
		else {
			this->path.estado_velocidade = ESTADO_CORRE;
			this->path.velocidade = MODULO_VELOCIDADE_CORRE;
		}
		for (int i = 0; i < num_inimigos; i++) {
			if (!inimigo[i]->morto) {
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
				else if ((dist < RAIO_SOM_CORRE || this->em_cobertura) && this->path.estado_velocidade >= ESTADO_ANDA) {
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
	}
	else {
		this->path.estado_velocidade = ESTADO_PARADO;
		this->path.velocidade = 0.0;
	}
}

//----------------------------------------------------------------------
Ponto_Mapa* Fugitivo::encontra_ponto_controle (Ponto_Mapa *m1_p, int m2, int m3, Inimigo **inimigos, int num_inimigos) {
	Ponto_Mapa *aux = (Ponto_Mapa*) malloc (sizeof(Ponto_Mapa));
	Ponto_Mapa *p = (Ponto_Mapa*) malloc (sizeof(Ponto_Mapa));
	Ponto_Mapa p_end;
	float x = m1_p->x;
	float y = m1_p->y;
	float x_dest, y_dest;
	float dist_temp = -1, dx, dy, temp;
	bool inimigo_enxerga;
	
	if (this->path.mesh_end == m3 || this->path.mesh_end == m2) {
		x_dest = this->path.end.x;
		y_dest = this->path.end.y;
	}
	else {
		x_dest = this->grafo->nodo[m3]->x;
		y_dest = this->grafo->nodo[m3]->y;
	}
	
	p_end.x = x_dest;
	p_end.y = y_dest;
	
	for (int i = 0; i < 4 && this->grafo->nodo[m2]->mesh->pontos[i].x != -1; i++) {
		///Testa o ponto i
		///Distancia do ponto i ao ponto inicial
		dx = this->grafo->nodo[m2]->mesh->pontos[i].x - x;
		dy = this->grafo->nodo[m2]->mesh->pontos[i].y - y;
		temp = sqrt(dx*dx + dy*dy);
		///Distancia do ponto i ao ponto final
		dx = x_dest - this->grafo->nodo[m2]->mesh->pontos[i].x;
		dy = y_dest - this->grafo->nodo[m2]->mesh->pontos[i].y;
		temp += sqrt(dx*dx + dy*dy);
		inimigo_enxerga = false;
		for (int j = 0; j < num_inimigos; j++) {
			if (!inimigos[j]->morto) {
				inimigo_enxerga = inimigo_enxerga || linha_cruza_triangulo (m1_p, &this->grafo->nodo[m2]->mesh->pontos[i], inimigos[j]->visao.pontos);
				inimigo_enxerga = inimigo_enxerga || linha_cruza_triangulo (&this->grafo->nodo[m2]->mesh->pontos[i], &p_end, inimigos[j]->visao.pontos);
			}
		}
			
		if (!inimigo_enxerga && (temp < dist_temp || dist_temp == -1)) {
			dist_temp = temp;
			p->x = this->grafo->nodo[m2]->mesh->pontos[i].x;
			p->y = this->grafo->nodo[m2]->mesh->pontos[i].y;
		}
		///Testa o ponto intermediario entre i e i+1
		if ((i == 0 || i == 2) && this->grafo->nodo[m2]->mesh->tipo != 3) {
			aux->x = (this->grafo->nodo[m2]->mesh->pontos[i].x + this->grafo->nodo[m2]->mesh->pontos[i+1].x) / 2.0;
			aux->y = (this->grafo->nodo[m2]->mesh->pontos[i].y + this->grafo->nodo[m2]->mesh->pontos[i+1].y) / 2.0;
			
			dx = aux->x - x;
			dy = aux->y - y;
			temp = sqrt(dx*dx + dy*dy);
			dx = x_dest - aux->x;
			dy = y_dest - aux->y;
			temp += sqrt(dx*dx + dy*dy);
			inimigo_enxerga = false;
			for (int j = 0; j < num_inimigos; j++) {
				if (!inimigos[j]->morto) {
					inimigo_enxerga = linha_cruza_triangulo (m1_p, aux, inimigos[j]->visao.pontos);
					inimigo_enxerga = inimigo_enxerga || linha_cruza_triangulo (aux, &p_end, inimigos[j]->visao.pontos);
				}
			}
			
			if (!inimigo_enxerga && (temp < dist_temp || dist_temp == -1)) {
				dist_temp = temp;
				p->x = aux->x;
				p->y = aux->y;
			}
		}
		///Testa o ponto intermediario entre i e i+2
		if ((i == 0 || i == 1) && this->grafo->nodo[m2]->mesh->tipo != 3) {
			aux->x = (this->grafo->nodo[m2]->mesh->pontos[i].x + this->grafo->nodo[m2]->mesh->pontos[i+2].x) / 2.0;
			aux->y = (this->grafo->nodo[m2]->mesh->pontos[i].y + this->grafo->nodo[m2]->mesh->pontos[i+2].y) / 2.0;
			
			dx = aux->x - x;
			dy = aux->y - y;
			temp = sqrt(dx*dx + dy*dy);
			dx = x_dest - aux->x;
			dy = y_dest - aux->y;
			temp += sqrt(dx*dx + dy*dy);
			inimigo_enxerga = false;
			for (int j = 0; j < num_inimigos; j++) {
				if (!inimigos[j]->morto) {
					inimigo_enxerga = linha_cruza_triangulo (m1_p, aux, inimigos[j]->visao.pontos);
					inimigo_enxerga = inimigo_enxerga || linha_cruza_triangulo (aux, &p_end, inimigos[j]->visao.pontos);
				}
			}
			
			if (!inimigo_enxerga && (temp < dist_temp || dist_temp == -1)) {
				dist_temp = temp;
				p->x = aux->x;
				p->y = aux->y;
			}
		}
		///Testa o ponto intermediario entre i e i+3
		if (i == 0 && this->grafo->nodo[m2]->mesh->tipo != 3) {
			aux->x = (this->grafo->nodo[m2]->mesh->pontos[i].x + this->grafo->nodo[m2]->mesh->pontos[i+3].x) / 2.0;
			aux->y = (this->grafo->nodo[m2]->mesh->pontos[i].y + this->grafo->nodo[m2]->mesh->pontos[i+3].y) / 2.0;
			
			dx = aux->x - x;
			dy = aux->y - y;
			temp = sqrt(dx*dx + dy*dy);
			dx = x_dest - aux->x;
			dy = y_dest - aux->y;
			temp += sqrt(dx*dx + dy*dy);
			inimigo_enxerga = false;
			for (int j = 0; j < num_inimigos; j++) {
				if (!inimigos[j]->morto) {
					inimigo_enxerga = linha_cruza_triangulo (m1_p, aux, inimigos[j]->visao.pontos);
					inimigo_enxerga = inimigo_enxerga || linha_cruza_triangulo (aux, &p_end, inimigos[j]->visao.pontos);
				}
			}
			
			if (!inimigo_enxerga && (temp < dist_temp || dist_temp == -1)) {
				dist_temp = temp;
				p->x = aux->x;
				p->y = aux->y;
			}
		}
	}
	
	///Caso todos os pontos passam pelo inimigo
	if (dist_temp == -1) {
		p->x = this->grafo->nodo[m2]->mesh->pontos[0].x;
		p->y = this->grafo->nodo[m2]->mesh->pontos[0].y;
	}
	
	free(aux);
	
	return p;
}
//----------------------------------------------------------------------
void Fugitivo::gera_caminho_exato (Inimigo **inimigos, int num_inimigos) {
	Ponto_Mapa *c;
	Ponto_Mapa *ultimo_ponto = (Ponto_Mapa*) malloc (sizeof(Ponto_Mapa));
	///Insere o primeiro ponto duas vezes para a Bspline começar no ponto inicial
	this->path.caminho_exato->insere_ponto(this->path.start.x, this->path.start.y, this->path.mesh_start);
	this->path.caminho_exato->insere_ponto(this->path.start.x, this->path.start.y, this->path.mesh_start);
	ultimo_ponto->x = this->path.start.x;
	ultimo_ponto->y = this->path.start.y;
	
	for (Item *p = this->path.caminho_mesh->first; p->prox != NULL; p = p->prox) {
		///Se for uma mesh de cobertura, adiciona-se dois pontos para a Bspline passar dentro desta mesh
		if (p->prox && (this->grafo->nodo[p->prox->id]->mesh->tipo == 1 || this->grafo->nodo[p->prox->id]->mesh->tipo == 2)) {
			if (p->prox->id != this->path.mesh_end) {
				this->path.caminho_exato->insere_ponto(grafo->nodo[p->prox->id]->x, this->grafo->nodo[p->prox->id]->y, p->prox->id);
				ultimo_ponto->x = this->grafo->nodo[p->prox->id]->x;
				ultimo_ponto->y = this->grafo->nodo[p->prox->id]->y;
			}
		}
		else if (p->prox && p->prox->prox) {
			c = this->encontra_ponto_controle (ultimo_ponto, p->prox->id, p->prox->prox->id, inimigos, num_inimigos);
			this->path.caminho_exato->insere_ponto(c->x, c->y, p->prox->id);
			ultimo_ponto->x = c->x;
			ultimo_ponto->y = c->y;
			free(c);
		}
		///Insere um ponto de controle para evitar que a bspline passe por dentro de uma parede
		else if (p->prox) {
			c = this->encontra_ponto_controle (ultimo_ponto, p->prox->id, -1, inimigos, num_inimigos);
			this->path.caminho_exato->insere_ponto(c->x, c->y, p->prox->id);
			ultimo_ponto->x = c->x;
			ultimo_ponto->y = c->y;
			free(c);
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
void Fugitivo::encontra_caminho_sem_ser_visto (Cenario *cenario, Inimigo **inimigo, int num_inimigos, Ponto_Mapa *p) {
	int mesh_penalizada;
	
	if (this->furtivo)
		this->grafo->set_modo(true);
	else
		this->grafo->set_modo(false);
	
	if (p) {
		this->caminho_errado = true;
		this->altera_velocidade(false, inimigo, num_inimigos);
		//printf("FUGITIVO PENALIZA\n");
		mesh_penalizada = this->mesh_caminho_mais_proxima (p);
		//if (mesh_penalizada == -1)
			//printf("Merda no Fugitivo...\n");
		this->grafo->penaliza_transicao_mesh (mesh_penalizada, this->path.mesh_end);
		this->inicializa_dados(false);
		this->existe_caminho = this->gera_caminho_mesh();
		if (this->existe_caminho) {
			this->altera_velocidade(true, inimigo, num_inimigos);
			this->gera_caminho_exato(inimigo, num_inimigos);
			free(p);
			p = NULL;
			//if (this->path.caminho_exato->pontos == NULL)
				//printf("3 - Caminho exato do Fugitivo esta vazio!\n");
		}
	}
}
//----------------------------------------------------------------------
void Fugitivo::gera_caminho (Cenario *cenario, Inimigo **inimigo, int num_inimigos) {
	
	if (this->furtivo)
		this->grafo->set_modo(true);
	else
		this->grafo->set_modo(false);
		
	if (!this->existe_caminho) {
		this->caminho_errado = false;
		this->inicializa_dados(true);
	}
	
	Ponto_Mapa *p = NULL;
	
	if (!this->caminho_errado) {
		this->inicializa_dados(true);

		this->existe_caminho = this->gera_caminho_mesh();	
		if (this->existe_caminho) {
			this->altera_velocidade(true, inimigo, num_inimigos);
			this->gera_caminho_exato(inimigo, num_inimigos);
			//if (this->path.caminho_exato->pontos == NULL)
				//printf("1 - Caminho exato do Fugitivo esta vazio!\n");
		}
	}
	
	if (!this->existe_caminho) {
		return;
	}

	p = this->verifica_caminho_valido(cenario, inimigo, num_inimigos);
	
	if (p) 
		encontra_caminho_sem_ser_visto(cenario, inimigo, num_inimigos, p);
	else {
		this->caminho_errado = false;
		this->altera_velocidade(true, inimigo, num_inimigos);
	}
	
	//if (this->entre_coberturas && this->path.mesh_atual != this->path.mesh_objetivo)
	if (!this->entre_coberturas)
		this->path.mesh_objetivo = this->path.mesh_end;
		
	//if (this->path.caminho_exato->pontos == NULL)
		//printf("2 - Caminho exato do Fugitivo esta vazio!\n");
}
//----------------------------------------------------------------------
Ponto_Mapa* Fugitivo::verifica_visao_inimigos (Cenario *cenario, Inimigo **inimigo, int num_inimigos, bool check_path) {
	Ponto_Mapa aux;
	float dx, dy, dist;
	bool visto = false;
	Ponto_Mapa *ponto_encontro = (Ponto_Mapa*) malloc (sizeof(Ponto_Mapa));
	
	///Percorre todos os inimigos
	for (int i = 0; i < num_inimigos; i++) {
		if (!inimigo[i]->morto) {
			if (inimigo[i]->estado_visao != VERIFICA_BARULHO)
				inimigo[i]->estado_visao = NEUTRO;
			///Verifica inicialmente se o inimigo vê o agente
			if (inimigo[i]->enxerga_ponto(&this->path.atual, cenario->paredes, cenario->num_parede, false) || (inimigo[i]->path.atual.x == this->path.atual.x && inimigo[i]->path.atual.y == this->path.atual.y)) {
				if (inimigo[i]->estado_visao != VERIFICA_BARULHO)
					inimigo[i]->estado_visao = VISUALIZA_AGENTE;
				visto = true;
				this->encontrado = true;
				ponto_encontro->x = this->path.atual.x;
				ponto_encontro->y = this->path.atual.y;
				this->existe_caminho = false;
				break;
			}
			if (check_path) {
				dx = this->path.atual.x - inimigo[i]->path.atual.x;
				dy = this->path.atual.y - inimigo[i]->path.atual.y;
				dist = sqrt(dx*dx + dy*dy); 
				if (dist < RAIO_VISAO && this->existe_caminho) {
					///Percorre todos os pontos da Bspline
					for (int k = this->path.p_atual_bspline; k >= 0 ; k--) {
						aux.x = this->path.caminho_exato->bspline[k].x;
						aux.y = this->path.caminho_exato->bspline[k].y;
						///Verifica se o inimigo enxerga o caminho que o agente executará
						if (inimigo[i]->enxerga_ponto(&aux, cenario->paredes, cenario->num_parede, false)) {
							if (inimigo[i]->estado_visao != VERIFICA_BARULHO)
								inimigo[i]->estado_visao = VISUALIZA_CAMINHO;
							visto = true;
							ponto_encontro->x = aux.x;
							ponto_encontro->y = aux.y;
							break;
						}
					}
				}
			}
		}
	}
	
	if (!visto) {
		free(ponto_encontro);
		ponto_encontro = NULL;
	}
	
	return ponto_encontro;
}
//----------------------------------------------------------------------
Ponto_Mapa* Fugitivo::verifica_caminho_valido (Cenario *cenario, Inimigo **inimigo, int num_inimigos) {
	Ponto_Mapa *p = NULL;
	p = this->caminho_atravessa_parede(cenario);
	if (p == NULL) 
		return this->verifica_visao_inimigos(cenario, inimigo, num_inimigos, true);
		
	return p;
}
//----------------------------------------------------------------------
void Fugitivo::atualiza_dados (Cenario *cenario, Inimigo **inimigo, int num_inimigos) {
	//float dx, dy, dist;
	
	///Atualizações em intervalos de tempo menores
	if (this->tempo_recalcula_1 == 0) {
		///Redefine o tempo para a atualização de dados
		this->tempo_recalcula_1 = TEMPO_RECALCULA_1;
		///Encontra o inimigo mais próximo do agente
		this->atualiza_distancias_inimigos(inimigo, num_inimigos);
		
		if (this->existe_caminho) {
			Ponto_Mapa *p = verifica_visao_inimigos(cenario, inimigo, num_inimigos, true);
			///Se algum inimigo estiver no seu caminho
			if (p) {
				encontra_caminho_sem_ser_visto(cenario, inimigo, num_inimigos, p);
			}
			else {
				this->altera_velocidade(true, inimigo, num_inimigos);
			}
			///Verifica se o ponto de destino encontra-se no campo de visão de algum inimigo
			/*for (int i = 0; i < num_inimigos; i++) {
				if (!inimigo[i]->morto) {
					dx = this->path.atual.x - inimigo[i]->path.atual.x;
					dy = this->path.atual.y - inimigo[i]->path.atual.y;
					dist = sqrt(dx*dx + dy*dy); 
					if (dist < RAIO_VISAO) {
						if (inimigo[i]->enxerga_ponto(&this->path.end, cenario->paredes, cenario->num_parede, false)) {
							this->existe_caminho = false;
							return;
						}
					}
				}
			}*/	
			///Atualiza destino da perseguição (se o agente estiver perseguindo alguém)
			if (this->perseguindo) 
				this->persegue_mais_proximo(cenario, inimigo, num_inimigos);
				
			///Emite barulho ao andar
			if (this->path.velocidade > 0.0)
				this->faz_barulho_passo(cenario, inimigo, num_inimigos);
		}
	}
	else {
		this->tempo_recalcula_1--;
	}
	///Atualizações em intervalos de tempo maiores
	if (this->tempo_recalcula_2 == 0) {
		///Redefine o tempo para a atualização de dados
		this->tempo_recalcula_2 = TEMPO_RECALCULA_2;
		///Atualiza a distancia das paredes
		this->atualiza_distancias_parede(cenario);
		
		if (this->existe_caminho) {
			if (!this->perseguindo)
				this->gera_caminho(cenario, inimigo, num_inimigos);
		}
		
		this->matou_recente = SEM_TENTATIVA;
	}
	else {
		this->tempo_recalcula_2--;
	}
}
//----------------------------------------------------------------------
void Fugitivo::encontra_prox_cobertura () {
	this->furtivo = true;
	this->perseguindo = false;
	this->entre_coberturas = true;
	if (this->existe_caminho) {
		Lista *aux, *inicio;
		///Percorre os pontos de controle de trás pra frente
		if (this->path.caminho_exato->pontos == NULL) {
				//printf("Caminho vazio - Caminho errado = %d\n", this->caminho_errado);
		}
		for (aux = this->path.caminho_exato->pontos; aux->prox != NULL; aux = aux->prox) { }
		///Encontra o ponto em que o agente está
		for (inicio = aux; inicio != NULL && inicio->num != this->path.mesh_atual; inicio = inicio->ant) {  }
		///Encontra o final da cobertura atual em que o agente se encontra
		for (aux = inicio; aux != NULL && (this->grafo->nodo[aux->num]->mesh->tipo == 1 || this->grafo->nodo[aux->num]->mesh->tipo == 2); aux = aux->ant) {  }
		///Percorre todos os pontos de controle da Bspline
		for (Lista *l = aux; l != NULL; l = l->ant) {
			if (this->grafo->nodo[l->num]->mesh->tipo == 1) {
				this->path.mesh_objetivo = l->num;
				return;
			}
		}
		
		this->path.mesh_objetivo = this->path.mesh_end;
	}
}
//----------------------------------------------------------------------
void Fugitivo::atualiza_distancias_parede (Cenario *cenario) {
	float dx, dy, dist;
	for (int i = 0; i < cenario->num_parede; i++) {
		dx = this->path.atual.x - cenario->paredes[i]->x;
		dy = this->path.atual.y - cenario->paredes[i]->y;
		dist = sqrt(dx*dx + dy*dy);
		if (dist <= DIST_MUITO_PROX)
			this->paredes_proximas[i] = PAREDE_MUITO_PROX;
		else if (dist <= DIST_PROX)
			this->paredes_proximas[i] = PAREDE_PROX;
		else if (dist <= DIST_MEDIA)
			this->paredes_proximas[i] = PAREDE_MEDIO;
		else
			this->paredes_proximas[i] = PAREDE_DISTANTE;
	}
}
//----------------------------------------------------------------------
void Fugitivo::atualiza_distancias_inimigos (Inimigo** inimigo, int num_inimigos) {
	float menor_dist = -1, dist, dist_ant = -1, dx, dy;
	int alvo = -1, prox = 0, m_prox = 0, medio = 0;
	int m_prox_aprox = 0, prox_aprox = 0, medio_aprox = 0;
	
	for (int i = 0; i < num_inimigos; i++) {
		if (!inimigo[i]->morto) {
			dist_ant = -1;
			dx = this->path.atual.x - inimigo[i]->path.atual.x;
			dy = this->path.atual.y - inimigo[i]->path.atual.y;
			dist = sqrt(dx*dx + dy*dy);
			///Calcula a distância anterior do inimigo
			if (inimigo[i]->path.p_atual_bspline < inimigo[i]->path.caminho_exato->pontos_discretizacao) {
				dx = this->path.atual.x - inimigo[i]->path.caminho_exato->bspline[inimigo[i]->path.p_atual_bspline + 1].x;
				dy = this->path.atual.y - inimigo[i]->path.caminho_exato->bspline[inimigo[i]->path.p_atual_bspline + 1].y;
				dist_ant = sqrt(dx*dx + dy*dy);
			}
			///Encontra o posicionamento dos inimigos e quais estão se aproximando do agente
			if (dist <= INIMIGO_MUITO_PROX) {
				m_prox++;
				///Verifica se o inimigo está se aproximando
				if (dist_ant != -1 && dist_ant > dist) 
					m_prox_aprox++;
			}
			else if (dist <= INIMIGO_PROX) {
				prox++;
				///Verifica se o inimigo está se aproximando
				if (dist_ant != -1 && dist_ant > dist) 
					prox_aprox++;
			}
			else if (dist <= INIMIGO_MEDIO) {
				medio++;
				///Verifica se o inimigo está se aproximando
				if (dist_ant != -1 && dist_ant > dist) 
					medio_aprox++;
			}
			///Procura o inimigo mais próximo do agente
			if (menor_dist > dist || menor_dist == -1) {
				menor_dist = dist;
				alvo = i;
			}
		}
	}
	
	this->inimigo_mais_prox = alvo;
	
	///Define o estado atual do posicionamento dos inimigos
	if (m_prox == 0 && prox == 0 && medio == 0) 
		this->estado_posicionamento = 0;
	else if (m_prox == 1 && prox == 0 && medio == 0)
		this->estado_posicionamento = 1;
	else if (m_prox == 1 && prox >= 1)
		this->estado_posicionamento = 2;
	else if (m_prox == 1 && prox == 0 && medio >= 1)
		this->estado_posicionamento = 3;
	else if (m_prox >= 2)
		this->estado_posicionamento = 4;
	else if (m_prox == 0 && prox >= 1 && medio == 0)
		this->estado_posicionamento = 5;
	else if (m_prox == 0 && prox >= 1 && medio >= 1)
		this->estado_posicionamento = 6;
	else if (m_prox == 0 && prox == 0 && medio >= 1)
		this->estado_posicionamento = 7;
		
	///Define o estado referente a quais inimigos estão se aproximando do agente
	if (m_prox_aprox == 0 && prox_aprox == 0 && medio_aprox == 0)
		this->estado_aproximacao = 0;
	else if (m_prox_aprox == 0 && prox_aprox == 0 && medio_aprox >= 1)
		this->estado_aproximacao = 1;
	else if (m_prox_aprox == 0 && prox_aprox >= 1)
		this->estado_aproximacao = 2;
	else if (m_prox_aprox >= 1)
		this->estado_aproximacao = 3;
		
}
//----------------------------------------------------------------------
void Fugitivo::encontra_esconderijo (Cenario *cenario, Inimigo **inimigo, int num_inimigos) {
	
	Ponto_Mapa p1, p_final;
	float mais_prox = -1, min_dist_p1, dx, dy, dist, dist_agente, min_dist_agente = -1;
	float tam_parede;
	int mesh_destino;
	int ve_p_final = -1, ve_p1;
	bool encontrado_nivel_anterior = false;
	int nivel_max = PAREDE_MEDIO;
	
	this->atualiza_distancias_parede(cenario);
	///Indica qual o nivel de distancia procurado atualmente (0, 1 ou 2) - OBS: Nao procura-se cobertura em paredes distantes (3)
	for (int nivel_atual = PAREDE_MUITO_PROX; nivel_atual <= nivel_max; nivel_atual++) {
		///Se um bom esconderijo já foi encontrado
		if (encontrado_nivel_anterior) {
			this->caminho_errado = false;
			this->path.mesh_end = mesh_destino;
			this->path.end.x = p_final.x;
			this->path.end.y = p_final.y;
			this->existe_caminho = true;
			this->gera_caminho(cenario, inimigo, num_inimigos);
			return;
		}
		///Procura qual parede está a uma distancia igual ao nivel atual
		for (int i = 0; i < cenario->num_parede; i++) {
			if (this->paredes_proximas[i] == nivel_atual) {
				///Encontra quais são as meshes de cobertura da parede 'i'
				for (int k = 0; k < cenario->nav_mesh->num_meshes; k++) {
					///Se a mesh for do tipo 0, todas as próximas meshes serão do tipo 0
					if (cenario->nav_mesh->meshes[k]->tipo == 0)
						break;
					else if (cenario->nav_mesh->meshes[k]->tipo != 3 && cenario->nav_mesh->meshes[k]->parede_pertencente == i) {
						p1.x = (cenario->nav_mesh->meshes[k]->pontos[0].x + cenario->nav_mesh->meshes[k]->pontos[3].x) / 2.0;
						p1.y = (cenario->nav_mesh->meshes[k]->pontos[0].y + cenario->nav_mesh->meshes[k]->pontos[3].y) / 2.0;
						
						///Calcula o tamanho da parede em que o agente vai se esconder (pega o tamanho do maior lado da parede)
						dx = cenario->nav_mesh->meshes[k]->pontos[0].x - cenario->nav_mesh->meshes[k]->pontos[1].x;
						dy = cenario->nav_mesh->meshes[k]->pontos[0].y - cenario->nav_mesh->meshes[k]->pontos[1].y;
						tam_parede = sqrt(dx*dx + dy*dy);
						dx = cenario->nav_mesh->meshes[k]->pontos[0].x - cenario->nav_mesh->meshes[k]->pontos[2].x;
						dy = cenario->nav_mesh->meshes[k]->pontos[0].y - cenario->nav_mesh->meshes[k]->pontos[2].y;
						if (sqrt(dx*dx + dy*dy) > tam_parede)
							tam_parede = sqrt(dx*dx + dy*dy);
						///Acrescenta um valor de segurança no tamanho da parede
						tam_parede += 2*LARGURA_MESH_COVER;
						
						///Calcula a distância da mesh até o agente
						dx = p1.x - this->path.atual.x;
						dy = p1.y - this->path.atual.y;
						dist_agente = sqrt(dx*dx + dy*dy);
							
						///Verifica a segurança da mesh em relação a todos os inimigos
						ve_p1 = 0;	min_dist_p1 = -1;
						for (int a = 0; a < num_inimigos; a++) {
							if (!inimigo[a]->morto) {
								dx = p1.x - inimigo[a]->path.atual.x;
								dy = p1.y - inimigo[a]->path.atual.y;
								dist = sqrt(dx*dx + dy*dy);
								if (dist < min_dist_p1 || min_dist_p1 == -1)
									min_dist_p1 = dist;
								//if (dist <= tam_parede || (dist <= INIMIGO_PROX && inimigo[a]->enxerga_ponto_visao_ilimitada(&p1, cenario->paredes, i))) 
								if (dist <= INIMIGO_PROX && inimigo[a]->enxerga_ponto_visao_ilimitada(&p1, cenario->paredes, i)) 
									ve_p1++;			
							}
						}
						if (ve_p1 == 0 && (dist_agente <= min_dist_agente || min_dist_agente == -1)) {
							min_dist_agente = dist_agente;
							mesh_destino = k;
							p_final.x = p1.x;
							p_final.y = p1.y;
							encontrado_nivel_anterior = true;
						}
						else if (!encontrado_nivel_anterior) {
							if (ve_p1 <= ve_p_final || ve_p_final == -1) {
								if (ve_p1 == ve_p_final) {
									if (min_dist_p1 > mais_prox) {
										mais_prox = min_dist_p1;
										mesh_destino = k;
										p_final.x = p1.x;
										p_final.y = p1.y;
									}
								}
								else {
									mais_prox = min_dist_p1;
									ve_p_final = ve_p1;
									mesh_destino = k;
									p_final.x = p1.x;
									p_final.y = p1.y;
								}
							}
						}
					}
				}
			}
		}
		
		if (mesh_destino == -1 && nivel_atual == nivel_max) {
			if (nivel_max != PAREDE_DISTANTE) {
				nivel_max++;
				nivel_atual = 0;
			}
			else
				mesh_destino = this->path.mesh_atual;
		}
	}
	
	//printf("P FINAL na mesh %d = (%f, %f)\n", mesh_destino, p_final.x, p_final.y);
	this->caminho_errado = false;
	this->path.mesh_end = mesh_destino;
	this->path.end.x = p_final.x;
	this->path.end.y = p_final.y;
	this->existe_caminho = true;
	this->gera_caminho(cenario, inimigo, num_inimigos);
	return;
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
			if (!inimigo[i]->morto) {
				dx = this->path.atual.x - inimigo[i]->path.atual.x;
				dy = this->path.atual.y - inimigo[i]->path.atual.y;
				dist = sqrt(dx*dx + dy*dy);
				if (dist <= raio_som && inimigo[i]->estado_visao != VISUALIZA_AGENTE) {
					printf("Barulho!\n");
					inimigo[i]->estado_visao = VERIFICA_BARULHO;
					inimigo[i]->verifica_pertubacao(cenario, &this->path.atual, this->path.mesh_atual);
				}
			}
		}
	}
}
//----------------------------------------------------------------------
void Fugitivo::faz_barulho (Cenario *cenario, Inimigo **inimigo, int num_inimigos) {
	float dx, dy, dist;
	int raio_som = RAIO_SOM_DISTRACAO;
		
	for (int i = 0; i < num_inimigos; i++) {
		if (!inimigo[i]->morto) {
			dx = this->path.atual.x - inimigo[i]->path.atual.x;
			dy = this->path.atual.y - inimigo[i]->path.atual.y;
			dist = sqrt(dx*dx + dy*dy);
			if (dist <= raio_som && inimigo[i]->estado_visao != VISUALIZA_AGENTE) {
				inimigo[i]->estado_visao = VERIFICA_BARULHO;
				inimigo[i]->verifica_pertubacao(cenario, &this->path.atual, this->path.mesh_atual);
			}
		}
	}
}
//----------------------------------------------------------------------
void Fugitivo::persegue_mais_proximo (Cenario *cenario, Inimigo **inimigo, int num_inimigos) {
	
	this->perseguindo = true;
	this->entre_coberturas = false;
	int alvo = this->inimigo_mais_prox;
	if (alvo != -1) {
		///Caminha até o último ponto que o alvo passou
		Ponto_Mapa p;
		int mesh;
		p.x = inimigo[alvo]->path.caminho_exato->bspline[inimigo[alvo]->path.p_atual_bspline + 1].x;
		p.y = inimigo[alvo]->path.caminho_exato->bspline[inimigo[alvo]->path.p_atual_bspline + 1].y;
		mesh = this->grafo->encontra_mesh(&p);
		this->caminho_errado = false;
		this->path.mesh_end = mesh;
		this->path.end.x = p.x;
		this->path.end.y = p.y;
		this->existe_caminho = true;
		this->furtivo = false;
		this->gera_caminho(cenario, inimigo, num_inimigos);		
	}
}
//----------------------------------------------------------------------
void Fugitivo::matar_inimigo_prox (Cenario* cenario, Inimigo **inimigo, int num_inimigos) {
	int chance;
	
	if (this->inimigo_mais_prox != -1 && this->visao_livre_inimigo(cenario, inimigo, this->inimigo_mais_prox)) {
		float dist, dx, dy;
		dx = this->path.atual.x - inimigo[this->inimigo_mais_prox]->path.atual.x;
		dy = this->path.atual.y - inimigo[this->inimigo_mais_prox]->path.atual.y;
		dist = sqrt(dx*dx + dy*dy);
		if (dist <= DIST_TIRO_PERTO)
			chance = 100 * PCT_ACERTO_PERTO;
		else if (dist <= DIST_TIRO_MEDIO)
			chance = 100 * PCT_ACERTO_MEDIO;
		else if (dist <= DIST_TIRO_LONGE)
			chance = 100 * PCT_ACERTO_LONGE;
		else
			chance = -1;
		
		int n = rand() % 101;	///Valor entre 0 a 100
		
		if (n <= chance) {
			inimigo[this->inimigo_mais_prox]->morto = true;
			this->matou_recente = ACERTOU_TIRO;
		}
		else {
			this->matou_recente = ERROU_TIRO;
			inimigo[this->inimigo_mais_prox]->verifica_pertubacao(cenario, &this->path.atual, this->path.mesh_atual);
			for (int i = 0; i < num_inimigos; i++) {
				if (!inimigo[i]->morto) 
					inimigo[i]->entra_alerta();
			}
		}
	}
	else
		this->matou_recente = SEM_VISAO;
}
//----------------------------------------------------------------------
bool Fugitivo::visao_livre_inimigo (Cenario* cenario, Inimigo** inimigo, int id) {
	if (id == -1)
		return false;
		
	return inimigo[id]->enxerga_ponto(&this->path.atual, cenario->paredes, cenario->num_parede, true);
}
//----------------------------------------------------------------------
int Fugitivo::dist_inimigo_prox (Inimigo** inimigo) {
	if (this->inimigo_mais_prox != -1) {
		float dist, dx, dy;
		dx = this->path.atual.x - inimigo[this->inimigo_mais_prox]->path.atual.x;
		dy = this->path.atual.y - inimigo[this->inimigo_mais_prox]->path.atual.y;
		dist = sqrt(dx*dx + dy*dy);
		if (dist <= DIST_TIRO_PERTO)
			return PERTO;
	}
	
	return LONGE;
}
//----------------------------------------------------------------------
bool Fugitivo::inimigo_prox_parado (Inimigo** inimigo) {
	if (this->inimigo_mais_prox != -1)
		return (inimigo[this->inimigo_mais_prox]->path.terminou_caminho && inimigo[this->inimigo_mais_prox]->espera > 0);
	
	return true;
}














