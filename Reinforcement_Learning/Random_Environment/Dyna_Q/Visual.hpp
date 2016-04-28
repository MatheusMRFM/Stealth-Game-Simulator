#ifndef VISUAL_HPP
#define VISUAL_HPP

#include "Fugitivo.hpp"
#include "Inimigo.hpp"

class Visual {
	public:
		bool visualiza_grafo;
		bool visualiza_mesh;
		bool visualiza_nodos_fechados;
		bool visualiza_caminho_mesh;
		bool visualiza_caminho_exato;
		bool visualiza_pontos_controle;
		bool visualiza_linhas_controle;
		
		Visual();
		~Visual();
		void desenha_cenario(Cenario *cenario, Grafo *grafo, Fugitivo *fugitivo, Inimigo **inimigo, int num_inimigos);
		void desenha_bordas(Cenario *cenario);
		void desenha_parede(Cenario *cenario);
		void desenha_meshes(Cenario *cenario);
		void desenha_grafo(Grafo *grafo);
		void desenha_front_walls(Cenario *cenario);
		void desenha_caminho_mesh(Grafo *grafo, Agente *agente);
		void desenha_Bspline (Agente *agente);
		void desenha_fugitivo (Fugitivo *fugitivo);
		void desenha_inimigos (Inimigo **inimigo, int num_inimigos);
};

#endif
