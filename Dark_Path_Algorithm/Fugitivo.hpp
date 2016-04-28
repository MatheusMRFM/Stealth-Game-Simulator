#ifndef FUGITIVO_HPP
#define FUGITIVO_HPP

#include "Agente.hpp"
#include "Inimigo.hpp"

#define RAIO_VISAO 220

class Grafo;

class Fugitivo : public Agente {
	public:
		Ponto_Mapa p_destino;
		int tempo_recalcula_1;		///Quando este valor for zero, o agente atualiza seu caminho, considerando as ultimas modificações no cenário
		int tempo_recalcula_2;
		bool encontrado;			///Usado para o modo de testes
		Lista *caminho;

		Fugitivo () {};
		Fugitivo(Cenario *cenario);
		~Fugitivo();
		virtual void gera_posicao_inicial (Cenario *cenario);
		virtual void restaura_valores (Cenario* cenario);
		virtual Ponto_Mapa* encontra_prox_ponto (Cenario *cenario);
		virtual void altera_velocidade (bool anda, Inimigo **inimigo, int num_inimigos);
		void define_destino (Cenario *cenario);
		Ponto_Mapa* verifica_visao_inimigos (Cenario *cenario, Inimigo **inimigo, int num_inimigos, bool check_path);
		void faz_barulho_passo (Cenario *cenario, Inimigo **inimigo, int num_inimigos);
};

#endif
