#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dfa.h"
#include "regex_parser.h"
#include "syntax_tree.h"

#define TAM_BUFFER_ENTRADA 1024

static void quitar_salto_linea(char *texto) {
    size_t largo;

    if (!texto) {
        return;
    }

    largo = strlen(texto);
    while (largo > 0 && (texto[largo - 1] == '\n' || texto[largo - 1] == '\r')) {
        texto[largo - 1] = '\0';
        largo--;
    }
}

static void imprimir_tabla_followpos(const ConjuntoPos *followpos, const char *simbolos_pos, int cantidad_posiciones) {
    int i;

    printf("\n=== Tabla FollowPos ===\n");
    for (i = 1; i <= cantidad_posiciones; i++) {
        char *texto = cpos_a_cadena(&followpos[i]);
        printf("pos %d ('%c') -> %s\n", i, simbolos_pos[i], texto ? texto : "{}");
        free(texto);
    }
}

static int preguntar_continuar(void) {
    char linea[32];

    printf("\nDesea ingresar otra cadena? (S/N): ");
    if (!fgets(linea, sizeof(linea), stdin)) {
        return 0;
    }

    return linea[0] == 'S' || linea[0] == 's';
}

int main(int argc, char **argv) {
    NodoSintaxis *raiz_original;
    NodoSintaxis *hoja_hash;
    NodoSintaxis *raiz_aumentada;

    int cantidad_posiciones;
    int siguiente_posicion;
    int posicion_hash = 0;
    int i;

    char *simbolos_pos = NULL;
    ConjuntoPos *followpos = NULL;
    AFD afd;

    char error_er[256];
    char linea_entrada[TAM_BUFFER_ENTRADA];

    if (argc < 2) {
        fprintf(stderr, "Uso: %s <expresion_regular>\n", argv[0]);
        fprintf(stderr, "Ejemplo: %s \"(a|b)*abb\"\n", argv[0]);
        return 1;
    }

    /* 1) Parsear ER recibida por linea de comandos. */
    raiz_original = parsear_er(argv[1], error_er, sizeof(error_er));
    if (!raiz_original) {
        fprintf(stderr, "Error al parsear ER: %s\n", error_er[0] ? error_er : "expresion invalida");
        return 1;
    }

    printf("Expresion regular recibida: %s\n", argv[1]);
    printf("\n=== Arbol de la ER Ingresada ===\n");
    arbol_imprimir(raiz_original, 0);

    /* 2) Aumentar ER a ER.# para identificar aceptacion. */
    hoja_hash = nodo_crear_literal('#');
    if (!hoja_hash) {
        fprintf(stderr, "Error: no hay memoria para el nodo final '#'.\n");
        arbol_liberar(raiz_original);
        return 1;
    }

    raiz_aumentada = nodo_crear_binario(NODO_CONCAT, raiz_original, hoja_hash);
    if (!raiz_aumentada) {
        fprintf(stderr, "Error: no hay memoria para el arbol aumentado.\n");
        arbol_liberar(raiz_original);
        arbol_liberar(hoja_hash);
        return 1;
    }

    cantidad_posiciones = arbol_contar_hojas(raiz_aumentada);
    if (cantidad_posiciones <= 0) {
        fprintf(stderr, "Error: la ER no genero hojas validas.\n");
        arbol_liberar(raiz_aumentada);
        return 1;
    }

    simbolos_pos = (char *)calloc((size_t)(cantidad_posiciones + 1), sizeof(char));
    if (!simbolos_pos) {
        fprintf(stderr, "Error: no hay memoria para simbolos por posicion.\n");
        arbol_liberar(raiz_aumentada);
        return 1;
    }

    siguiente_posicion = 1;
    arbol_asignar_posiciones(raiz_aumentada, &siguiente_posicion, simbolos_pos, &posicion_hash);

    if (posicion_hash <= 0) {
        fprintf(stderr, "Error interno: no se encontro la posicion de '#'.\n");
        free(simbolos_pos);
        arbol_liberar(raiz_aumentada);
        return 1;
    }

    if (!arbol_inicializar_conjuntos(raiz_aumentada, cantidad_posiciones)) {
        fprintf(stderr, "Error: no hay memoria para firstpos/lastpos.\n");
        free(simbolos_pos);
        arbol_liberar(raiz_aumentada);
        return 1;
    }

    followpos = (ConjuntoPos *)calloc((size_t)(cantidad_posiciones + 1), sizeof(ConjuntoPos));
    if (!followpos) {
        fprintf(stderr, "Error: no hay memoria para followpos.\n");
        free(simbolos_pos);
        arbol_liberar(raiz_aumentada);
        return 1;
    }

    for (i = 1; i <= cantidad_posiciones; i++) {
        if (!cpos_iniciar(&followpos[i], cantidad_posiciones)) {
            fprintf(stderr, "Error: no hay memoria para followpos[%d].\n", i);
            while (--i >= 1) {
                cpos_liberar(&followpos[i]);
            }
            free(followpos);
            free(simbolos_pos);
            arbol_liberar(raiz_aumentada);
            return 1;
        }
    }

    /* 3) Calcular nullable/firstpos/lastpos/followpos. */
    arbol_calcular_funciones(raiz_aumentada, followpos, cantidad_posiciones);

    printf("\n=== Arbol Aumentado (ER.#) con posiciones ===\n");
    arbol_imprimir(raiz_aumentada, 0);

    imprimir_tabla_followpos(followpos, simbolos_pos, cantidad_posiciones);

    /* 4) Construir AFD directo y mostrarlo. */
    if (!afd_construir(&afd, raiz_aumentada, followpos, simbolos_pos, cantidad_posiciones, posicion_hash)) {
        fprintf(stderr, "Error: no se pudo construir el AFD.\n");
        for (i = 1; i <= cantidad_posiciones; i++) {
            cpos_liberar(&followpos[i]);
        }
        free(followpos);
        free(simbolos_pos);
        arbol_liberar(raiz_aumentada);
        return 1;
    }

    afd_imprimir(&afd);

    /* 5) Modo interactivo de validacion de cadenas. */
    printf("\n=== Modo Interactivo ===\n");
    while (1) {
        int indice_invalido;
        char caracter_invalido;
        int aceptada;

        while (1) {
            printf("\nIngrese una cadena (enter vacio = epsilon): ");
            if (!fgets(linea_entrada, sizeof(linea_entrada), stdin)) {
                printf("\nFin de entrada detectado.\n");
                afd_liberar(&afd);
                for (i = 1; i <= cantidad_posiciones; i++) {
                    cpos_liberar(&followpos[i]);
                }
                free(followpos);
                free(simbolos_pos);
                arbol_liberar(raiz_aumentada);
                return 0;
            }

            quitar_salto_linea(linea_entrada);

            if (!afd_cadena_solo_alfabeto(&afd, linea_entrada, &indice_invalido, &caracter_invalido)) {
                printf(
                    "Error: caracter invalido '%c' en posicion %d. Debe pertenecer al alfabeto de la ER.\n",
                    caracter_invalido,
                    indice_invalido + 1
                );
                continue;
            }
            break;
        }

        aceptada = afd_validar_cadena(&afd, linea_entrada);
        if (aceptada) {
            printf("Resultado: Aceptada\n");
        } else {
            printf("Resultado: No Aceptada\n");
        }

        if (!preguntar_continuar()) {
            break;
        }
    }

    afd_liberar(&afd);
    for (i = 1; i <= cantidad_posiciones; i++) {
        cpos_liberar(&followpos[i]);
    }
    free(followpos);
    free(simbolos_pos);
    arbol_liberar(raiz_aumentada);

    printf("\nPrograma finalizado.\n");
    return 0;
}
