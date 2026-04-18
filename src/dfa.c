#include "dfa.h"

#include <stdio.h>
#include <stdlib.h>

static void afd_inicializar(AFD *afd) {
    int i;
    afd->estados = NULL;
    afd->cantidad = 0;
    afd->capacidad = 0;

    afd->alfabeto = NULL;
    afd->tam_alfabeto = 0;

    afd->cantidad_posiciones = 0;
    afd->posicion_hash = 0;
    afd->estado_inicial = -1;

    for (i = 0; i < 256; i++) {
        afd->indice_alfabeto[i] = -1;
    }
}

void afd_liberar(AFD *afd) {
    int i;
    if (!afd) {
        return;
    }

    for (i = 0; i < afd->cantidad; i++) {
        cpos_liberar(&afd->estados[i].posiciones);
        free(afd->estados[i].transiciones);
    }

    free(afd->estados);
    free(afd->alfabeto);

    afd_inicializar(afd);
}

static int afd_expandir_estados(AFD *afd) {
    int nueva_capacidad;
    EstadoAFD *nuevos_estados;

    if (afd->cantidad < afd->capacidad) {
        return 1;
    }

    nueva_capacidad = (afd->capacidad == 0) ? 8 : afd->capacidad * 2;
    nuevos_estados = (EstadoAFD *)realloc(afd->estados, (size_t)nueva_capacidad * sizeof(EstadoAFD));
    if (!nuevos_estados) {
        return 0;
    }

    afd->estados = nuevos_estados;
    afd->capacidad = nueva_capacidad;
    return 1;
}

static int afd_buscar_estado(const AFD *afd, const ConjuntoPos *posiciones) {
    int i;
    for (i = 0; i < afd->cantidad; i++) {
        if (cpos_iguales(&afd->estados[i].posiciones, posiciones)) {
            return i;
        }
    }
    return -1;
}

static int afd_agregar_estado(AFD *afd, const ConjuntoPos *posiciones) {
    int i;
    int indice;

    if (!afd_expandir_estados(afd)) {
        return -1;
    }

    indice = afd->cantidad;

    if (!cpos_iniciar(&afd->estados[indice].posiciones, afd->cantidad_posiciones)) {
        return -1;
    }
    cpos_copiar(&afd->estados[indice].posiciones, posiciones);

    afd->estados[indice].transiciones = (int *)malloc((size_t)afd->tam_alfabeto * sizeof(int));
    if (!afd->estados[indice].transiciones) {
        cpos_liberar(&afd->estados[indice].posiciones);
        return -1;
    }

    for (i = 0; i < afd->tam_alfabeto; i++) {
        afd->estados[indice].transiciones[i] = -1;
    }

    afd->estados[indice].es_aceptacion = cpos_contiene(posiciones, afd->posicion_hash);
    afd->estados[indice].marcado = 0;

    afd->cantidad++;
    return indice;
}

static int afd_construir_alfabeto(AFD *afd, const char *simbolos_pos) {
    int i;
    int usado[256] = {0};
    int cantidad = 0;

    for (i = 1; i <= afd->cantidad_posiciones; i++) {
        unsigned char c = (unsigned char)simbolos_pos[i];
        if (c == (unsigned char)'#') {
            continue;
        }
        if (!usado[c]) {
            usado[c] = 1;
            cantidad++;
        }
    }

    afd->alfabeto = (char *)malloc((size_t)cantidad * sizeof(char));
    if (!afd->alfabeto && cantidad > 0) {
        return 0;
    }

    afd->tam_alfabeto = cantidad;
    cantidad = 0;
    for (i = 0; i < 256; i++) {
        if (usado[i]) {
            afd->alfabeto[cantidad] = (char)i;
            afd->indice_alfabeto[i] = cantidad;
            cantidad++;
        }
    }

    return 1;
}

