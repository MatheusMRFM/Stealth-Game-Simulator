#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "A_Star.hpp"
#include "Inimigo.hpp"

Nodo::Nodo (Mesh *m) {
	this->g = 0;
	this->h = 0;
	this->f = 0;
	this->estado = 0;
	this->pai = -1;
	this->pertence_caminho = false;
	this->existe_inimigo = 0;
	float minX = 100000.0f, minY = 100000.0f, maxX = 0.0f, maxY = 0.0f;
	this->mesh = m;
	///Encontra minimos e maximos da mesh
	for (int j = 0; j < 4; j++) {
	    if (this->mesh->pontos[j].x != -1) {
		if (this->mesh->pontos[j].x >= maxX) 
		    maxX = this->mesh->pontos[j].x;
		if (this->mesh->pontos[j].x <= minX) 
		    minX = this->mesh->pontos[j].x;
		if (this->mesh->pontos[j].y >= maxY) 
		    maxY = this->mesh->pontos[j].y;
		if (this->mesh->pontos[j].y <= minY) 
		    minY = this->mesh->pontos[j].y;
	    }
	}
	///Baricentro do Triangulo
	if (this->mesh->tipo == 3) {
	    this->x = (this->mesh->pontos[0].x + this->mesh->pontos[1].x + this->mesh->pontos[2].x) / 3.0;
	    this->y = (this->mesh->pontos[0].y + this->mesh->pontos[1].y + this->mesh->pontos[2].y) / 3.0;
	}
	///Centro do poligono
	else {
	    this->x = (minX + maxX)/2.0;
	    this->y = (minY + maxY)/2.0;
	}
	
	///Remove casas decimais por conta de erros numéricos
	this->max_x = (int)(100*maxX) / 100.0;
	this->max_y = (int)(100*maxY) / 100.0;
	this->min_x = (int)(100*minX) / 100.0;
	this->min_y = (int)(100*minY) / 100.0;
}

Nodo::~Nodo () {
	
}
//**********************************************************************
//**********************************************************************
//**********************************************************************

Grafo::Grafo (NavMesh *nav) {
    ///Inicializa estruturas básicas
    this->closed = new ListaDE();
    this->open = new ListaDE();
    
    this->modo_furtivo = false;
    this->inimigo = NULL;
    
    this->num_no = nav->num_meshes;
    this->nodo = (Nodo**) malloc (this->num_no * sizeof(Nodo*));
    this->m_adj = (float**) malloc (this->num_no * sizeof(float*));
    this->peso = (int**) malloc (this->num_no * sizeof(int*));
    this->caminho_circular = false;
    
    for (int i = 0; i < this->num_no; i++) {
	this->nodo[i] = new Nodo(nav->meshes[i]);
	this->m_adj[i] = (float*) malloc (this->num_no*sizeof(float));
	this->peso[i] = (int*) malloc (this->num_no*sizeof(int));
	for (int j = 0; j < this->num_no; j++) {
	    this->m_adj[i][j] = -1;
	    this->peso[i][j] = 0;
	}
    }
    
    ///Cria as ligações entre os nós
    this->Liga_Nodos_Cobertura();
    this->Liga_Nodos_Triangulares();
    this->Liga_Nodos_Restantes();
}

Grafo::~Grafo () {
    for (int i = 0; i < this->num_no; i++) {
	delete this->nodo[i];
	free(this->m_adj[i]);
	free(this->peso[i]);
    }
	    
    free(this->nodo);
    free(this->m_adj);
    free(this->peso);
    this->inimigo = NULL;
}

void Grafo::set_modo (bool valor) {
    this->modo_furtivo = valor;
}

void Grafo::limpa_caminho_anterior (bool limpa_pesos) {
    delete this->open;
    delete this->closed;
    this->closed = new ListaDE();
    this->open = new ListaDE();
    this->caminho_circular = false;
	
    for (int i = 0; i < this->num_no; i++) {
	this->nodo[i]->pai = -1;
	this->nodo[i]->estado = 0;
	this->nodo[i]->g = 0;
	this->nodo[i]->h = 0;
	this->nodo[i]->f = 0;
	this->nodo[i]->pertence_caminho = false;
	for (int j = 0; j < this->num_no; j++) {
	    if (limpa_pesos) {
		    this->peso[i][j] = 0;
	    }
	    if (this->m_adj[i][j] != -1) {
		    this->m_adj[i][j] = encontra_g(i, j, NULL, false);
	    }
	}
    }
}

