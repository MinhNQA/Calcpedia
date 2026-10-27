# Calcpedia

A terminal calculator in C++17 with a math handbook built in. No dependencies.

## Build

```bash
make
```

Or:

```bash
cmake -S . -B build
cmake --build build
```

You need a C++17 compiler (`g++`, `clang++`, or MSVC).

## Run

```bash
./calcpedia
./calcpedia -e "2 + 3*4"
./calcpedia --explain "5! + sqrt(16)/2"
```

On Windows the binary is `calcpedia.exe`.

## Calculate

Operator precedence, parentheses, variables, `ans`, and constants `pi`, `e`, `phi`, `tau`.

```
> 2 + 3*4
14
> a = 5
5
> a! + ans
125
> nCr(10, 3)
120
> gcd(84, 30)
6
```

About 35 functions: trig (with inverse and hyperbolic), `exp`/`ln`/`log`/`log10`/`log2`, `sqrt`/`cbrt`/`pow`/`hypot`, rounding, `gamma`/`erf`, `nCr`/`nPr`, `gcd`/`lcm`, `fact`, `deg`/`rad`.

Angle mode:

```
> rad
> sin(pi/2)
1
> deg
> sin(90)
1
```

## Visual

```
> plot sin(x)/x -10 10
> plot sin(x), cos(x) -6.3 6.3
> table sin(x) 0 3.14 0.5
```

Several expressions can be overlaid; each series gets its own marker. Axes are drawn when zero is in range.

## Handbook

```
> help gamma
> help !
> search derivative
> list
> try gamma
```

Each entry has a definition, a formula, an example, and related topics. `try` runs the example in the live calculator.

## Explain

```
> explain 5! + sqrt(16)/2
```

Annotates every symbol or function with its handbook definition, then shows the step-by-step evaluation order.

## Layout

```
src/     parser (lexer + AST), evaluator, plotter, handbook, explain, REPL
Makefile
CMakeLists.txt
README.md
```

## Commands

| Command | Meaning |
|---|---|
| `help [topic]` | handbook lookup |
| `search <word>` | keyword search |
| `list` | catalog |
| `try [topic]` | run an entry's example |
| `explain <expr>` | teach while calculating |
| `plot <expr>[, expr...] [xmin xmax]` | ASCII graph |
| `table <expr> [xmin xmax [step]]` | value table |
| `deg` / `rad` | angle mode |
| `vars` / `clear` / `functions` | environment |
| `quit` | exit |
