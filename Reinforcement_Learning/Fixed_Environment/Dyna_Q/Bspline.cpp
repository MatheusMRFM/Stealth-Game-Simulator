#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "Bspline.hpp"

Bspline::Bspline () {
	this->pontos_discretizacao = DISCRETIZACAO;
	this->nos_bspline = 0;
	this->grau = GRAU_INICIAL;
	this->cont_pontos = 0;
	this->pontos = inicializa();
	this->bspline = (Ponto*) malloc(pontos_discretizacao * sizeof(Ponto));	
	this->VT = NULL;
	this->mostraLinhas = true;
}


Bspline::~Bspline () {
	
}

void Bspline::limpa_bspline () {
	this->nos_bspline = 0;
	this->cont_pontos = 0;
	this->pontos = libera(this->pontos);
	free(this->VT);
	this->VT = NULL;
}

void Bspline::insere_ponto (double x, double y, int mesh_correspondente) {
	
	//Insere um novo ponto na lista e guarda as coordenadas deste
	pontos = insere(this->pontos, mesh_correspondente, x, y);
	this->cont_pontos++;
}

///Rearranja Nós
//Os nós da curva são rearranjados caso aconteça alguma das situações abaixo:
//1. Modifica-se o grau da B-Spline;
//2. Pontos de controle são inseridos/removidos da listagem de pontos de controle.
//REFERÊNCIA: 'Ciemborowicz, M. B-Spline for Processing, 2010. Site: <http://bspline.ciemborowicz.pl/index.php?id=presentation>.'
void Bspline::rearranjaNos() {
	nos_bspline = cont_pontos + grau + 1;
	
	if (VT == NULL)
		VT = (double*) malloc(nos_bspline * sizeof(double));
	else
		VT = (double*) realloc(VT, nos_bspline * sizeof(double));

	for (int i = 0; i < nos_bspline; i++)
		VT[i] = i/((double)nos_bspline-1);
}


///Definição da Função Base
//REFERÊNCIA: 'Weisstein, E. W. B-Spline, 2013. MathWorld - A Wolfram Web Resource. <Site: http://mathworld.wolfram.com/B-Spline.html>.'
double Bspline::N(int i, int j, double t) {	
	double NIJ;
	
	if (j == 0) {
		if ((t >= VT[i]) && (t < VT[i+1]))
			return 1.0;
		else
			return 0.0;
	}

	NIJ = ((t - VT[i])/(VT[i+j] - VT[i]))*N(i, j-1, t);
	NIJ += ((VT[i+j+1] - t)/(VT[i+j+1] - VT[i+1]))*N(i+1, j-1, t);
	
	return NIJ;
}


///Definição da Curva
//REFERÊNCIA: 'Weisstein, Eric W. B-Spline, 2013. MathWorld - A Wolfram Web Resource. <Site: http://mathworld.wolfram.com/B-Spline.html>.'
double Bspline::C(double t, int VARX) {   
	Lista *ponto; //Ponteiro para percorrer a lista de pontos de controle
	
	int i = 0;
	double POS = 0.0;
	
	for(ponto = pontos; ponto != NULL; ponto = ponto->prox) {
        if (VARX)
			POS += ponto->x*N(i, grau, t);
		else
			POS += ponto->y*N(i, grau, t);
			
		i++;
    }

	return POS;
}

///Atualização da B-Spline
void Bspline::atualizaBspline() {
	//Discretização (Geração da Curva)
	double dn = 1.0/((double)pontos_discretizacao - 1);
	
	int primeiroPonto = -1; //Ponto Inicial
	int ultimoPonto   = -1; //Ponto Final
	
	//Atualiza os nós internos da curva: T(grau) <= t <= T(nos_bspline - grau - 1)
    for(int i = 0; i < pontos_discretizacao; i++)  {
		if ((i*dn >= VT[grau]) && (i*dn <= VT[nos_bspline-grau-1])) {
			bspline[i].x = C(i*dn, 1);
			bspline[i].y = C(i*dn, 0);
			
			if (primeiroPonto < 0)
				primeiroPonto = i;
				
			ultimoPonto = i;
		}
    }
    //Nós restantes recebem os valores do menor e maior nós internos
    for(int i = 0; i < pontos_discretizacao; i++)  {
		if (i*dn < VT[grau]) {
			bspline[i].x = bspline[primeiroPonto].x;
			bspline[i].y = bspline[primeiroPonto].y;
		}
		else if (i*dn > VT[nos_bspline-grau-1]) {
			bspline[i].x = bspline[ultimoPonto].x;
			bspline[i].y = bspline[ultimoPonto].y;
		}
	}
}

void Bspline::altera_grau (bool aumenta) {
	if (aumenta)
		grau++;
	else
		grau--;
	//Atualiza B-Spline
	rearranjaNos();
	atualizaBspline();
}

void Bspline::altera_discretizacao (int novo_valor) {
	pontos_discretizacao = novo_valor;
	bspline = (Ponto*) realloc(bspline, pontos_discretizacao * sizeof(Ponto));
	atualizaBspline();
}

void Bspline::imprime_tamanho_discretizacao () {
	float dx, dy, dist;
	for(int i = 0; i < pontos_discretizacao-1; i++) {
		dx = bspline[i].x - bspline[i+1].x;
		dy = bspline[i].y - bspline[i+1].y;
		dist = sqrt(dx*dx + dy*dy);
		printf("dist = %f\n", dist);
	}
}	
	
	
	
	
	
	
	
	
	


