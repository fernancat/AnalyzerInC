#ifndef PARSER_ER_H
#define PARSER_ER_H

#include <stddef.h>

#include "syntax_tree.h"

/* Construye el arbol de sintaxis de una ER. Devuelve NULL si hay error. */
NodoSintaxis *parsear_er(const char *expresion_regular, char *mensaje_error, size_t tam_error);

#endif
