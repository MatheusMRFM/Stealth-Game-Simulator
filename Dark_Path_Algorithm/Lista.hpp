#ifndef LISTA_H_INCLUDED
#define LISTA_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>

typedef struct Ponto_Mapa {
	float x;
	float y;
}Ponto_Mapa;

///Estrutura: LISTA DUPLAMENTE ENCADEADA
typedef struct lista {
    double x; ///Coordenada X do ponto de controle
    double y; ///Coordenada Y do ponto de controle
    
    int num; ///Mesh correspondente ao ponto de controle
    
    struct lista *ant; ///Aponta para o ponto de controle anterior da lista
    struct lista *prox; ///Aponta para o próximo ponto de controle da lista
}Lista;


///Funções: LISTA DUPLAMENTE ENCADEADA
Lista* libera (Lista*);
Lista* inicializa(void);

Lista* busca(Lista*, int);
Lista* remove(Lista*, int);
Lista* igualaNos(Lista*, int, int);
Lista* insere(Lista*, int, double, double);

void imprime(Lista*);
void imprimeInverso(Lista*);


///****************** PILHA CENARIO ************************************
typedef struct Elemento{
	Ponto_Mapa p1;
	Ponto_Mapa p2;
	bool usado;
	
	Elemento *prox;
}Elemento;

class Pilha {
	public:
		Elemento *topo;
		
		Pilha();
		~Pilha();
		void empilha(float x1, float y1, float x2, float y2);
		Elemento* desempilha();
		Elemento* procura_menor_x (float xMin, float yMax, float yMin);
		Elemento* procura_igual (float xMin, float yMax, float yMin, float v_x);
		void renova_pilha ();
};

///***************** LISTA A* ******************************************
typedef struct Item {
	int id;
	float f;
	
	Item *ant;
	Item *prox;
}Item;

///Lista duplamente encadeada
class ListaDE {	
	public:
		Item *first;
		
		ListaDE();
		~ListaDE();
		void insere_ordenado (int indice, float valor);
		void insere_fim (int indice, float valor);
		void insere_inicio (int indice, float valor);
		void imprime ();
		Item* remove_topo ();
		Item* remove (int indice);
};













#endif //LISTA_H_INCLUDED
