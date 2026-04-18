#ifndef AFD_H
#define AFD_H

#include "pos_set.h"
#include "syntax_tree.h"

/* Estado individual del AFD. */
typedef struct {
    ConjuntoPos posiciones;
    int *transiciones;
    int es_aceptacion;
    int marcado;
} EstadoAFD;

/* Estructura completa del AFD. */
typedef struct {
    EstadoAFD *estados;
    int cantidad;
    int capacidad;

    char *alfabeto;
    int tam_alfabeto;
    int indice_alfabeto[256];

    int cantidad_posiciones;
    int posicion_hash;
    int estado_inicial;
} AFD;

int afd_construir(
    AFD *afd,
    const NodoSintaxis *raiz,
    const ConjuntoPos *followpos,
    const char *simbolos_pos,
    int cantidad_posiciones,
    int posicion_hash
);
void afd_liberar(AFD *afd);
void afd_imprimir(const AFD *afd);

int afd_cadena_solo_alfabeto(const AFD *afd, const char *entrada, int *indice_invalido, char *caracter_invalido);
int afd_validar_cadena(const AFD *afd, const char *entrada);

#endif
