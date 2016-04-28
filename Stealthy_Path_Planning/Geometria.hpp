#ifndef GEOMETRIA_HPP
#define GEOMETRIA_HPP

#include "A_Star.hpp"

#define EPSILON 0.01

typedef struct Vetor3D {
	float x, y, z;
}Vetor3D;

Vetor3D* prod_vetorial (Vetor3D *v1, Vetor3D *v2);
float prod_escalar (Vetor3D *v1, Vetor3D *v2);
float angulo_entre_vetores (Vetor3D *v1, Vetor3D *v2);
bool mesmo_lado (Vetor3D *v1, Vetor3D *v2, Vetor3D *aresta);
bool verifica_dentro_triangulo (Ponto_Mapa *pts, Ponto_Mapa *p);
bool verifica_dentro_poligono (Ponto_Mapa *pontos, Ponto_Mapa *p);
bool intersecao_boundingbox (Ponto_Mapa *a1, Ponto_Mapa *a2, Ponto_Mapa *b1, Ponto_Mapa *b2);
bool lado_do_ponto (Ponto_Mapa *a1, Ponto_Mapa *a2, Ponto_Mapa *b);
bool linhas_se_cruzam (Ponto_Mapa *a1, Ponto_Mapa *a2, Ponto_Mapa *b1, Ponto_Mapa *b2);
bool linha_cruza_triangulo (Ponto_Mapa *a1, Ponto_Mapa *a2, Ponto_Mapa *pontos);

#endif
