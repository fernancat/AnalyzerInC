#include "syntax_tree.h"

#include <stdio.h>
#include <stdlib.h>

static void imprimir_indentacion(int profundidad) {
    int i;
    for (i = 0; i < profundidad; i++) {
        printf("  ");
    }
}

NodoSintaxis *nodo_crear_literal(char simbolo) {
    NodoSintaxis *nodo = (NodoSintaxis *)calloc(1, sizeof(NodoSintaxis));
    if (!nodo) {
        return NULL;
    }

    nodo->tipo = NODO_LITERAL;
    nodo->simbolo = simbolo;
    nodo->posicion = 0;
    return nodo;
}

NodoSintaxis *nodo_crear_unario(TipoNodo tipo, NodoSintaxis *hijo) {
    NodoSintaxis *nodo;

    if (!hijo || tipo != NODO_KLEENE) {
        return NULL;
    }

    nodo = (NodoSintaxis *)calloc(1, sizeof(NodoSintaxis));
    if (!nodo) {
        return NULL;
    }

    nodo->tipo = tipo;
    nodo->izq = hijo;
    return nodo;
}

NodoSintaxis *nodo_crear_binario(TipoNodo tipo, NodoSintaxis *izq, NodoSintaxis *der) {
    NodoSintaxis *nodo;

    if (!izq || !der || (tipo != NODO_UNION && tipo != NODO_CONCAT)) {
        return NULL;
    }

    nodo = (NodoSintaxis *)calloc(1, sizeof(NodoSintaxis));
    if (!nodo) {
        return NULL;
    }

    nodo->tipo = tipo;
    nodo->izq = izq;
    nodo->der = der;
    return nodo;
}

void arbol_liberar(NodoSintaxis *nodo) {
    if (!nodo) {
        return;
    }

    arbol_liberar(nodo->izq);
    arbol_liberar(nodo->der);

    if (nodo->conjuntos_inicializados) {
        cpos_liberar(&nodo->primera_pos);
        cpos_liberar(&nodo->ultima_pos);
    }

    free(nodo);
}

void arbol_imprimir(const NodoSintaxis *nodo, int profundidad) {
    if (!nodo) {
        return;
    }

    imprimir_indentacion(profundidad);
    switch (nodo->tipo) {
        case NODO_LITERAL:
            if (nodo->posicion > 0) {
                printf("'%c' [pos=%d]\n", nodo->simbolo, nodo->posicion);
            } else {
                printf("'%c'\n", nodo->simbolo);
            }
            break;
        case NODO_UNION:
            printf("|\n");
            break;
        case NODO_CONCAT:
            printf(".\n");
            break;
        case NODO_KLEENE:
            printf("*\n");
            break;
    }

    arbol_imprimir(nodo->izq, profundidad + 1);
    arbol_imprimir(nodo->der, profundidad + 1);
}

int arbol_contar_hojas(const NodoSintaxis *nodo) {
    if (!nodo) {
        return 0;
    }
    if (nodo->tipo == NODO_LITERAL) {
        return 1;
    }

    return arbol_contar_hojas(nodo->izq) + arbol_contar_hojas(nodo->der);
}

void arbol_asignar_posiciones(NodoSintaxis *nodo, int *siguiente_pos, char *simbolos_pos, int *posicion_hash) {
    if (!nodo || !siguiente_pos || !simbolos_pos) {
        return;
    }

    if (nodo->tipo == NODO_LITERAL) {
        nodo->posicion = *siguiente_pos;
        simbolos_pos[*siguiente_pos] = nodo->simbolo;
        if (nodo->simbolo == '#') {
            *posicion_hash = *siguiente_pos;
        }
        (*siguiente_pos)++;
        return;
    }

    arbol_asignar_posiciones(nodo->izq, siguiente_pos, simbolos_pos, posicion_hash);
    arbol_asignar_posiciones(nodo->der, siguiente_pos, simbolos_pos, posicion_hash);
}

int arbol_inicializar_conjuntos(NodoSintaxis *nodo, int cantidad_posiciones) {
    if (!nodo) {
        return 1;
    }

    if (!arbol_inicializar_conjuntos(nodo->izq, cantidad_posiciones)) {
        return 0;
    }
    if (!arbol_inicializar_conjuntos(nodo->der, cantidad_posiciones)) {
        return 0;
    }

    if (!cpos_iniciar(&nodo->primera_pos, cantidad_posiciones)) {
        return 0;
    }
    if (!cpos_iniciar(&nodo->ultima_pos, cantidad_posiciones)) {
        cpos_liberar(&nodo->primera_pos);
        return 0;
    }

    nodo->conjuntos_inicializados = 1;
    return 1;
}

/* Aplica followpos[p] = followpos[p] U conjunto_destino para cada p en conjunto_origen. */
static void agregar_enlaces_follow(
    const ConjuntoPos *conjunto_origen,
    const ConjuntoPos *conjunto_destino,
    ConjuntoPos *followpos,
    int cantidad_posiciones
) {
    int i;
    for (i = 1; i <= cantidad_posiciones; i++) {
        if (cpos_contiene(conjunto_origen, i)) {
            cpos_unir_en(&followpos[i], conjunto_destino);
        }
    }
}

void arbol_calcular_funciones(NodoSintaxis *nodo, ConjuntoPos *followpos, int cantidad_posiciones) {
    if (!nodo) {
        return;
    }

    arbol_calcular_funciones(nodo->izq, followpos, cantidad_posiciones);
    arbol_calcular_funciones(nodo->der, followpos, cantidad_posiciones);

    cpos_limpiar(&nodo->primera_pos);
    cpos_limpiar(&nodo->ultima_pos);

    switch (nodo->tipo) {
        case NODO_LITERAL:
            nodo->anulable = 0;
            cpos_agregar(&nodo->primera_pos, nodo->posicion);
            cpos_agregar(&nodo->ultima_pos, nodo->posicion);
            break;

        case NODO_UNION:
            nodo->anulable = nodo->izq->anulable || nodo->der->anulable;
            cpos_unir_en(&nodo->primera_pos, &nodo->izq->primera_pos);
            cpos_unir_en(&nodo->primera_pos, &nodo->der->primera_pos);
            cpos_unir_en(&nodo->ultima_pos, &nodo->izq->ultima_pos);
            cpos_unir_en(&nodo->ultima_pos, &nodo->der->ultima_pos);
            break;

        case NODO_CONCAT:
            nodo->anulable = nodo->izq->anulable && nodo->der->anulable;

            cpos_unir_en(&nodo->primera_pos, &nodo->izq->primera_pos);
            if (nodo->izq->anulable) {
                cpos_unir_en(&nodo->primera_pos, &nodo->der->primera_pos);
            }

            cpos_unir_en(&nodo->ultima_pos, &nodo->der->ultima_pos);
            if (nodo->der->anulable) {
                cpos_unir_en(&nodo->ultima_pos, &nodo->izq->ultima_pos);
            }

            agregar_enlaces_follow(&nodo->izq->ultima_pos, &nodo->der->primera_pos, followpos, cantidad_posiciones);
            break;

        case NODO_KLEENE:
            nodo->anulable = 1;
            cpos_unir_en(&nodo->primera_pos, &nodo->izq->primera_pos);
            cpos_unir_en(&nodo->ultima_pos, &nodo->izq->ultima_pos);
            agregar_enlaces_follow(&nodo->izq->ultima_pos, &nodo->izq->primera_pos, followpos, cantidad_posiciones);
            break;
    }
}
