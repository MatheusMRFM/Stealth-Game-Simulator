#ifndef VISUAL_HPP
#define VISUAL_HPP

#include "Cenario.hpp"
#include "Fugitivo.hpp"
#include "Inimigo.hpp"

class Visual {
	public:
		Visual();
		~Visual();
		void desenha_cenario(Cenario *cenario, Fugitivo *fugitivo, Inimigo **inimigo, int num_inimigos);
		void desenha_bordas(Cenario *cenario);
		void desenha_parede(Cenario *cenario);
		void desenha_grid (Cenario *cenario);
		void desenha_fugitivo (Fugitivo *fugitivo, Cenario *cenario);
		void desenha_inimigos (Inimigo **inimigo, int num_inimigos, Cenario *cenario);
};

#endif
