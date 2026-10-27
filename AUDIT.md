# Calcpedia source audit

## Result
- Builds successfully with GCC in C++17 mode using `make`.
- Builds successfully with CMake.
- Built-in self-test passes after correcting two incorrect expected values.
- Tested arithmetic precedence, right-associative exponentiation, unary minus, factorial, scientific notation, combinations/permutations, gcd/lcm, `explain`, `plot`, `table`, and `help gamma`.

## Corrections made in this checked copy
- `2^3!` expected value changed from 512 to 64. Factorial binds before exponentiation, so this is `2^(3!) = 2^6 = 64`.
- `sqrt(16)/2` expected value changed from 8 to 2. The expression is `(sqrt(16))/2 = 4/2 = 2`.

These were mistakes in the self-test expectations, not parser/evaluator failures.

## Notes
- Angle mode defaults to radians; use `deg` in the REPL before expecting `sin(90) = 1`.
- Implicit multiplication such as `2(3+4)` or `3pi` is not supported; write `2*(3+4)` and `3*pi`.
- This is a source-level audit on Linux with GCC 14.2.0. Windows/MSYS2 has not been tested in this environment.
