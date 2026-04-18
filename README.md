# Validador de Cadenas con ER -> AFD (C)

Proyecto en C que recibe una expresion regular por linea de comandos, construye el arbol de sintaxis, aplica conversion directa ER -> AFD (firstpos/lastpos/followpos) y valida cadenas en modo interactivo.

## Estructura de carpetas

- `include/`
- `include/pos_set.h`: operaciones de conjuntos de posiciones (`firstpos`, `lastpos`, `followpos`, estados del AFD).
- `include/syntax_tree.h`: definicion de nodos del arbol y funciones de analisis del arbol.
- `include/regex_parser.h`: parser de ER y validaciones de entrada.
- `include/dfa.h`: estructura del AFD y funciones de construccion/validacion.
- `src/`
- `src/parser.c`: parser recursivo para `|`, concatenacion implicita, `*` y parentesis.
- `src/syntax_tree.c`: asignacion de posiciones y calculo de `nullable`, `firstpos`, `lastpos`, `followpos`.
- `src/dfa.c`: algoritmo de construccion del AFD a partir de conjuntos de posiciones.
- `src/pos_set.c`: implementacion de operaciones de conjuntos.
- `src/main.c`: flujo principal, visualizacion e interaccion con el usuario.
- `Makefile`: compilacion del proyecto.

## Como compilar

```bash
make
```

Genera el ejecutable `Validador`.

## Como ejecutar

```bash
./Validador "(a|b)*abb"
```

Luego el programa entra a modo interactivo para validar cadenas.

## ER soportada

- Union: `|`
- Cerradura de Kleene: `*`
- Parentesis: `(` `)`
- Concatenacion implicita: por ejemplo, `ab` significa `a.b`
- Literales: caracteres imprimibles excepto operadores `| * ( )` y `#`

## Notas importantes

- Internamente se aumenta la ER con `#` para detectar estados de aceptacion (`ER.#`).
- Si una cadena tiene caracteres fuera del alfabeto de la ER, se marca error y se vuelve a pedir la entrada.
- Se trabaja con manejo de memoria explicito (malloc/free) para cada estructura.
# AnalyzerInC
