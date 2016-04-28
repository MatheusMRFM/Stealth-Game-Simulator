#ifndef INIMIGO_HPP
#define INIMIGO_HPP

#include "Agente.hpp"

#define TAM_VISAO_INICIAL 			120
#define DIST_AGENTE_INICIAL			130
#define DIST_MAX_AGENTE_INICIAL		160
#define DIST_MAX_DESTINO			140
#define CHANCE_PROX_DESTINO			0
#define ANGULO_VISAO 				1.04719755		///60 graus em rad		
#define NUM_FRAMES_ESPERA 			2000

///Defines para indicar o estado de visão do inimigo
#define NEUTRO 0
#define VERIFICA_BARULHO 1
#define VISUALIZA_CAMINHO 2
#define VISUALIZA_AGENTE 3


class Inimigo : public Agente {
	public:
		int id;
		int num_percursos;
		
		float tam_visao;
		float angulo_visao;
		Poligono visao;
		int estado_visao;		///Indica o que o agente está vendo ou procurando (NEUTRO, VERIFICA_BARULHO, VISUALIZA_CAMINHO ou VISUALIZA_AGENTE)
		int espera;				///Numero de frames que o agente irá ficar esperando até começar a andar novamente
		bool alerta;
		bool morto;
		bool encontrou_corpo; 	///Indica se um inimigo viu um inimigo morto
	
		Inimigo(Cenario *cenario, int id_inimigo);
		~Inimigo();
		virtual void gera_posicao_inicial (float tam_cenario, Agente *fugitivo, Parede **parede, int num_parede);
		virtual void restaura_valores (Cenario* cenario);
		virtual Ponto_Mapa* encontra_ponto_controle (Ponto_Mapa *m1_p, int m2, int m3);
		virtual void gera_caminho_exato ();
		virtual void gera_caminho (Cenario *cenario);
		virtual Ponto_Mapa* verifica_caminho_valido (Cenario *cenario);
		virtual void atualiza_presenca_mesh ();
		virtual void altera_velocidade (bool anda);
		bool enxerga_ponto (Ponto_Mapa *p, Parede **parede, int num_parede, bool ilimitado);
		bool enxerga_ponto_visao_ilimitada (Ponto_Mapa *p, Parede **parede, int indice_parede);
		void atualiza_campo_visao ();
		void define_novo_destino (Cenario *cenario, Ponto_Mapa *d);
		void verifica_pertubacao (Cenario *cenario, Ponto_Mapa *p, int mesh);
		void entra_alerta();
		void ve_inimigo_morto (Cenario* cenario, Inimigo** inimigo, int num_inimigos);
};


#endif