void Grafo::Liga_Nodos_Cobertura () {
    ///Percorre cada nodo
    for (int i = 0; i < this->num_no-1; i++) {
	///Compara o nodo i com os outros nodos
	for (int j = i+1; j < this->num_no; j++) {
	    ///Analisa apenas os nodos de cobertura
	    if ((this->nodo[i]->mesh->tipo == 1 && this->nodo[j]->mesh->tipo == 2) || (this->nodo[i]->mesh->tipo == 2 && this->nodo[j]->mesh->tipo == 1) || (this->nodo[i]->mesh->tipo == 2 && this->nodo[j]->mesh->tipo == 3)) {
		///Compara todos os pontos dos nodos i e j para ver se estes compartilham um vértice
		for (int a = 0; a < 4; a++) {
		    for (int b = 0; b < 4; b++) {
			if (this->nodo[i]->mesh->pontos[a].x == this->nodo[j]->mesh->pontos[b].x && this->nodo[i]->mesh->pontos[a].y == this->nodo[j]->mesh->pontos[b].y) {
			    this->m_adj[i][j] = encontra_g(i, j, NULL, false);
			    this->m_adj[j][i] = encontra_g(j, i, NULL, false);
			}
		    }
		}
	    }
	}
    }
}

void Grafo::Liga_Nodos_Triangulares () {
    ///Percorre todos os nodos do tipo 1 (cobertura)
    for (int i = 1; i < this->num_no-1; i++) {
	if (this->nodo[i]->mesh->tipo == 1) {
	    ///Encontra qual o nodo triangular em que seus vizinhos se conectam
	    ///Este nodo triangular é seu vizinho também
	    for (int j = 0; j < this->num_no; j++) {
		if (this->m_adj[i-1][j] != -1 && this->nodo[j]->mesh->tipo == 3) {
		    if (this->m_adj[i+1][j] != -1) {
			this->m_adj[i][j] = encontra_g(i, j, NULL, false);
			this->m_adj[j][i] = encontra_g(j, i, NULL, false);
		    }
		    ///i-7 pq o ultimo bloco tipo 1 é vizinho dos blocos i-1 e i-7 (primeiro bloco tipo 2 em volta de uma parede)
		    else if (i-7 >= 0 && this->m_adj[i-7][j] != -1) {
			this->m_adj[i][j] = encontra_g(i, j, NULL, false);
			this->m_adj[j][i] = encontra_g(j, i, NULL, false);
		    }
		}
	    }
	}
    }
}

void Grafo::Liga_Nodos_Restantes () {	
    for (int i = 0; i < this->num_no; i++) {
	for (int j = 0; j < this->num_no; j++) {
	    if (this->nodo[i]->mesh->tipo == 0 || this->nodo[j]->mesh->tipo == 0) {
		float minX_1 = this->nodo[i]->min_x, minY_1 = this->nodo[i]->min_y, maxX_1 = this->nodo[i]->max_x, maxY_1 = this->nodo[i]->max_y;
		float minX_2 = this->nodo[j]->min_x, minY_2 = this->nodo[j]->min_y, maxX_2 = this->nodo[j]->max_x, maxY_2 = this->nodo[j]->max_y;
		
		///Verifica se os mesmos se conectam em X
		if (minX_1 == maxX_2 || maxX_1 == minX_2) {
		    ///Verifica se os mesmos se conectam em Y
		    if ((maxY_2 < maxY_1 && maxY_2 > minY_1) || (minY_2 < maxY_1 && minY_2 > minY_1) || (maxY_2 > maxY_1 && minY_2 < minY_1) || (minY_2 < minY_1 && maxY_2 <= maxY_1 && maxY_2 > minY_1) || (maxY_2 > maxY_1 && minY_2 >= minY_1 && minY_2 < maxY_1)) {
			this->m_adj[i][j] = encontra_g(i, j, NULL, false);
			this->m_adj[j][i] = encontra_g(j, i, NULL, false);
		    }
		}
		
		///Verifica se os mesmos se conectam em Y
		if (minY_1 == maxY_2 || maxY_1 == minY_2) {
		    ///Verifica se os mesmos se conectam em X
		    if ((maxX_2 < maxX_1 && maxX_2 > minX_1) || (minX_2 < maxX_1 && minX_2 > minX_1) || (maxX_2 > maxX_1 && minX_2 < minX_1) || (minX_2 < minX_1 && maxX_2 <= maxX_1 && maxX_2 > minX_1) || (maxX_2 > maxX_1 && minX_2 >= minX_1 && minX_2 < maxX_1) || (minX_1 == minX_2 && maxX_1 == maxX_2)) {
			this->m_adj[i][j] = encontra_g(i, j, NULL, false);
			this->m_adj[j][i] = encontra_g(j, i, NULL, false);
		    }
		}
	    }
	}
    }
}

