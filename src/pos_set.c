#include "pos_set.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int cpos_iniciar(ConjuntoPos *conjunto, int tam) {
    if (!conjunto || tam < 0) {
        return 0;
    }

    conjunto->bits = (unsigned char *)calloc((size_t)(tam + 1), sizeof(unsigned char));
    if (!conjunto->bits) {
        return 0;
    }

    conjunto->tam = tam;
    return 1;
}

void cpos_liberar(ConjuntoPos *conjunto) {
    if (!conjunto) {
        return;
    }

    free(conjunto->bits);
    conjunto->bits = NULL;
    conjunto->tam = 0;
}

void cpos_limpiar(ConjuntoPos *conjunto) {
    if (!conjunto || !conjunto->bits) {
        return;
    }

    memset(conjunto->bits, 0, (size_t)(conjunto->tam + 1));
}

void cpos_copiar(ConjuntoPos *destino, const ConjuntoPos *origen) {
    if (!destino || !origen || !destino->bits || !origen->bits || destino->tam != origen->tam) {
        return;
    }

    memcpy(destino->bits, origen->bits, (size_t)(origen->tam + 1));
}

void cpos_agregar(ConjuntoPos *conjunto, int posicion) {
    if (!conjunto || !conjunto->bits || posicion < 1 || posicion > conjunto->tam) {
        return;
    }

    conjunto->bits[posicion] = 1;
}

int cpos_contiene(const ConjuntoPos *conjunto, int posicion) {
    if (!conjunto || !conjunto->bits || posicion < 1 || posicion > conjunto->tam) {
        return 0;
    }

    return conjunto->bits[posicion] != 0;
}

void cpos_unir_en(ConjuntoPos *destino, const ConjuntoPos *origen) {
    int i;

    if (!destino || !origen || !destino->bits || !origen->bits || destino->tam != origen->tam) {
        return;
    }

    for (i = 1; i <= destino->tam; i++) {
        if (origen->bits[i]) {
            destino->bits[i] = 1;
        }
    }
}

int cpos_iguales(const ConjuntoPos *a, const ConjuntoPos *b) {
    if (!a || !b || !a->bits || !b->bits || a->tam != b->tam) {
        return 0;
    }

    return memcmp(a->bits, b->bits, (size_t)(a->tam + 1)) == 0;
}

int cpos_esta_vacio(const ConjuntoPos *conjunto) {
    int i;

    if (!conjunto || !conjunto->bits) {
        return 1;
    }

    for (i = 1; i <= conjunto->tam; i++) {
        if (conjunto->bits[i]) {
            return 0;
        }
    }

    return 1;
}

char *cpos_a_cadena(const ConjuntoPos *conjunto) {
    int i;
    int primero = 1;
    size_t capacidad = 32;
    size_t largo = 0;
    char *buffer;

    if (!conjunto || !conjunto->bits) {
        return NULL;
    }

    buffer = (char *)malloc(capacidad);
    if (!buffer) {
        return NULL;
    }

    buffer[largo++] = '{';
    for (i = 1; i <= conjunto->tam; i++) {
        if (!conjunto->bits[i]) {
            continue;
        }

        if (!primero) {
            if (largo + 2 >= capacidad) {
                capacidad *= 2;
                buffer = (char *)realloc(buffer, capacidad);
                if (!buffer) {
                    return NULL;
                }
            }
            buffer[largo++] = ',';
        }

        {
            char numero[32];
            int escritos = snprintf(numero, sizeof(numero), "%d", i);
            size_t necesarios = (size_t)(escritos > 0 ? escritos : 0);

            if (largo + necesarios + 2 >= capacidad) {
                while (largo + necesarios + 2 >= capacidad) {
                    capacidad *= 2;
                }
                buffer = (char *)realloc(buffer, capacidad);
                if (!buffer) {
                    return NULL;
                }
            }

            if (escritos > 0) {
                memcpy(buffer + largo, numero, necesarios);
                largo += necesarios;
            }
        }

        primero = 0;
    }

    if (largo + 2 >= capacidad) {
        capacidad += 2;
        buffer = (char *)realloc(buffer, capacidad);
        if (!buffer) {
            return NULL;
        }
    }

    buffer[largo++] = '}';
    buffer[largo] = '\0';
    return buffer;
}
