#include "Lista.hpp"

///Inicialização da lista duplamente encadeada
Lista* inicializa(void) {
    return NULL;
}	

///Inserção de um novo nó no início da lista circular duplamente encadeada
Lista* insere(Lista* l, int num, double x, double y) {
    Lista *novo = (Lista*) malloc(sizeof(Lista));
    
	novo->x = x;
	novo->y = y;
	novo->num = num;

	novo->prox = l;
	novo->ant = NULL;

	///Verifica se a lista não está vazia
	if (l != NULL) {
		l->ant = novo;
	}
		
	return novo;
}

///Imprime valores contidos na lista duplamente encadeada
void imprime(Lista* l) {
	///Ponteiro que aponta para o nó inicial da lista duplamente encadeada
    Lista *p = l;

    while (p != NULL) {
        printf("NÓ: %d, (X: %lf, Y: %lf)\n", p->num, p->x, p->y);
        p = p->prox;
    }
}


///Imprime valores contidos na lista duplamente encadeada (aparecem na ordem de inserção)
void imprimeInverso(Lista* l) {
	///Ponteiro que aponta para o nó inicial da lista duplamente encadeada
    Lista *p = l;

    while (p->prox != NULL)
        p = p->prox;

    while (p != NULL) {
		printf("NÓ: %d, (X: %lf, Y: %lf)\n", p->num, p->x, p->y);
        p = p->ant;
    }
}

///Iguala os valores internos de dois nós
Lista* igualaNos(Lista* l, int num_i, int num_j) {
	Lista *N1 = busca(l, num_i);
    Lista *N2 = busca(l, num_j);
    
    N2->x = N1->x;
    N2->y = N1->y;
    
    return l;
}


///Busca valor contido na lista duplamente encadeada
Lista* busca(Lista* l, int num) {
    Lista *p;
    
    //Percorre a lista procurando o valor passado como parâmetro
    for (p = l; p != NULL; p = p->prox)
        if (p->num == num)
            return p;
    
    //Caso o elemento não tenha sido encontrado
    return NULL;
}


///Remove um elemento da lista duplamente encadeada
Lista* remove(Lista* l, int num) {
    Lista *p = busca(l, num);
    
    //Se o elemento não tiver sido encontrado, retorna-se a lista inalterada
    if (p == NULL)
        return l;  
        
    //Retira-se elemento do encadeamento
    if (l == p)
        l = p->prox;
    else
        p->ant->prox = p->prox;
        
    if (p->prox != NULL)
		p->prox->ant = p->ant;
        
    free(p);
    return l;
}


///Libera memória alocada para a lista duplamente encadeada
Lista* libera(Lista* l) {
	Lista *p = l;
	
	while (p != NULL) 	{
		//Guarda referência para o próximo elemento
		Lista *t = p->prox;
		
		//Libera a memória apontada por p
		free(p);
		
		//Faz p apontar para o próximo elemento da lista
		p = t;
	}
	
	return NULL;
}

//**********************************************************************
//***************************** PILHA **********************************
//**********************************************************************
Pilha::Pilha() {
	this->topo = NULL;
}

Pilha::~Pilha () {
	while (this->topo) {
		this->desempilha();
	}
}

void Pilha::empilha(float x1, float y1, float x2, float y2) {
	Elemento* novo = (Elemento*) malloc (sizeof(Elemento));
	
	novo->usado = false;
	novo->p1.x = x1;
	novo->p1.y = y1;
	novo->p2.x = x2;
	novo->p2.y = y2;
	
	novo->prox = this->topo;
	this->topo = novo;
}

Elemento* Pilha::desempilha() {
	Elemento *sai = this->topo;
	
	this->topo = this->topo->prox;
	sai->prox = NULL;
	
	return sai;
}

Elemento* Pilha::procura_menor_x (float xMin, float yMax, float yMin) {
	Elemento* menor = this->topo;
	for (Elemento *aux = this->topo; aux != NULL; aux = aux->prox) {
		if ((aux->p1.y < yMax && aux->p1.y > yMin) ||  (aux->p2.y < yMax && aux->p2.y > yMin) || (aux->p1.y > yMax && aux->p2.y < yMin) || (aux->p2.y < yMin && aux->p1.y <= yMax && aux->p1.y > yMin) || (aux->p1.y > yMax && aux->p2.y >= yMin && aux->p2.y < yMax)) {
			if (aux->p1.x >= xMin && aux->p1.x <= menor->p1.x) {
				if (aux->p1.x < menor->p1.x || (aux->p1.x == menor->p1.x && aux->p1.y > menor->p1.y)) {
					menor = aux;
				}
			}
		}
	}
	
	menor->usado = true;
	
	return menor;
}

