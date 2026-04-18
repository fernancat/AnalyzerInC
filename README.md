# Validador de Cadenas: ER -> AFD Directo (C)

Proyecto en C que recibe una expresion regular por linea de comandos, construye el arbol de sintaxis aumentado y genera un AFD por el metodo directo (`nullable`, `firstpos`, `lastpos`, `followpos`), sin usar Thompson.

## Estructura de carpetas

- `include/`
- `include/pos_set.h`: manejo de conjuntos de posiciones (`ConjuntoPos`).
- `include/syntax_tree.h`: nodos del arbol y funciones del algoritmo de sintaxis.
- `include/regex_parser.h`: parser recursivo descendente para la ER.
- `include/dfa.h`: definiciones del AFD y funciones de validacion.
- `src/`
- `src/pos_set.c`: implementacion de operaciones sobre conjuntos.
- `src/syntax_tree.c`: calculo de `anulable`, `primera_pos`, `ultima_pos` y `followpos`.
- `src/parser.c`: parseo de ER con `|`, concatenacion implicita, `*` y parentesis.
- `src/dfa.c`: construccion del AFD directo.
- `src/main.c`: flujo principal y modo interactivo.
- `Makefile`: compilacion del proyecto.

## Compilar

```bash
make
```

Esto genera el ejecutable `Validador`.

## Ejecutar

```bash
./Validador "(a|b)*abb"
```

## Operadores soportados

- Union: `|`
- Cerradura de Kleene: `*`
- Parentesis: `(` `)`
- Concatenacion implicita: por ejemplo, `ab` equivale a `a.b`

## Flujo del programa

1. Parsea la expresion regular.
2. Muestra el arbol de la ER original.
3. Aumenta internamente la ER como `ER.#`.
4. Calcula `followpos`.
5. Construye y muestra el AFD.
6. Entra en modo interactivo para validar cadenas.

## Nota de memoria

El proyecto usa manejo manual de memoria (`calloc`, `realloc`, `free`) y libera estructuras tanto en rutas de exito como de error.