int afd_construir(
    AFD *afd,
    const NodoSintaxis *raiz,
    const ConjuntoPos *followpos,
    const char *simbolos_pos,
    int cantidad_posiciones,
    int posicion_hash
) {
    int actual;
    int i;
    ConjuntoPos temporal;

    if (!afd || !raiz || !followpos || !simbolos_pos || cantidad_posiciones <= 0 || posicion_hash <= 0) {
        return 0;
    }

    afd_inicializar(afd);
    afd->cantidad_posiciones = cantidad_posiciones;
    afd->posicion_hash = posicion_hash;

    if (!afd_construir_alfabeto(afd, simbolos_pos)) {
        afd_liberar(afd);
        return 0;
    }

    afd->estado_inicial = afd_agregar_estado(afd, &raiz->primera_pos);
    if (afd->estado_inicial < 0) {
        afd_liberar(afd);
        return 0;
    }

    if (!cpos_iniciar(&temporal, cantidad_posiciones)) {
        afd_liberar(afd);
        return 0;
    }

    /* Algoritmo directo: procesar estados no marcados. */
    while (1) {
        actual = -1;
        for (i = 0; i < afd->cantidad; i++) {
            if (!afd->estados[i].marcado) {
                actual = i;
                break;
            }
        }

        if (actual < 0) {
            break;
        }

        afd->estados[actual].marcado = 1;

        for (i = 0; i < afd->tam_alfabeto; i++) {
            int p;
            int destino;
            char simbolo = afd->alfabeto[i];

            cpos_limpiar(&temporal);

            for (p = 1; p <= cantidad_posiciones; p++) {
                if (cpos_contiene(&afd->estados[actual].posiciones, p) && simbolos_pos[p] == simbolo) {
                    cpos_unir_en(&temporal, &followpos[p]);
                }
            }

            if (cpos_esta_vacio(&temporal)) {
                continue;
            }

            destino = afd_buscar_estado(afd, &temporal);
            if (destino < 0) {
                destino = afd_agregar_estado(afd, &temporal);
                if (destino < 0) {
                    cpos_liberar(&temporal);
                    afd_liberar(afd);
                    return 0;
                }
            }

            afd->estados[actual].transiciones[i] = destino;
        }
    }

    cpos_liberar(&temporal);
    return 1;
}

void afd_imprimir(const AFD *afd) {
    int i;
    int j;

    if (!afd) {
        return;
    }

    printf("\n=== AFD Construido ===\n");
    printf("Alfabeto: { ");
    for (i = 0; i < afd->tam_alfabeto; i++) {
        printf("%c", afd->alfabeto[i]);
        if (i + 1 < afd->tam_alfabeto) {
            printf(", ");
        }
    }
    printf(" }\n\n");

    for (i = 0; i < afd->cantidad; i++) {
        char *texto_conjunto = cpos_a_cadena(&afd->estados[i].posiciones);
        printf("q%d %s %s\n", i, afd->estados[i].es_aceptacion ? "(Aceptacion)" : "", texto_conjunto ? texto_conjunto : "{}");
        free(texto_conjunto);

        for (j = 0; j < afd->tam_alfabeto; j++) {
            int destino = afd->estados[i].transiciones[j];
            if (destino >= 0) {
                printf("  --%c--> q%d\n", afd->alfabeto[j], destino);
            }
        }
    }
}

int afd_cadena_solo_alfabeto(const AFD *afd, const char *entrada, int *indice_invalido, char *caracter_invalido) {
    int i;
    if (!afd || !entrada) {
        return 0;
    }

    for (i = 0; entrada[i] != '\0'; i++) {
        unsigned char c = (unsigned char)entrada[i];
        if (afd->indice_alfabeto[c] < 0) {
            if (indice_invalido) {
                *indice_invalido = i;
            }
            if (caracter_invalido) {
                *caracter_invalido = entrada[i];
            }
            return 0;
        }
    }

    return 1;
}

int afd_validar_cadena(const AFD *afd, const char *entrada) {
    int estado;
    int i;

    if (!afd || !entrada) {
        return 0;
    }

    estado = afd->estado_inicial;
    for (i = 0; entrada[i] != '\0'; i++) {
        unsigned char c = (unsigned char)entrada[i];
        int indice_simbolo = afd->indice_alfabeto[c];
        if (indice_simbolo < 0) {
            return 0;
        }

        estado = afd->estados[estado].transiciones[indice_simbolo];
        if (estado < 0) {
            return 0;
        }
    }

    return afd->estados[estado].es_aceptacion;
}
