#ifndef INIMIGO_HPP
#define INIMIGO_HPP

#include "Agente.hpp"

#define TAM_VISAO_INICIAL 120
#define DIST_AGENTE_INICIAL 150
#define ANGULO_VISAO 1.04719755		///60 graus em rad		
#define NUM_FRAMES_ESPERA 2000

///Defines para indicar o estado de visão do inimigo
#define NEUTRO 0
#define VERIFICA_BARULHO 1
#define VISUALIZA_CAMINHO 2
#define VISUALIZA_AGENTE 3

#define MAX_TENTATIVAS 10


class Inimigo : public Agente {
	public:
		Grid_Cell **grid;
		float tam_visao;
		float angulo_visao;
		Poligono visao;
		int estado_visao;		///Indica o que o agente está vendo ou procurando (NEUTRO, VERIFICA_BARULHO, VISUALIZA_CAMINHO ou VISUALIZA_AGENTE)
		int espera;				///Numero de frames que o agente irá ficar esperando até começar a andar novamente
	
		Inimigo(Cenario *cenario);
		~Inimigo();
		virtual void gera_posicao_inicial (Cenario *cenario, Agente *fugitivo);
		virtual void restaura_valores (Cenario* cenario);
		virtual Ponto_Mapa* encontra_prox_ponto (Cenario *cenario);
		virtual void altera_velocidade (bool anda);
		bool enxerga_ponto (Ponto_Mapa *p, Parede **parede, int num_parede, bool ilimitado);
		void atualiza_campo_visao (Cenario *cenario);
		void define_novo_destino (Cenario *cenario);
		void verifica_pertubacao (Cenario *cenario, Ponto_Mapa *p);
		bool enxerga_ponto_visao_ilimitada (Cenario *cenario, Ponto_Mapa *p, Parede **parede, int indice_parede);
};


#endif
