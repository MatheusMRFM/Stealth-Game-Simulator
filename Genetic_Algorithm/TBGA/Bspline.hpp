#ifndef B_SPLINE_H_INCLUDED
#define B_SPLINE_H_INCLUDED

#include "Lista.hpp"

#define DISCRETIZACAO 500
#define GRAU_INICIAL 2

///Estrutura: PONTO B-SPLINE
typedef struct Ponto {
    double x, y;
}Ponto;

class Bspline {
	public:
		Lista *pontos;    			///Lista duplamente encadeada que contém os pontos do domínio (pontos de controle)
		int cont_pontos;          	///Número de pontos existentes na listagem de pontos de controle
		Ponto *bspline;		    	///Estrutura referente ao conjunto de pontos da B-Spline
		int pontos_discretizacao; 	///Número de pontos para a discretização da B-Spline
		int nos_bspline;	      	///Número de pontos existentes na listagem de pontos da B-Spline
		int grau;	             	///Grau da B-Spline
		double *VT;					///Vetor de nós da B-Spline
		bool mostraLinhas;
		
		Bspline();
		~Bspline();
		double C(double t, int VARX);
		double N(int i, int j, double t);
		void atualizaBspline ();
		void rearranjaNos ();
		void insere_ponto (double x, double y, int mesh_correspondente);
		void altera_grau (bool aumenta);
		void altera_discretizacao (int novo_valor);
		void imprime_tamanho_discretizacao ();
		void limpa_bspline ();
};


#endif

