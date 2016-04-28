#ifndef AGENTE_HPP
#define AGENTE_HPP

#include "Cenario.hpp"

#define MODULO_VELOCIDADE_PATRULHA 0.03
#define MODULO_VELOCIDADE_ALERTA 0.04
#define MODULO_VELOCIDADE_SNEAK 0.04
#define MODULO_VELOCIDADE_ANDA 0.06
#define MODULO_VELOCIDADE_CORRE 0.10

#define RAIO_SOM_SNEAK 0
#define RAIO_SOM_ANDA 100
#define RAIO_SOM_CORRE 170
#define RAIO_SOM_DISTRACAO 160 ///Igual a DIST_MEDIA

#define ESTADO_PARADO 0
#define ESTADO_SNEAK 1
#define ESTADO_ANDA 2
#define ESTADO_CORRE 3

typedef struct Path {
	Ponto_Mapa start, end, atual, objetivo;
	float velocidade;
	Vetor3D direcao;
	int estado_velocidade;	///0 = parado, 1 = Senak, 2 = Anda, 3 = Corre
	bool terminou_caminho;
}Path;

class Agente {
	public:
		Path path;
		bool existe_caminho;
		int recalcula;
		
		Agente();
		~Agente();
		virtual void gera_posicao_inicial (Cenario *cenario);
		virtual Ponto_Mapa* encontra_prox_ponto (Cenario *cenario);
		virtual void altera_velocidade (bool anda);
		virtual void restaura_valores ();
		void caminha (Cenario *cenario);
		Ponto_Mapa* gera_posicao (Cenario *cenario);
		void atualiza_posicao ();
		void inicializa_dados (bool limpa_pesos);
};

#endif
