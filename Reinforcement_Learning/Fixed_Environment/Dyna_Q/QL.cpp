#include <stdio.h>
#include <stdlib.h>
#include <math.h> 
#include <unistd.h>
#include <time.h>
#include "QL.hpp"


Fugitivo_RL::Fugitivo_RL(Cenario *cenario, char* nome_arq, float aprendizado, float desconto, float exploracao) {
	this->grafo = new Grafo(cenario->nav_mesh);
	this->em_cobertura = false;
	this->existe_caminho = true;
	this->caminho_errado = false;
	this->encontrado = false;
	this->path.velocidade = MODULO_VELOCIDADE_SNEAK;
	this->tempo_recalcula_1 = 0;
	this->tempo_recalcula_2 = TEMPO_RECALCULA_2;
	this->tempo_recalcula_3 = TEMPO_RECALCULA_3;
	this->entre_coberturas = true;
	this->estado_aproximacao = 0;
	this->estado_posicionamento = 0;
	this->paredes_proximas = (int*) malloc (sizeof(int)*cenario->num_parede);
	this->atualiza_distancias_parede(cenario);
	this->perseguindo = false;
	this->furtivo = true;
	this->inimigo_mais_prox = 0;
	this->matou_recente = SEM_TENTATIVA;
	this->em_alerta = false;
	this->chegou_destino = false;
	this->num_estados = VAL_DIST_DESTINO*VAL_POS_INIMIGOS*VAL_CAMINHO_SEGURO*VAL_INIMIGO_APROX*VAL_VISAO_LIVRE*VAL_DIST_PROX*VAL_INIMIGO_PARADO;
	
	this->QValues = (float**) malloc (this->num_estados*sizeof(float*));
	this->Modelo = (RS**) malloc (this->num_estados*sizeof(RS*));
	for (int i = 0; i < this->num_estados; i++) {
		this->QValues[i] = (float*) malloc (NUM_ACAO*sizeof(float));
		this->Modelo[i] = (RS*) malloc (NUM_ACAO*sizeof(RS));
		for (int j = 0; j < NUM_ACAO; j++) {
			this->QValues[i][j] = QVALUE_INICIAL;
			this->Modelo[i][j].reward = 0;
			this->Modelo[i][j].estado_destino = -1;
		}
	}
	
	if (nome_arq) 
		this->carrega_QValues(nome_arq);
	
	this->alpha = aprendizado;
	this->gamma = desconto;
	this->phi = exploracao;
	
	this->acao_anterior = PROX_COBERTURA;
	this->estado_anterior = NULL;
}

