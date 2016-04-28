#ifndef AGENTE_HPP
#define AGENTE_HPP

#include "A_Star.hpp"
#include "Bspline.hpp"

#define MODULO_VELOCIDADE_PATRULHA 0.015
#define MODULO_VELOCIDADE_ALERTA 0.02
#define MODULO_VELOCIDADE_SNEAK 0.02
#define MODULO_VELOCIDADE_ANDA 0.03
#define MODULO_VELOCIDADE_CORRE 0.05

/*#define MODULO_VELOCIDADE_PATRULHA 0.03
#define MODULO_VELOCIDADE_ALERTA 0.04
#define MODULO_VELOCIDADE_SNEAK 0.04
#define MODULO_VELOCIDADE_ANDA 0.06
#define MODULO_VELOCIDADE_CORRE 0.1*/

#define RAIO_SOM_SNEAK 0
#define RAIO_SOM_ANDA 100
#define RAIO_SOM_CORRE 170
#define RAIO_SOM_DISTRACAO 160 ///Igual a DIST_MEDIA

#define ESTADO_PARADO 0
#define ESTADO_SNEAK 1
#define ESTADO_ANDA 2
#define ESTADO_CORRE 3

typedef struct Path {
	Ponto_Mapa start, end, atual;
	int mesh_start, mesh_end, mesh_atual;
	ListaDE *caminho_mesh;
	Bspline *caminho_exato;
	bool entre_pontos_bspline;
	int p_atual_bspline;
	float velocidade;
	int estado_velocidade;	///0 = parado, 1 = Senak, 2 = Anda, 3 = Corre
	Vetor3D direcao;
	bool terminou_caminho;
	int mesh_objetivo;		///Mesh referente a próxima cobertura (para os inimigos, mesh_objetivo = mesh_end)
}Path;

class Agente {
	public:
		Path path;
		bool existe_caminho;
		Grafo *grafo;
		
		Agente();
		~Agente();
		virtual void gera_posicao_inicial (float tam_cenario);
		virtual void gera_caminho_exato ();								///Cria o caminho de exato com Bspline do ponto inicial ao destino (especificados em "path")
		virtual Ponto_Mapa* encontra_ponto_controle (Ponto_Mapa *m1_p, int m2, int m3);
		virtual void gera_caminho ();
		virtual Ponto_Mapa* verifica_caminho_valido (Cenario *cenario);
		virtual void atualiza_presenca_mesh ();
		virtual void altera_velocidade (bool anda);
		virtual void restaura_valores ();
		Ponto_Mapa* gera_posicao (float tam_cenario, int id_inimigo);
		bool gera_caminho_mesh ();										///Cria o caminho de meshes do ponto inicial ao destino (especificados em "path")
		void gera_caminho_exato_basico ();								///Cria o caminho de exato com Bspline do ponto inicial ao destino (especificados em "path")
		void atualiza_posicao ();
		void inicializa_dados (bool limpa_pesos);
		Ponto_Mapa* caminho_atravessa_parede (Cenario *cenario);
		int mesh_caminho_mais_proxima (Ponto_Mapa *p);	///Dado um ponto p, encontra qual a mesh pertencente ao caminho mais próxima
};

#endif
