#ifndef FUGITIVO_HPP
#define FUGITIVO_HPP

#include "Agente.hpp"
#include "Inimigo.hpp"

#define TEMPO_RECALCULA_1 100
#define TEMPO_RECALCULA_2 400
#define TEMPO_RECALCULA_3 1500

#define RAIO_VISAO 220

///Define o intervalo permitido para a dist do ponto inicial ao ponto final
#define PCT_MAX_DIST_DESTINO 0.7
#define PCT_MIN_DIST_DESTINO 0.3

///Defines para indicar as distâncias relativas do agente as paredes
#define DIST_MUITO_PROX 20
#define DIST_PROX 50
#define DIST_MEDIA 80
///Defines para indicar as distâncias relativas do agente aos inimigos
#define INIMIGO_MUITO_PROX 130
#define INIMIGO_PROX 150
#define INIMIGO_MEDIO 170
///Possíveis valores de distância das paredes baseadas nos dfines acima
#define PAREDE_MUITO_PROX 0
#define PAREDE_PROX 1
#define PAREDE_MEDIO 2
#define PAREDE_DISTANTE 3

///Distâncias dos tiros
#define DIST_TIRO_LONGE 150
#define DIST_TIRO_MEDIO 100
#define DIST_TIRO_PERTO 60

///Porcentagens de acerto do tiro relativo as distâncias
#define PCT_ACERTO_MUITO_LONGE 0.0
#define PCT_ACERTO_LONGE 0.1
#define PCT_ACERTO_MEDIO 0.5
#define PCT_ACERTO_PERTO 1.0

///Valores da variável de estado referente a distância do inimigo mais próximo
#define PERTO 0
#define LONGE 1

///Valores que indicam se o agente acertou, errou ou nem tentou dar um tiro
#define ERROU_TIRO 3
#define ACERTOU_TIRO 2
#define SEM_VISAO 1
#define SEM_TENTATIVA 0

class Grafo;

class Fugitivo : public Agente {
	public:
		Ponto_Mapa p_destino;
		bool em_cobertura;			///Indica se o agente encontra-se em cobertura ou não
		bool caminho_errado;		///Indica se o caminho planejado em um dado instante é inapropriado. Enquando esta flag for verdadeira, a cada frame, recalcula-se um novo caminho
		int tempo_recalcula_1;		///Quando este valor for zero, o agente atualiza seu caminho, considerando as ultimas modificações no cenário
		int tempo_recalcula_2;
		bool entre_coberturas; 		///Indica se o agente está caminhando de cobertura em cobertura (tecla 'c') ou se está fazendo o caminho completo
		int *paredes_proximas;		///Vetor que informa quais paredes estão mais próximas do agente. É atualizada a cada 'tempo_recalcula' frames (PAREDE_MUITO_PROX, PAREDE_PROX, PAREDE_MEDIO ou PAREDE_DISTANTE)
		bool encontrado;			///Usado para o modo de testes
		bool perseguindo;			///Indica se o agente está perseguindo alguém ou não
		bool furtivo;				///Indica se o agente está caminhando furtivamente ou se o mesmo deseja apenas o menor caminho até seu destino
		int inimigo_mais_prox;		///ID do inimigo mais próximo do agente
		int estado_posicionamento;	///Variável de estado que indica a disposição dos inimigos em relação ao agente. Varia entre 0 - 7
		int estado_aproximacao;		///Variável de estado que indica quais agentes estão se aproximando do agente. Varia entre 0 - 3
		int matou_recente;			///Indica se na última ação o agente matou um inimigo ou se errou o tiro. Pode variar entre SEM_TENTATIVA, SEM_VISAO, ACERTOU_TIRO ou ERROU_TIRO

		Fugitivo () {};
		Fugitivo(Cenario *cenario);
		~Fugitivo();
		virtual void gera_posicao_inicial (float tam_cenario);
		virtual void restaura_valores (Cenario* cenario);
		virtual Ponto_Mapa* encontra_ponto_controle (Ponto_Mapa *m1_p, int m2, int m3, Inimigo **inimigos, int num_inimigos);
		virtual void gera_caminho_exato ( Inimigo **inimigos, int num_inimigos);
		virtual void gera_caminho (Cenario *cenario, Inimigo **inimigo, int num_inimigos);
		virtual Ponto_Mapa* verifica_caminho_valido (Cenario *cenario, Inimigo **inimigo, int num_inimigos);
		virtual void atualiza_presenca_mesh ();
		virtual void altera_velocidade (bool anda, Inimigo **inimigo, int num_inimigos);
		void define_destino (float tam_cenario);
		Ponto_Mapa* verifica_visao_inimigos (Cenario *cenario, Inimigo **inimigo, int num_inimigos, bool check_path);
		void encontra_caminho_sem_ser_visto (Cenario *cenario, Inimigo **inimigo, int num_inimigos, Ponto_Mapa *p);
		virtual void atualiza_dados (Cenario *cenario, Inimigo **inimigo, int num_inimigos);
		void encontra_prox_cobertura ();
		void atualiza_distancias_parede (Cenario *cenario);
		void atualiza_distancias_inimigos (Inimigo** inimigo, int num_inimigos);
		void encontra_esconderijo (Cenario *cenario, Inimigo **inimigo, int num_inimigos);
		void faz_barulho_passo (Cenario *cenario, Inimigo **inimigo, int num_inimigos);
		void faz_barulho (Cenario *cenario, Inimigo **inimigo, int num_inimigos);
		void restaura_caminho (Cenario* cenario, Inimigo** inimigo, int num_inimigos);
		void persegue_mais_proximo (Cenario *cenario, Inimigo **inimigo, int num_inimigos);
		void matar_inimigo_prox (Cenario* cenario, Inimigo **inimigo, int num_inimigos);
		bool visao_livre_inimigo (Cenario* cenario, Inimigo** inimigo, int id);
		int dist_inimigo_prox (Inimigo** inimigo);
		bool inimigo_prox_parado (Inimigo** inimigo);
};

#endif