Fugitivo_RL::~Fugitivo_RL() {
	
}
//----------------------------------------------------------------------
void Fugitivo_RL::savla_QValues (char* arq_saida) {
	char nome[200];
	FILE *arq = NULL;
	
	if (arq_saida == NULL)  {
		sprintf(nome, "QValues/QValues_%1.3f_%1.1f_%1.1f.txt", this->alpha, this->gamma, this->phi);
		arq = fopen(nome, "w+");
	}
	else {
		arq = fopen(arq_saida, "w+");
	}
	for (int i = 0; i < this->num_estados; i++) {
		for (int j = 0; j < NUM_ACAO; j++) {
			fprintf(arq, "%f\t", this->QValues[i][j]);
		}
		fprintf(arq,"\n");
	}
	fclose(arq);
}
//----------------------------------------------------------------------
void Fugitivo_RL::carrega_QValues (char* nome_arq) {
	FILE *arq = fopen(nome_arq, "r");
	for (int i = 0; i < num_estados; i++) {
		for (int j = 0; j < NUM_ACAO; j++) {
			fscanf(arq, "%f\t", &this->QValues[i][j]);
		}
		fscanf(arq, "\n");
	}
	fclose(arq);
}
//----------------------------------------------------------------------
void Fugitivo_RL::restaura_valores (Cenario* cenario) {
	this->path.terminou_caminho = false;
	this->existe_caminho = true;
	this->caminho_errado = false;
	this->path.velocidade = MODULO_VELOCIDADE_SNEAK;
	this->tempo_recalcula_1 = 0;
	this->tempo_recalcula_2 = TEMPO_RECALCULA_2;
	this->tempo_recalcula_3 = TEMPO_RECALCULA_3;
	this->encontrado = false;
	this->estado_aproximacao = 0;
	this->estado_posicionamento = 0;
	free(this->paredes_proximas);
	this->paredes_proximas = (int*) malloc (sizeof(int)*cenario->num_parede);
	this->atualiza_distancias_parede(cenario);
	this->perseguindo = false;
	this->furtivo = true;
	this->inimigo_mais_prox = 0;
	this->matou_recente = SEM_TENTATIVA;
	this->chegou_destino = false;
	this->entre_coberturas = true;
	this->em_alerta = false;
	delete this->grafo;
	this->grafo = new Grafo(cenario->nav_mesh);
	
	this->acao_anterior = PROX_COBERTURA;
	if (this->estado_anterior)
		free(this->estado_anterior);
	this->estado_anterior = NULL;
}
//----------------------------------------------------------------------
int Fugitivo_RL::distancia_destino () {
	float dx, dy, dist;
	dx = this->p_destino.x - this->path.atual.x;
	dy = this->p_destino.y - this->path.atual.y;
	dist = sqrt(dx*dx + dy*dy);
	if (dist <= DIST_DESTINO_PERTO)
		return DESTINO_PERTO;
	else if (dist <= DIST_DESTINO_MEDIO)
		return DESTINO_MEDIO;

	return DESTINO_LONGE;
}
//----------------------------------------------------------------------
int Fugitivo_RL::acao_em_andamento (Inimigo** inimigo) {
	int valor;
	
	switch (this->acao_anterior) {
		case PROX_COBERTURA:
			if (this->path.mesh_objetivo != this->path.mesh_atual) 
				valor = EM_ANDAMENTO_PROX;
			else
				valor = EM_ANDAMENTO_NADA;
			break;
		case ESCONDE:
			if (this->path.mesh_end != this->path.mesh_atual)
				valor = EM_ANDAMENTO_ESCONDE;
			else
				valor = EM_ANDAMENTO_NADA;
			break;
		default:
			valor = EM_ANDAMENTO_NADA;
			break;
	}
	
	return valor;
}
//----------------------------------------------------------------------
void Fugitivo_RL::atualiza_dados (Cenario *cenario, Inimigo **inimigo, int num_inimigos) {
	Ponto_Mapa *p = NULL;
	
	///Verifica se o agente chegou no destino
	if (this->path.atual.x == this->p_destino.x && this->path.atual.y == this->p_destino.y) {
		this->chegou_destino = true;
		Estado* estado = this->define_estado_atual(cenario, inimigo, num_inimigos);
		float Q_prox_estado = QVALUE_MAX;
		float reforco = RCP_COMPLETA;
		
		if (this->estado_anterior != NULL) {
			QValues[this->estado_anterior->id][this->acao_anterior] = (1 - this->alpha)*QValues[this->estado_anterior->id][this->acao_anterior] + this->alpha*(reforco + this->gamma*Q_prox_estado);	
			if (QValues[this->estado_anterior->id][this->acao_anterior] > QVALUE_MAX) {
				QValues[this->estado_anterior->id][this->acao_anterior] = QVALUE_MAX;
			}
			else if (QValues[this->estado_anterior->id][this->acao_anterior] < QVALUE_MIN) {
				QValues[this->estado_anterior->id][this->acao_anterior] = QVALUE_MIN;
			}
			this->Modelo[this->estado_anterior->id][this->acao_anterior].reward = reforco;
			this->Modelo[this->estado_anterior->id][this->acao_anterior].estado_destino = estado->id;
		}
		
		free(estado);
		return;
	}
	
	///Verifica se o agente foi encontrado
	if (this->encontrado) {
		Estado* estado = this->define_estado_atual(cenario, inimigo, num_inimigos);
		float reforco = -1*PUN_ENCONTRADO;
		float Q_prox_estado = QVALUE_MIN;
		
		if (this->estado_anterior != NULL) {
			QValues[this->estado_anterior->id][this->acao_anterior] = (1 - this->alpha)*QValues[this->estado_anterior->id][this->acao_anterior] + this->alpha*(reforco + this->gamma*Q_prox_estado);	
			if (QValues[this->estado_anterior->id][this->acao_anterior] > QVALUE_MAX) {
				QValues[this->estado_anterior->id][this->acao_anterior] = QVALUE_MAX;
			}
			else if (QValues[this->estado_anterior->id][this->acao_anterior] < QVALUE_MIN) {
				QValues[this->estado_anterior->id][this->acao_anterior] = QVALUE_MIN;
			}
			this->Modelo[this->estado_anterior->id][this->acao_anterior].reward = reforco;
			this->Modelo[this->estado_anterior->id][this->acao_anterior].estado_destino = estado->id;
		}
		
		free(estado);
		return;
	}
	
	///Atualizações em intervalos de tempo menores
	if (this->tempo_recalcula_1 == 0) {
		this->tempo_recalcula_1 = TEMPO_RECALCULA_1;
		p = this->verifica_caminho_valido(cenario, inimigo, num_inimigos);
		///Encontra o inimigo mais próximo do agente
		this->atualiza_distancias_inimigos(inimigo, num_inimigos);
		
		if (this->existe_caminho) {
			///Se algum inimigo estiver no seu caminho
			if (p) {
				encontra_caminho_sem_ser_visto(cenario, inimigo, num_inimigos, p);
				p = NULL;
			}
			else {
				this->altera_velocidade(true, inimigo, num_inimigos);
			}
			///Atualiza destino da perseguição (se o agente estiver perseguindo alguém)
			if (this->perseguindo) 
				this->persegue_mais_proximo(cenario, inimigo, num_inimigos);
				
			///Emite barulho ao andar
			if (this->path.velocidade > 0.0)
				this->faz_barulho_passo(cenario, inimigo, num_inimigos);
		}
	}
	else {
		///Verifica se os inimigos conseguem ver o agente
		p = this->verifica_visao_inimigos(cenario, inimigo, num_inimigos, false);
		this->tempo_recalcula_1--;
	}
	
	
	
	///Atualizações em intervalos de tempo maiores
	if (this->tempo_recalcula_2 == 0) {
		this->tempo_recalcula_2 = TEMPO_RECALCULA_2;
		///Escolhe a próxima ação usando Aprendizado por Reforço
		this->prox_acao(cenario, inimigo, num_inimigos);
		///Marca que o inimigo não matou um inimigo recentemente (se tiver matado, já terá sido considerado no reforço aplicado na escolha da próxima ação)
		this->matou_recente = SEM_TENTATIVA;
	}
	else {
		this->tempo_recalcula_2--;
	}
	
	
	
	
	if (this->tempo_recalcula_3 == 0) {
		///Redefine o tempo para a atualização de dados
		this->tempo_recalcula_3 = TEMPO_RECALCULA_3;
		///Atualiza a distancia das paredes
		this->atualiza_distancias_parede(cenario);
	}
	else {
		this->tempo_recalcula_3--;
	}
	
	if (p)
		free(p);
}
//----------------------------------------------------------------------
Estado* Fugitivo_RL::define_estado_atual (Cenario* cenario, Inimigo** inimigo, int num_inimigos) {
	Estado *estado = (Estado*) malloc (sizeof(Estado));
	estado->dist_destino = this->distancia_destino();
	estado->pos_inimigos = this->estado_posicionamento;
	estado->seguro = this->existe_caminho;
	estado->aproxima = this->estado_aproximacao;
	estado->visao_livre = this->visao_livre_inimigo(cenario, inimigo, this->inimigo_mais_prox);
	estado->dist_prox = this->dist_inimigo_prox(inimigo);
	estado->parado = this->inimigo_prox_parado(inimigo);
	
	estado->id = 0;
	
	estado->id += estado->dist_destino*VAL_POS_INIMIGOS*VAL_CAMINHO_SEGURO*VAL_INIMIGO_APROX*VAL_VISAO_LIVRE*VAL_DIST_PROX*VAL_INIMIGO_PARADO;
	estado->id += estado->pos_inimigos*VAL_CAMINHO_SEGURO*VAL_INIMIGO_APROX*VAL_VISAO_LIVRE*VAL_DIST_PROX*VAL_INIMIGO_PARADO;
	estado->id += estado->seguro*VAL_INIMIGO_APROX*VAL_VISAO_LIVRE*VAL_DIST_PROX*VAL_INIMIGO_PARADO;
	estado->id += estado->aproxima*VAL_VISAO_LIVRE*VAL_DIST_PROX*VAL_INIMIGO_PARADO;
	estado->id += estado->visao_livre*VAL_DIST_PROX*VAL_INIMIGO_PARADO;
	estado->id += estado->dist_prox*VAL_INIMIGO_PARADO;
	estado->id += estado->parado;
	
	return estado;
}
//----------------------------------------------------------------------
int Fugitivo_RL::escolhe_melhor_acao (int estado) {
	float max = -2.0;
	int acao[NUM_ACAO];
	bool multi_max = false;
	
	for (int i = 0; i < NUM_ACAO; i++) 
		acao[i] = 0;
	
	for (int i=0; i<NUM_ACAO; i++) {
		if (QValues[estado][i] > max) {
			max = QValues[estado][i];
			multi_max = false;
			for (int j = 0; j < NUM_ACAO; j++) 
				acao[j] = 0;
			acao[i] = 1;
		}
		else if (QValues[estado][i] == max) {
			multi_max = true;
			acao[i] = 1;
		}
	}
	
	///Se existir diversos valores máximos, escolhe-se aquele que foi usado na acao anterior (se o mesmo for máximo também)
	if (multi_max && acao[this->acao_anterior] == 1) {
		return this->acao_anterior;
	}
	else {
		for (int i = 0; i < NUM_ACAO; i++) {
			if (acao[i] == 1)
				return i;
		}
	}
	
	return this->acao_anterior;
}
//----------------------------------------------------------------------
int Fugitivo_RL::escolhe_acao(int estado) {
	srand(time(NULL));
	float max = QVALUE_MIN;
	int num_valores_max = 0;
	for (int i=0; i<NUM_ACAO; i++) {
		if (QValues[estado][i] > max) {
			max = QValues[estado][i];
			num_valores_max = 1;
		}
		else if (QValues[estado][i] == max) 
			num_valores_max++;
	}

	int acao_escolhida, acao;
	if (num_valores_max != 0) {
		int explora = rand() % 11;
		if (explora < this->phi*10) {
			acao = rand() % NUM_ACAO;
		}
		else {
			acao_escolhida = 0;
			int cont = 0;
			for (int i=0; i<NUM_ACAO; i++) {
				if (QValues[estado][i] == max) {
					if (cont == acao_escolhida) {
						acao = i;
						break;
					}
					else 
						cont++;
				}
			}
		}
	}
	else
		return -1;
	
	return acao;
}
//----------------------------------------------------------------------
void Fugitivo_RL::aprende_modelo () {
	srand(time(NULL));
	for (int i = 0; i < N; i++) {
		int p = rand() % 11;
		int e = rand() % (int)(0.3*this->num_estados);
		int cont = 0, encontrado = false, state, action;
		
		bool prioridade = false;
		if (p < PRI_SWEEP)
			prioridade = true;
			
		///Encontra o par estado-ação a ser replicado por experiência
		for (int a = 0; a < this->num_estados; a++) {
			for (int b = 0; b < NUM_ACAO; b++) {
				if (this->Modelo[a][b].estado_destino != -1) {
					if (prioridade && this->Modelo[a][b].reward >= REWARD_PRI) {
						if (cont == e) {
							state = a;
							action = b;
							encontrado = true;
							break;
						}
						cont++;
					}
					else if (!prioridade) {
						if (cont == e) {
							state = a;
							action = b;
							encontrado = true;
							break;
						}
						cont++;
					}
				} 
			}
			if (encontrado)
				break;
			else if (a == this->num_estados-1) {
				a = 0;
				if (cont == 0)
					prioridade = false;
			}
		}
		
		
		float reforco = this->Modelo[state][action].reward;
		int s_linha = this->Modelo[state][action].estado_destino;
		int melhor_acao = this->escolhe_melhor_acao (s_linha); 
		float Q_prox_estado = QValues[s_linha][melhor_acao];
		
		//printf("%f -> ", QValues[state][action]); 
		QValues[state][action] = (1 - this->alpha)*QValues[state][action] + this->alpha*(reforco + this->gamma*Q_prox_estado);
		//printf("%f\n", QValues[state][action]);
		if (QValues[state][action] > QVALUE_MAX) {
			QValues[state][action] = QVALUE_MAX;
		}
		else if (QValues[state][action] < QVALUE_MIN) {
			QValues[state][action] = QVALUE_MIN;
		}
	}
}
//----------------------------------------------------------------------
float Fugitivo_RL::calcula_reforco(Inimigo** inimigo, int num_inimigos) {
	float reforco = 0.0;
	int dist_prox = this->estado_anterior->dist_prox;
	int aproxima = this->estado_anterior->aproxima;
	int pos_inimigos = this->estado_anterior->pos_inimigos;
	int dist_destino = this->estado_anterior->dist_destino;
	
	///Ensina o agente a se esconder quando não existir caminho até o destino
	if (!this->estado_anterior->seguro) {
		if (this->acao_anterior == ESCONDE)
			reforco += RCP_ESCONDE_CERTO;
		else
			reforco -= PUN_NAO_ESCONDE;
			
		return reforco;
	}
	///Reforço relativo ao agente atirar enquanto estiver muito próximo de um inimigo
	if (this->estado_anterior->visao_livre && dist_prox == PERTO) {
		if (this->acao_anterior == MATAR_INIMIGO)
			reforco += RCP_MATA;
		else
			reforco -= PUN_ERRA;
			
		return reforco;
	}
	///Reforço por não ir em direção ao destino quando não existe inimigo no cenario
	if (pos_inimigos == 0) {
		if (this->acao_anterior != PROX_COBERTURA)
			reforco -= PUN_PARADO_SEM_INIMIGO;
		else
			reforco += RCP_ANDA_SEM_INIMIGOS;
			
		return reforco;
	}
	///Ensina o agente a ir em direção ao objetivo quando estiver próximo de seu destino
	if (dist_destino == DESTINO_PERTO && aproxima != 3 && (pos_inimigos == 0 || pos_inimigos == 5 || pos_inimigos == 6 || pos_inimigos == 7)) {
		if (this->acao_anterior != PROX_COBERTURA)
			reforco -= PUN_AFASTA_DESTINO;
		else
			reforco += RCP_DESTINO_PERTO;
			
		return reforco;
	}
	///Ensina o agente a se esconder quando tiver inimigos se aproximando ou quando tiver inimigos por perto
	if ((aproxima == 3 || aproxima == 2) && pos_inimigos != 0 && pos_inimigos != 7) {
		if (this->acao_anterior == ESCONDE)
			reforco += RCP_ESCONDE_CERTO;
		else
			reforco -= PUN_NAO_ESCONDE;
			
		return reforco;
	}
	///Ensina o agente a perseguir quando tiver poucos inimigos e os mesmos estiverem se afastando
	if (dist_destino == DESTINO_LONGE && aproxima == 0 && pos_inimigos == 1) {
		if (this->acao_anterior == PERSEGUE) 
			reforco += RCP_PERSEGUE_CERTO;
		else
			reforco -= PUN_PERSEGUE_ERRADO;
			
		return reforco;
	}
	///Reforço relativo por andar em direção ao objetivo
	if (this->estado_anterior->seguro) {
		if (this->acao_anterior != PROX_COBERTURA)
			reforco -= PUN_ANDA_SEM_CAMINHO;
		else
			reforco += RCP_ANDA_DESTINO;
			
		return reforco;
	}
	
	return reforco;
}
//----------------------------------------------------------------------
void Fugitivo_RL::prox_acao (Cenario* cenario, Inimigo** inimigo, int num_inimigos) {
	Estado *estado = this->define_estado_atual(cenario, inimigo, num_inimigos);
	int acao = this->escolhe_acao(estado->id);
	int melhor_acao = this->escolhe_melhor_acao (estado->id); 
	float Q_prox_estado = QValues[estado->id][melhor_acao];
	
	if (this->estado_anterior != NULL) {	///Na primeira execução, não é permitido atualizar a QTable
		float reforco = 0.0;
		QValues[this->estado_anterior->id][this->acao_anterior] = (1 - this->alpha)*QValues[this->estado_anterior->id][this->acao_anterior] + this->alpha*(reforco + this->gamma*Q_prox_estado);
		if (QValues[this->estado_anterior->id][this->acao_anterior] > QVALUE_MAX) {
			QValues[this->estado_anterior->id][this->acao_anterior] = QVALUE_MAX;
		}
		else if (QValues[this->estado_anterior->id][this->acao_anterior] < QVALUE_MIN) {
			QValues[this->estado_anterior->id][this->acao_anterior] = QVALUE_MIN;
		}
		
		this->Modelo[this->estado_anterior->id][this->acao_anterior].reward = reforco;
		this->Modelo[this->estado_anterior->id][this->acao_anterior].estado_destino = estado->id;
		this->aprende_modelo();
	}
	
	//printf("(%d, %d) -> [%f, %f, %f, %f]\n", estado->id, acao, QValues[estado->id][0], QValues[estado->id][1], QValues[estado->id][2], QValues[estado->id][3]);
	//printf("(%d, %d) -> (%f, %f)\n", estado->id, acao, this->path.atual.x, this->path.atual.y);
	
	switch (acao) {
		///Próxima Cobertura
		case PROX_COBERTURA:
			//printf("PROX_COBERTURA\n");
			if (this->path.end.x != this->p_destino.x) {
				this->restaura_caminho(cenario, inimigo, num_inimigos);
			}
			else {
				this->gera_caminho(cenario, inimigo, num_inimigos);
			}
			this->encontra_prox_cobertura();
		break;
		///Mata inimigo
		case MATAR_INIMIGO:
			//printf("MATA\n");
			this->matar_inimigo_prox (cenario, inimigo, num_inimigos);
		break;
		///Esconde
		case ESCONDE:
			//printf("ESCONDE\n");
			this->furtivo = true;
			this->perseguindo = false;
			this->entre_coberturas = false;
			this->encontra_esconderijo(cenario, inimigo, num_inimigos);
		break;
		///Persegue inimigo mais próximo
		case PERSEGUE:
			//printf("PERSEGUE\n");
			this->persegue_mais_proximo(cenario, inimigo, num_inimigos);
		break;
		///Distrai inimigos
		case DISTRAI:
			//printf("DISTRAI\n");
			this->faz_barulho (cenario, inimigo, num_inimigos);
		break;
		default:
			printf("!!!!!!!!!!!!!!!!!!Ação desonhecida: %d\n", acao);
	}
	
	free(this->estado_anterior);
	this->estado_anterior = estado;
	this->acao_anterior = acao;
}
























