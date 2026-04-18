#include "regex_parser.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

/*
 * Analizador recursivo descendente para ER con:
 * - Union: |
 * - Concatenacion implicita
 * - Kleene: *
 * - Parentesis: ( )
 */
typedef struct {
    const char *entrada;
    size_t indice;
    char *mensaje_error;
    size_t tam_error;
    int hay_error;
} Analizador;

static void analizador_fijar_error(Analizador *analizador, const char *mensaje) {
    if (!analizador || analizador->hay_error) {
        return;
    }

    analizador->hay_error = 1;
    if (analizador->mensaje_error && analizador->tam_error > 0) {
        snprintf(
            analizador->mensaje_error,
            analizador->tam_error,
            "Error en posicion %zu: %s",
            analizador->indice,
            mensaje
        );
    }
}

static void analizador_saltar_espacios(Analizador *analizador) {
    while (
        analizador->entrada[analizador->indice] != '\0' &&
        isspace((unsigned char)analizador->entrada[analizador->indice])
    ) {
        analizador->indice++;
    }
}

static char analizador_ver(Analizador *analizador) {
    analizador_saltar_espacios(analizador);
    return analizador->entrada[analizador->indice];
}

static char analizador_consumir(Analizador *analizador) {
    char c = analizador_ver(analizador);
    if (c != '\0') {
        analizador->indice++;
    }
    return c;
}

static int es_simbolo_literal(char c) {
    if (c == '\0') {
        return 0;
    }
    if (c == '|' || c == '*' || c == '(' || c == ')' || c == '#') {
        return 0;
    }
    if (isspace((unsigned char)c)) {
        return 0;
    }
    return isprint((unsigned char)c) != 0;
}

static int puede_iniciar_factor(char c) {
    return c == '(' || es_simbolo_literal(c);
}

static NodoSintaxis *parsear_expresion(Analizador *analizador);

/* primary -> '(' expresion ')' | literal */
static NodoSintaxis *parsear_primario(Analizador *analizador) {
    char c;
    NodoSintaxis *nodo;

    if (analizador->hay_error) {
        return NULL;
    }

    c = analizador_ver(analizador);

    if (c == '(') {
        analizador_consumir(analizador);
        nodo = parsear_expresion(analizador);
        if (analizador->hay_error) {
            arbol_liberar(nodo);
            return NULL;
        }
        if (analizador_ver(analizador) != ')') {
            analizador_fijar_error(analizador, "faltaba ')' para cerrar parentesis");
            arbol_liberar(nodo);
            return NULL;
        }
        analizador_consumir(analizador);
        return nodo;
    }

    if (es_simbolo_literal(c)) {
        analizador_consumir(analizador);
        nodo = nodo_crear_literal(c);
        if (!nodo) {
            analizador_fijar_error(analizador, "sin memoria para crear nodo literal");
        }
        return nodo;
    }

    analizador_fijar_error(analizador, "se esperaba un literal o '('");
    return NULL;
}

/* factor -> primary ('*')* */
static NodoSintaxis *parsear_factor(Analizador *analizador) {
    NodoSintaxis *nodo;

    nodo = parsear_primario(analizador);
    if (!nodo || analizador->hay_error) {
        return nodo;
    }

    while (analizador_ver(analizador) == '*') {
        NodoSintaxis *nodo_kleene;
        analizador_consumir(analizador);

        nodo_kleene = nodo_crear_unario(NODO_KLEENE, nodo);
        if (!nodo_kleene) {
            arbol_liberar(nodo);
            analizador_fijar_error(analizador, "sin memoria para crear nodo kleene");
            return NULL;
        }
        nodo = nodo_kleene;
    }

    return nodo;
}

/* term -> factor factor* (concatenacion implicita) */
static NodoSintaxis *parsear_termino(Analizador *analizador) {
    NodoSintaxis *izq;

    if (!puede_iniciar_factor(analizador_ver(analizador))) {
        analizador_fijar_error(analizador, "termino vacio no permitido");
        return NULL;
    }

    izq = parsear_factor(analizador);
    if (!izq || analizador->hay_error) {
        return izq;
    }

    while (puede_iniciar_factor(analizador_ver(analizador))) {
        NodoSintaxis *der = parsear_factor(analizador);
        NodoSintaxis *nodo_concat;
        if (!der || analizador->hay_error) {
            arbol_liberar(izq);
            return NULL;
        }

        nodo_concat = nodo_crear_binario(NODO_CONCAT, izq, der);
        if (!nodo_concat) {
            arbol_liberar(izq);
            arbol_liberar(der);
            analizador_fijar_error(analizador, "sin memoria para crear nodo concatenacion");
            return NULL;
        }
        izq = nodo_concat;
    }

    return izq;
}

/* expresion -> termino ('|' termino)* */
static NodoSintaxis *parsear_expresion(Analizador *analizador) {
    NodoSintaxis *izq = parsear_termino(analizador);

    if (!izq || analizador->hay_error) {
        return izq;
    }

    while (analizador_ver(analizador) == '|') {
        NodoSintaxis *der;
        NodoSintaxis *nodo_union;

        analizador_consumir(analizador);

        der = parsear_termino(analizador);
        if (!der || analizador->hay_error) {
            arbol_liberar(izq);
            return NULL;
        }

        nodo_union = nodo_crear_binario(NODO_UNION, izq, der);
        if (!nodo_union) {
            arbol_liberar(izq);
            arbol_liberar(der);
            analizador_fijar_error(analizador, "sin memoria para crear nodo union");
            return NULL;
        }

        izq = nodo_union;
    }

    return izq;
}

NodoSintaxis *parsear_er(const char *expresion_regular, char *mensaje_error, size_t tam_error) {
    Analizador analizador;
    NodoSintaxis *arbol;

    if (!expresion_regular || expresion_regular[0] == '\0') {
        if (mensaje_error && tam_error > 0) {
            snprintf(mensaje_error, tam_error, "La expresion regular no puede estar vacia");
        }
        return NULL;
    }

    analizador.entrada = expresion_regular;
    analizador.indice = 0;
    analizador.mensaje_error = mensaje_error;
    analizador.tam_error = tam_error;
    analizador.hay_error = 0;

    if (mensaje_error && tam_error > 0) {
        mensaje_error[0] = '\0';
    }

    arbol = parsear_expresion(&analizador);
    if (!arbol || analizador.hay_error) {
        if (!analizador.hay_error) {
            analizador_fijar_error(&analizador, "no se pudo construir el arbol");
        }
        arbol_liberar(arbol);
        return NULL;
    }

    if (analizador_ver(&analizador) != '\0') {
        analizador_fijar_error(&analizador, "hay simbolos sobrantes al final");
        arbol_liberar(arbol);
        return NULL;
    }

    return arbol;
}