Elemento* Pilha::procura_igual (float xMin, float yMax, float yMin, float v_x) {
	Elemento* menor = this->topo;
	for (Elemento *aux = this->topo; aux != NULL; aux = aux->prox) {
		if ((aux->p1.y < yMax && aux->p1.y > yMin) ||  (aux->p2.y < yMax && aux->p2.y > yMin) || (aux->p1.y > yMax && aux->p2.y < yMin) || (aux->p2.y < yMin && aux->p1.y <= yMax && aux->p1.y > yMin) || (aux->p1.y > yMax && aux->p2.y >= yMin && aux->p2.y < yMax)) {
			if (aux->p1.x >= xMin && aux->p1.x <= menor->p1.x) {
				if (menor->usado || (!aux->usado && (aux->p1.x < menor->p1.x || (aux->p1.x == menor->p1.x && aux->p1.y > menor->p1.y)))) {
					menor = aux;
				}
			}
		}
	}
	
	if (menor->p1.x == v_x && !menor->usado) {
		menor->usado = true;
	}
	else 
		menor = NULL;
	
	return menor;
}

void Pilha::renova_pilha () {
	for (Elemento *aux = this->topo; aux != NULL; aux = aux->prox) {
		aux->usado = false;
	}
}
//**********************************************************************
//**************************** LISTA A* ********************************
//**********************************************************************
ListaDE::ListaDE () {
	this->first = NULL;
}

ListaDE::~ListaDE () {
	Item *aux;
	for (Item *p = this->first; p != NULL; ) {
		aux = p;
		p = p->prox;
		free(aux);
	}
}

void ListaDE::insere_ordenado (int indice, float valor) {
	Item* aux = (Item*) malloc (sizeof(Item));
	aux->id = indice;
	aux->f = valor;
	
	if (this->first == NULL) {
		this->first = aux;
		this->first->ant = NULL;
		this->first->prox = NULL;
	}
	else if (this->first->f >= valor) {
		aux->prox = this->first;
		this->first->ant = aux;
		aux->ant = NULL;
		this->first = aux;
	}
	else {
		Item* p;
		for (p = this->first; p->prox != NULL; p = p->prox) {
			if (p->f <= valor && p->prox->f > valor) {
				aux->ant = p;
				aux->prox = p->prox;
				p->prox = aux;
				aux->prox->ant = aux;
				//imprime();
				return;
			}
		}
		
		if (p->f <= valor) {
			aux->ant = p;
			aux->prox = NULL;
			p->prox = aux;
		}
		else {
			aux->prox = p;
			aux->ant = p->ant;
			p->ant = aux;
			if (aux->ant != NULL) 
				aux->ant->prox = aux;
		}
	}
	//imprime();
}

void ListaDE::insere_fim (int indice, float valor) {
	Item* aux = (Item*) malloc (sizeof(Item));
	aux->id = indice;
	aux->f = valor;
	
	if (this->first == NULL) {
		this->first = aux;
		aux->prox = NULL;
		aux->ant = NULL;
	}
	else {
		Item* p;
		for (p = this->first; p->prox != NULL; p = p->prox) { }
		
		p->prox = aux;
		aux->prox = NULL;
		aux->ant = p;
	}
}

void ListaDE::insere_inicio (int indice, float valor) {
	Item* aux = (Item*) malloc (sizeof(Item));
	aux->id = indice;
	aux->f = valor;
	aux->prox = this->first;
	aux->ant = NULL;
	this->first = aux;
	if (aux->prox != NULL)
		aux->prox->ant = aux;
}

Item* ListaDE::remove_topo () {
	if (this->first != NULL) {
		Item* aux = this->first;
		this->first = this->first->prox;
		aux->prox = NULL;
		if (this->first != NULL) 
			this->first->ant = NULL;
		
		return aux;
	}
	
	return NULL;
}

Item* ListaDE::remove (int indice) {
	Item* aux;
	if (this->first->id == indice) {
		aux = this->first;
		this->first = this->first->prox;
		aux->prox = NULL;
		if (this->first != NULL) 
			this->first->ant = NULL;
	}
	Item *p;
	if (this->first == NULL)
		return NULL;
	for (p = this->first; p->prox != NULL && p->prox->id != indice; p = p->prox) {}
	if (p->prox == NULL) 
		return NULL;
	else {
		aux = p->prox;
		p->prox = aux->prox;
		aux->ant = NULL;
		aux->prox = NULL;
		if (p->prox != NULL)
			p->prox->ant = p;
	}
	
	return aux;
}

void ListaDE::imprime () {
	printf("LISTA ORDENADA CRESCENTE: \n");
	for (Item* p = this->first; p != NULL; p = p->prox) {
		printf("(%d, %.1f)\n", p->id, p->f);
	}
}


















