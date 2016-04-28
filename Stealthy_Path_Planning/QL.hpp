#ifndef QL_HPP
#define QL_HPP

#include "Fugitivo.hpp"


///Defines para as ações possíveis
#define PROX_COBERTURA 0
#define MATAR_INIMIGO 1
#define ESCONDE 2
#define PERSEGUE 3
#define DISTRAI 4

#define NUM_ACAO 4

///Defines para o número de valores para cada variável de estado
#define VAL_DIST_DESTINO	3
//#define VAL_ACAO_ANDAMENTO	3
#define VAL_POS_INIMIGOS	8
//#define VAL_ESCONDIDO		2
#define VAL_CAMINHO_SEGURO	2
#define VAL_INIMIGO_APROX	4
#define VAL_VISAO_LIVRE		2
#define VAL_DIST_PROX		2
#define VAL_INIMIGO_PARADO	2

///Valores para as ações em andamento
#define EM_ANDAMENTO_PROX		0
#define EM_ANDAMENTO_ESCONDE	1
//#define EM_ANDAMENTO_PERSEGUE	2
#define EM_ANDAMENTO_NADA		2

///Defines para definir a distância do agente ao objetivo
#define DIST_DESTINO_PERTO	100
#define DIST_DESTINO_MEDIO 200
#define DESTINO_PERTO 0
#define DESTINO_MEDIO 1
#define DESTINO_LONGE 2

///Defines do Q-Learning
#define QVALUE_INICIAL 0.0
#define QVALUE_MAX 1.0
#define QVALUE_MIN -1.0

#define ALPHA_INICIAL 0.5
#define GAMMA_INICIAL 0.1
#define PHI_INICIAL 0.5

///Defines dos valores de Recompensa
#define RCP_COMPLETA 1.0
#define RCP_MATA 1.0
#define RCP_ANDA_DESTINO 1.0
#define RCP_ANDA_SEM_INIMIGOS 1.0
#define RCP_PERSEGUE_CERTO 0.5
#define RCP_ESCONDE_CERTO 1.0
#define RCP_DESTINO_PERTO 1.0

///Defines dos valores de Punição
#define PUN_INCOMPLETO 1.0
#define PUN_ERRA 1.0
#define PUN_ENCONTRADO 1.0
#define PUN_DISTRAI_ERRADO 0.3
#define PUN_DISTRAI_LONGE 1.0
#define PUN_ANDA_SEM_CAMINHO 1.0
#define PUN_PERSEGUE_MORTOS 1.0
#define PUN_PARADO_SEM_INIMIGO 1.0
#define PUN_PERSEGUE_ERRADO 1.0
#define PUN_NAO_ESCONDE 1.0
#define PUN_AFASTA_DESTINO 1.0



typedef struct Estado {
	int id;
	int dist_destino;
	int acao_andamento;
	int pos_inimigos;
	bool escondido;
	bool seguro;
	int aproxima;
	bool alerta;
	bool visao_livre;
	int dist_prox;
	bool parado; 
}Estado;

class Fugitivo_RL : public Fugitivo {
	public:
		float** QValues;
		int num_estados;		
		Estado *estado_anterior;		///Último estado do agente
		int acao_anterior;				///Última ação executada pelo agente
		float alpha;					///Taxa de aprendizado
		float gamma;					///Taxa de desconto
		float phi;						///Taxa de exploração
		bool em_alerta;					///Indica se os inimigos entraram em alerta ou não
		bool chegou_destino;			///Indica se o agente chegou em seu destino
		int tempo_recalcula_3;			///Tempo para medir a distância até paredes
		
		Fugitivo_RL(Cenario* cenario, char* nome_arq, float aprendizado, float desconto, float exploracao);
		~Fugitivo_RL();
		virtual void restaura_valores (Cenario* cenario);
		virtual void atualiza_dados (Cenario *cenario, Inimigo **inimigo, int num_inimigos);
		void savla_QValues (char* arq_saida);
		void carrega_QValues (char* nome_arq);
		float calcula_reforco(Inimigo** inimigo, int num_inimigos);
		int distancia_destino ();
		int acao_em_andamento (Inimigo** inimigo);
		Estado* define_estado_atual (Cenario* cenario, Inimigo** inimigo, int num_inimigos);
		void prox_acao (Cenario* cenario, Inimigo** inimigo, int num_inimigos);
		float calcula_reforco ();
		int escolhe_melhor_acao (int estado);
		int escolhe_acao(int estado);
};

#endif