int Grafo::encontra_mesh (Ponto_Mapa *p) {
    for (int i = 0; i < this->num_no; i++) {
	///Verifica se o ponto esta contido no bounding box do poligono
	if (p->x >= this->nodo[i]->min_x && p->x <= this->nodo[i]->max_x) {
	    if (p->y >= this->nodo[i]->min_y && p->y <= this->nodo[i]->max_y) {
		///Se for um poligono triangular ou rotacionado
		if (this->nodo[i]->mesh->tipo != 0) {
		    if (verifica_dentro_poligono(this->nodo[i]->mesh->pontos, p))
			return i;
		}
		else
		    return i;
	    }
	}
    }
    
    return -1;
}

void Grafo::penaliza_transicao_mesh (int m, int destino) {
    if (m == -1) {
	printf("\n\nERRO: Não foi encontrada a mesh a ser penalizada!\n\n");
	exit(0);
    }
    
    int pai = this->nodo[m]->pai;
    
    ///Se a mesh penalizada for a inicial
    if (pai == -1 || (pai != -1 && !this->nodo[pai]->pertence_caminho)) {
	///Encontra-se qual a mesh filho e penaliza a transição para a mesma
	for (int i = 0; i < this->num_no; i++) {
	    if (this->nodo[i]->pai == m && this->nodo[i]->pertence_caminho) {
		this->peso[m][i] += 1;
		return;
	    }
	}
	///Caso a mesh inicial seja também a final, então a mesma não possui filhos
	this->peso[m][m] += 1;
	return;
    }
    else {
	this->peso[pai][m] += 1;
	///Penaliza a proxima transicao tambem
	if (this->modo_furtivo) {
	    for (int i = 0; i < this->num_no; i++) {
		if (this->nodo[i]->pai == m && this->nodo[i]->pertence_caminho) {
		    this->peso[m][i] += 1;
		    return;
		}	
	    }
	}
    }
}
//*********************************************************************
//                        Núcleo do A*
//*********************************************************************
float Grafo::encontra_g (int m1, int m2, Ponto_Mapa *p, bool start) {
    float dx, dy, dx2 = 0, dy2 = 0, dist_m1, dist_m2;
    
    ///Se o agente, quando passa de m1 para m2, encontra um inimigo, esta transicao recebe uma distancia máxima
    if (this->peso[m1][m2] != 0) {
	return this->peso[m1][m2] * (PESO_IMPEDIMENTO / LIMITE_IMPEDIMENTO);
    }
    
    if (p == NULL) {
	dx = this->nodo[m1]->x - this->nodo[m2]->x;
	dy = this->nodo[m1]->y - this->nodo[m2]->y;
	///Isso serve para pesar mais quando o agente anda em meshes muito grandes (não sei se funciona bem mesmo....)
	if (this->nodo[m1]->pai != -1)	{
	    dx2 = this->nodo[this->nodo[m1]->pai]->x - this->nodo[m1]->x;
	    dy2 = this->nodo[this->nodo[m1]->pai]->y - this->nodo[m1]->y;
	}
    }
    else {
	if (start) {
	    dx = p->x - this->nodo[m2]->x;
	    dy = p->y - this->nodo[m2]->y;
	}
	else {
	    dx = p->x - this->nodo[m1]->x;
	    dy = p->y - this->nodo[m1]->y;
	}
    }
    float dist = sqrt(dx*dx + dy*dy) + sqrt(dx2*dx2 + dy2*dy2);
    
    if (this->modo_furtivo) {
	///Penalização por andar fora de cobertura
	if ((this->nodo[m1]->mesh->tipo==0 || this->nodo[m1]->mesh->tipo==3) && (this->nodo[m2]->mesh->tipo==0 || this->nodo[m2]->mesh->tipo==3))
	    dist += PESO_COBERTURA_NEG*dist;
	///Penalização pela distancia andada fora de cobertura ate entrar em uma
	else if ((this->nodo[m1]->mesh->tipo==0 || this->nodo[m1]->mesh->tipo==3) && (this->nodo[m2]->mesh->tipo==1 || this->nodo[m2]->mesh->tipo==2)) 
	    dist += PESO_COBERTURA_NEG*dist;
	///Penalização por sair da cobertura
	else if ((this->nodo[m1]->mesh->tipo==1 || this->nodo[m1]->mesh->tipo==2) && (this->nodo[m2]->mesh->tipo==0 || this->nodo[m2]->mesh->tipo==3)) 
	    dist += PESO_SAI_COBERTURA*dist;
	///Recompensa por andar em cobertura
	else if ((this->nodo[m1]->mesh->tipo==1 || this->nodo[m1]->mesh->tipo==2) && (this->nodo[m2]->mesh->tipo==1 || this->nodo[m2]->mesh->tipo==2))
	    dist -= PESO_MANTEM_COBERTURA*dist;
		
	for (int i = 0; i < this->num_inimigos; i++) {
	    if (!inimigo[i]->morto) {
		dx = this->nodo[m1]->x - this->inimigo[i]->path.atual.x;
		dy = this->nodo[m1]->y - this->inimigo[i]->path.atual.y;
		dx2 = this->nodo[m2]->x - this->inimigo[i]->path.atual.x;
		dy2 = this->nodo[m2]->y - this->inimigo[i]->path.atual.y;
		dist_m1 = sqrt(dx*dx + dy*dy);
		dist_m2 = sqrt(dx2*dx2 + dy2*dy2);
		if (dist_m2 < DISTANCIA_SEGURANCA)
			dist += dist*PESO_AREA_RISCO;
		if (dist_m1 > dist_m2)
			dist += dist*PESO_APROXIMA;
	    }
	}
    }
	    
    return dist;
}
//----------------------------------------------------------------------
float Grafo::encontra_h (int atual, Ponto_Mapa *destino, Ponto_Mapa *start) {
	if (atual == -1) {
	    printf("ERRO: Valor incorreto para a função encontra_h: atual == -1\n");
	    exit(0);
	}
	float dx, dy;
	if (start == NULL) {
	    dx = this->nodo[atual]->x - destino->x;
	    dy = this->nodo[atual]->y - destino->y;
	}
	else {
	    dx = start->x - destino->x;
	    dy = start->y - destino->y;
	}
	float dist = sqrt(dx*dx + dy*dy);
	
	if (this->modo_furtivo) {
	    if (this->nodo[atual]->mesh->tipo == 1 || this->nodo[atual]->mesh->tipo == 2)
		dist -= PESO_H_COBERTURA*dist;
	}
	
	return dist;
}
//----------------------------------------------------------------------
/// A*
bool Grafo::encontra_menor_caminho (Ponto_Mapa *start, int mesh_start, Ponto_Mapa *destino, int mesh_destino) {
    this->open->insere_ordenado(mesh_start, encontra_h(mesh_start, destino, start));
    this->nodo[mesh_start]->estado = 1;
    
    Item *atual;
    bool fim = false;
    bool encontrado = false;
    while (!fim) {
	atual = this->open->remove_topo();
	if (atual == NULL) {
	    //printf("Caminho não encontrado!\n");
	    fim = true;
	    encontrado = false;
	    break;
	}
	if (((this->peso[mesh_start][mesh_start] == 0 || atual->id != mesh_start) && this->modo_furtivo) || (!this->modo_furtivo)) {
	    this->closed->insere_inicio(atual->id, atual->f);
	    this->nodo[atual->id]->estado = -1;
	    if (atual->id == mesh_destino) {
		fim = true;
		if (this->nodo[atual->id]->f >= PESO_IMPEDIMENTO) {
		    /*if (this->modo_furtivo)
			printf("[FURTIVO]Não existe caminho sem ser visto pelo inimigo!\n");
		    else
			printf("[NORMAL]Não existe caminho sem ser visto pelo inimigo!\n");*/
		    encontrado = false;
		    return encontrado;
		}
		//printf("Custo = %f\n", this->nodo[atual->id]->f);
		encontrado = true;
		return encontrado;
	    }
	}
	else {
	    this->caminho_circular = true;
	    this->nodo[atual->id]->f = PESO_IMPEDIMENTO;
	    this->peso[mesh_start][mesh_start] = 0;
	}
	for (int k = 0; k < this->num_no; k++) {
	    if (k != atual->id && this->m_adj[atual->id][k] != -1 && this->nodo[k]->estado != -1) {
		if (this->nodo[k]->estado == 0) {
		    if (atual->id == mesh_start) 
			this->nodo[k]->g = encontra_g(atual->id, k, start, true);
		    else if (k == mesh_destino)
			this->nodo[k]->g = this->nodo[atual->id]->g + encontra_g(atual->id, k, destino, false);
		    else
			this->nodo[k]->g = this->nodo[atual->id]->g + this->m_adj[atual->id][k];
		    this->nodo[k]->h = encontra_h(k, destino, NULL);
		    this->nodo[k]->f = this->nodo[k]->g + this->nodo[k]->h;
		    this->nodo[k]->pai = atual->id;
		    this->nodo[k]->estado = 1;
		    this->open->insere_ordenado(k, this->nodo[k]->f);
		}
		else {
		    float g_temp = this->nodo[atual->id]->g + this->m_adj[atual->id][k];
		    float f_temp = this->nodo[k]->h + g_temp;
		    if (f_temp < this->nodo[k]->f) {
			this->nodo[k]->pai = atual->id;
			this->nodo[k]->g = g_temp;
			this->nodo[k]->f = f_temp;
			free(this->open->remove(k));
			this->open->insere_ordenado(k, this->nodo[k]->f);
		    }
		}
	    }
	}
	
	if (atual)
		free(atual);
    }
    
    return encontrado;
}































