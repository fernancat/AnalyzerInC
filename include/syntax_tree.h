#ifndef ARBOL_SINTAXIS_H
#define ARBOL_SINTAXIS_H

#include "pos_set.h"

/* Tipos de nodos que pueden existir en la ER. */
typedef enum {
    NODO_LITERAL,
    NODO_UNION,
    NODO_CONCAT,
    NODO_KLEENE
} TipoNodo;

/*
 * Nodo del arbol de sintaxis aumentado.
 * Guarda informacion estructural y los conjuntos necesarios
 * para el algoritmo directo ER -> AFD.
 */
typedef struct NodoSintaxis {
    TipoNodo tipo;
    char simbolo;
    int posicion;
    int anulable;
    int conjuntos_inicializados;
    ConjuntoPos primera_pos;
    ConjuntoPos ultima_pos;
    struct NodoSintaxis *izq;
    struct NodoSintaxis *der;
} NodoSintaxis;

NodoSintaxis *nodo_crear_literal(char simbolo);
NodoSintaxis *nodo_crear_unario(TipoNodo tipo, NodoSintaxis *hijo);
NodoSintaxis *nodo_crear_binario(TipoNodo tipo, NodoSintaxis *izq, NodoSintaxis *der);
void arbol_liberar(NodoSintaxis *nodo);

void arbol_imprimir(const NodoSintaxis *nodo, int profundidad);

int arbol_contar_hojas(const NodoSintaxis *nodo);
void arbol_asignar_posiciones(NodoSintaxis *nodo, int *siguiente_pos, char *simbolos_pos, int *posicion_hash);
int arbol_inicializar_conjuntos(NodoSintaxis *nodo, int cantidad_posiciones);

void arbol_calcular_funciones(NodoSintaxis *nodo, ConjuntoPos *followpos, int cantidad_posiciones);

#endif
