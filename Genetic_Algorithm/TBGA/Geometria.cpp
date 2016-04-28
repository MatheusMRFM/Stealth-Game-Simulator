#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "Geometria.hpp"

Vetor3D* prod_vetorial (Vetor3D *v1, Vetor3D *v2) {
	Vetor3D *r = (Vetor3D*) malloc (sizeof(Vetor3D));
	r->x = (v1->y * v2->z)-(v2->y * v1->z);
	r->y = -(v1->x * v2->z)-(v2->x * v1->z);
	r->z = (v1->x * v2->y)-(v2->x * v1->y);
	
	return r;
}

float prod_escalar (Vetor3D *v1, Vetor3D *v2) {
	return v1->x*v2->x + v1->y*v2->y + v1->z*v2->z;
}

float angulo_entre_vetores (Vetor3D *v1, Vetor3D *v2) {
	float produto = prod_escalar(v1, v2);
	float mod1 = sqrt(v1->x*v1->x + v1->y*v1->y);
	float mod2 = sqrt(v2->x*v2->x + v2->y*v2->y);
	float cos = produto / (float)(mod1*mod2);
	
	//printf("mod1 = %f\tmod2 = %f\tproduto = %f\tcos = %f\n", mod1, mod2, produto, cos);
	
	return acos(cos);
}

bool mesmo_lado (Vetor3D *v1, Vetor3D *v2, Vetor3D *aresta) {
	Vetor3D *cp1 = prod_vetorial(aresta, v1);
    Vetor3D *cp2 = prod_vetorial(aresta, v2);
    if (prod_escalar(cp1, cp2) > 0) {
		free(cp1);
		free(cp2);
		return true;
	}
    else {
		free(cp1);
		free(cp2);
		return false;
	}
}

bool verifica_dentro_triangulo (Ponto_Mapa *pts, Ponto_Mapa *p) {
	Ponto_Mapa pontos[4];
	pontos[0].x = pts[0].x; pontos[0].y = pts[0].y;
	pontos[1].x = pts[1].x; pontos[1].y = pts[1].y;
	pontos[2].x = pts[2].x; pontos[2].y = pts[2].y;
	///Cria um triangulo um pouco maior para arrumar erros numericos
	if (pontos[0].x < pontos[1].x) {
		pontos[1].x += EPSILON;
		pontos[2].y -= EPSILON;
	}
	else {
		pontos[1].y += EPSILON;
		pontos[2].x -= EPSILON;
	}
	
	Vetor3D aAB, aBC, aAC, aBA;	 //aAB = B-A, aBC = C-B, aAC = C-A, aBA = A - B
	aAB.x = pontos[1].x - pontos[0].x;		aAB.y = pontos[1].y - pontos[0].y;		aAB.z = 0;
	aBC.x = pontos[2].x - pontos[1].x;		aBC.y = pontos[2].y - pontos[1].y;		aBC.z = 0;
	aAC.x = pontos[2].x - pontos[0].x;		aAC.y = pontos[2].y - pontos[0].y;		aAC.z = 0;
	aBA.x = pontos[0].x - pontos[1].x;		aBA.y = pontos[0].y - pontos[1].y;		aBA.z = 0;
	
	Vetor3D pA, pB;
	pA.x = p->x - pontos[0].x;		pA.y = p->y - pontos[0].y;		pA.z = 0;
	pB.x = p->x - pontos[1].x;		pB.y = p->y - pontos[1].y;		pB.z = 0;
	//pC.x = p->x - pontos[2]->x;		pC.y = p->y - pontos[2]->y;		pC.z = 0;
	
	if (mesmo_lado(&pB, &aBA, &aBC) && mesmo_lado(&pA, &aAB, &aAC) && mesmo_lado(&pA, &aAC, &aAB))
		return true;
	else
		return false;
}

bool verifica_dentro_poligono (Ponto_Mapa *pontos, Ponto_Mapa *p) {
	///Se o poligono for um triangulo
	if (pontos[3].x == -1) 
		return verifica_dentro_triangulo(pontos, p);
	///Caso contrário
	else {
		Ponto_Mapa *aux = (Ponto_Mapa*) malloc (3*sizeof(Ponto_Mapa));
		aux[0].x = pontos[3].x;		aux[0].y = pontos[3].y;
		aux[1].x = pontos[1].x;		aux[1].y = pontos[1].y;
		aux[2].x = pontos[2].x;		aux[2].y = pontos[2].y;
		if (verifica_dentro_triangulo(pontos, p) || verifica_dentro_triangulo(aux, p)) {
			free(aux);
			return true;
		}
		else {
			free(aux);
			return false;
		}
	}
}

bool intersecao_boundingbox (Ponto_Mapa *a1, Ponto_Mapa *a2, Ponto_Mapa *b1, Ponto_Mapa *b2) {
	Poligono a, b;
	
	if (a1->x < a2->x) 
		a.pontos[0].x = a1->x;
	else
		a.pontos[3].x = a2->x;
	if (a1->y < a2->y) 
		a.pontos[0].y = a1->y;
	else
		a.pontos[3].y = a2->y;
		
	if (b1->x < b2->x) 
		b.pontos[0].x = b1->x;
	else
		b.pontos[3].x = b2->x;
	if (b1->y < b2->y) 
		b.pontos[0].y = b1->y;
	else
		b.pontos[3].y = b2->y;
		
	return (	a.pontos[0].x < b.pontos[3].x
			&& 	a.pontos[3].x > b.pontos[0].x 
			&&	a.pontos[0].y < b.pontos[3].y 
			&&	a.pontos[3].y > b.pontos[0].y);
}

bool lado_do_ponto (Ponto_Mapa *a1, Ponto_Mapa *a2, Ponto_Mapa *b) {
	Vetor3D v1, v2, *r;
	
	v1.x = a2->x - a1->x;		v1.y = a2->y - a1->y;		v1.z = 0;
	v2.x = b->x - a1->x;		v2.y = b->y - a1->y;		v2.z = 0;
	
	r = prod_vetorial (&v1, &v2);
	
	if (r->z < 0) {
		free(r);
		return true;
	}
	else {
		free(r);
		return false;
	}
}

bool linhas_se_cruzam (Ponto_Mapa *a1, Ponto_Mapa *a2, Ponto_Mapa *b1, Ponto_Mapa *b2) {
	bool a_b1, a_b2, b_a1, b_a2;
	a_b1 = lado_do_ponto(a1, a2, b1);
	a_b2 = lado_do_ponto(a1, a2, b2);
	
	///Verifica se o segmento b1-b2 cruza com a reta que contém o segmento a1-a2
	if (a_b1 != a_b2) {
		b_a1 = lado_do_ponto(b1, b2, a1);
		b_a2 = lado_do_ponto(b1, b2, a2);
		///Verifica se o segmento a1-a2 cruza com a reta que contém o segmento b1-b2
		if (b_a1 != b_a2) {
			return true;
		}
	}
	
	return false;
}

bool linha_cruza_triangulo (Ponto_Mapa *a1, Ponto_Mapa *a2, Ponto_Mapa *pontos) {
	bool a_b1, a_b2, a_b3;
	
	a_b1 = linhas_se_cruzam (a1, a2, &pontos[0], &pontos[1]);
	a_b2 = linhas_se_cruzam (a1, a2, &pontos[0], &pontos[2]);
	a_b3 = linhas_se_cruzam (a1, a2, &pontos[2], &pontos[3]);
	
	return (a_b1 || a_b2 || a_b3);
}
























