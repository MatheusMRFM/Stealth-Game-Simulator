#include <stdio.h>
#include <stdlib.h>
#include <math.h> 
#include <unistd.h>
#include <time.h>
#include "QL.hpp"
#include "Reward.cpp"

Fugitivo_RL::Fugitivo_RL(Cenario *cenario, char* nome_arq, float aprendizado, float desconto, float exploracao) {
	this->grafo = new Grafo(cenario->nav_mesh);
	this->em_cobertura = false;
	this->existe_caminho = true;
	this->caminho_errado = false;
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
	this->num_estados = VAL_POS_INIMIGOS*VAL_INIMIGO_APROX*VAL_VISAO_LIVRE*VAL_DIST_PROX*VAL_INIMIGO_PARADO;
	
	this->QValues = (float**) malloc (this->num_estados*sizeof(float*));
	this->Z = (float**) malloc (this->num_estados*sizeof(float*));
	for (int i = 0; i < this->num_estados; i++) {
		this->QValues[i] = (float*) malloc (NUM_ACAO*sizeof(float));
		this->Z[i] = (float*) malloc (NUM_ACAO*sizeof(float));
		for (int j = 0; j < NUM_ACAO; j++) {
			this->QValues[i][j] = QVALUE_INICIAL;
			this->Z[i][j] = 0.0;
		}
	}
	
	if (nome_arq) 
		this->carrega_QValues(nome_arq);
	else
		this->carrega_tabela_reforco();
	
	this->alpha = aprendizado;
	this->gamma = desconto;
	this->phi = exploracao;
	this->lambda = LAMBDA_INICIAL;
	
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
		sprintf(nome, "../QValues/QValues_%1.3f_%1.1f_%1.1f.txt", this->alpha, this->gamma, this->phi);
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
void Fugitivo_RL::carrega_tabela_reforco () {
	char aux;
	FILE *arq = fopen("../Reward.cpp", "r");
	///Le a linha inicial
	do {
		fscanf(arq, "%c", &aux);
	} while (aux != '\n');
    ///Le a matriz de recompensa
    for (int i = 0; i < num_estados; i++) {
		for (int j = 0; j < NUM_ACAO; j++) {
			fscanf(arq, "\t%f\t%c", &this->QValues[i][j], &aux);
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
	//float dx, dy, dist;
	
	Ponto_Mapa *p = NULL;
	
	///Verifica se o agente chegou no destino
	if (this->path.atual.x == this->p_destino.x && this->path.atual.y == this->p_destino.y) {
		this->chegou_destino = true;
		float reforco = RCP_COMPLETA;
		
		if (this->estado_anterior != NULL) {
			float delta = reforco;
			///Atualiza o vestígio do estado anteriro (trace (S,A))
			this->Z[this->estado_anterior->id][this->acao_anterior] = 1.0;
			///Altera os QValues de todos os pares (S,A) usando elegibility traces (Z)
			for (int i = 0; i < this->num_estados; i++) {
				for (int j = 0; j < NUM_ACAO; j++) {
					///Altera o QValue
					QValues[i][j] = QValues[i][j] + this->alpha*delta*this->Z[i][j];
					if (QValues[i][j] > QVALUE_MAX) 		QValues[i][j] = QVALUE_MAX;
					else if (QValues[i][j] < QVALUE_MIN)	QValues[i][j] = QVALUE_MIN;
					///Altera a tabela de traces Z
					this->Z[i][j] = this->gamma*this->lambda*this->Z[i][j];
				}
			}
		}
		return;
	}
	
	///Verifica se o agente foi encontrado
	if (this->encontrado) {
		float reforco = -1*PUN_ENCONTRADO;
		
		if (this->estado_anterior != NULL) {
			float delta = reforco;
			///Atualiza o vestígio do estado anteriro (trace (S,A))
			this->Z[this->estado_anterior->id][this->acao_anterior] = 1.0;
			///Altera os QValues de todos os pares (S,A) usando elegibility traces (Z)
			for (int i = 0; i < this->num_estados; i++) {
				for (int j = 0; j < NUM_ACAO; j++) {
					///Altera o QValue
					QValues[i][j] = QValues[i][j] + this->alpha*delta*this->Z[i][j];
					if (QValues[i][j] > QVALUE_MAX) 		QValues[i][j] = QVALUE_MAX;
					else if (QValues[i][j] < QVALUE_MIN)	QValues[i][j] = QVALUE_MIN;
					///Altera a tabela de traces Z
					this->Z[i][j] = this->gamma*this->lambda*this->Z[i][j];
				}
			}
		}
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
	estado->pos_inimigos = this->estado_posicionamento;
	estado->seguro = this->existe_caminho;
	estado->aproxima = this->estado_aproximacao;
	estado->visao_livre = this->visao_livre_inimigo(cenario, inimigo, this->inimigo_mais_prox);
	estado->dist_prox = this->dist_inimigo_prox(inimigo);
	estado->parado = this->inimigo_prox_parado(inimigo);
	
	estado->id = 0;
	
	estado->id += estado->pos_inimigos*VAL_INIMIGO_APROX*VAL_VISAO_LIVRE*VAL_DIST_PROX*VAL_INIMIGO_PARADO;
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
	srand (time(NULL));
	float max = QVALUE_MIN;
	int num_valores_max = 0;
	for (int i=0; i<NUM_ACAO; i++) {
		//printf("%1.4f\t", QValues[estado][i]);
		if (QValues[estado][i] > max) {
			max = QValues[estado][i];
			num_valores_max = 1;
		}
		else if (QValues[estado][i] == max) 
			num_valores_max++;
	}
	//printf("\n");
	int acao_escolhida, acao;
	if (num_valores_max != 0) {
		int explora = rand() % 11;
		if (explora < this->phi*10) {
			//printf("(Explora)\n");
			acao = rand() % NUM_ACAO;
		}
		else {
			//acao_escolhida = rand() % num_valores_max;
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
void Fugitivo_RL::prox_acao (Cenario* cenario, Inimigo** inimigo, int num_inimigos) {
	Estado *estado = this->define_estado_atual(cenario, inimigo, num_inimigos);
	int acao = this->escolhe_acao(estado->id);
	float Q_prox_estado = QValues[estado->id][acao];
	
	if (this->estado_anterior != NULL) {	///Na primeira execução, não é permitido atualizar a QTable
		float reforco = calcula_reforco();
		float delta = reforco + this->gamma*Q_prox_estado - QValues[this->estado_anterior->id][this->acao_anterior];
		///Atualiza o vestígio do estado anterior (trace (S,A))
		this->Z[this->estado_anterior->id][this->acao_anterior] = 1.0;
		///Altera os QValues de todos os pares (S,A) usando elegibility traces (Z)
		for (int i = 0; i < this->num_estados; i++) {
			for (int j = 0; j < NUM_ACAO; j++) {
				///Altera o QValue
				QValues[i][j] = QValues[i][j] + this->alpha*delta*this->Z[i][j];
				if (QValues[i][j] > QVALUE_MAX) 		QValues[i][j] = QVALUE_MAX;
				else if (QValues[i][j] < QVALUE_MIN)	QValues[i][j] = QVALUE_MIN;
				///Altera a tabela de traces Z
				this->Z[i][j] = this->gamma*this->lambda*this->Z[i][j];
			}
		}
	}
	
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
























