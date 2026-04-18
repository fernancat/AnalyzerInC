#ifndef CONJUNTO_POS_H
#define CONJUNTO_POS_H

#include <stddef.h>

/*
 * Conjunto de posiciones representado con bits (0/1).
 * Se usa para firstpos, lastpos, followpos y estados del AFD.
 */
typedef struct {
    unsigned char *bits;
    int tam;
} ConjuntoPos;

int cpos_iniciar(ConjuntoPos *conjunto, int tam);
void cpos_liberar(ConjuntoPos *conjunto);
void cpos_limpiar(ConjuntoPos *conjunto);
void cpos_copiar(ConjuntoPos *destino, const ConjuntoPos *origen);
void cpos_agregar(ConjuntoPos *conjunto, int posicion);
int cpos_contiene(const ConjuntoPos *conjunto, int posicion);
void cpos_unir_en(ConjuntoPos *destino, const ConjuntoPos *origen);
int cpos_iguales(const ConjuntoPos *a, const ConjuntoPos *b);
int cpos_esta_vacio(const ConjuntoPos *conjunto);
char *cpos_a_cadena(const ConjuntoPos *conjunto);

#endif
