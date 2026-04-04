# BAD Language Compiler

A small compiler for the BAD language.

It parses `.bad` source files, generates assembly (`build/out.s`), then uses `clang` to assemble/link and run the result.

## What It Does

- Lexes BAD source code into tokens
- Parses tokens into an AST
- Generates assembly (not C fallback)
- Builds executable output with `clang`
- Runs the compiled program

## Project Structure

```
.
├── include/
│   ├── ast.h
│   └── bad.h
├── src/
│   ├── main.c
│   ├── lexer.c
│   ├── parser.c
│   ├── codegen.c
│   ├── codegen_asm.c
│   └── executor.c
├── examples/
│   └── hello.bad
├── build/
│   ├── bad        (compiler executable)
│   ├── out.s      (generated assembly)
│   └── out/out.exe (compiled BAD program)
├── Makefile
└── .vscode/
    ├── tasks.json
    ├── settings.json
    └── extensions.json
```

## Build

From project root:

```bash
make
```

## Run

```bash
make run
```

Run a specific file:

```bash
make run BADFILE=examples/hello.bad
```

`examples/demo.bad` is the default auto-run file (non-interactive).
`examples/hello.bad` contains `read(...)` and waits for user input.

Expected pipeline:

1. Parse your `.bad` file
2. Generate `build/out.s`
3. Assemble/link to executable (`build/out` on Unix, `build/out.exe` on Windows)
4. Run executable

## One-Click Run in VS Code

This project is configured so you do not need to rebuild manually each time.

### Option 1: VS Code Tasks (no extension required)

- Run task `BAD: Run Current File`
- It automatically uses the file currently open in the editor (`${file}`)
- Build happens automatically via `make run ...`

### Option 2: Code Runner Extension

- Recommended extension is already listed in `.vscode/extensions.json`
- Open any `.bad` file and click Run Code
- It auto-runs:

```bash
make run BADFILE="$fullFileName"
```

So edit -> click run -> auto build + run.

## Platform Support

The compiler is assembly-first:

- macOS (Apple Silicon): generates AArch64 assembly for Mach-O/Clang toolchain
- Windows x64: generates x86_64 assembly for Windows calling convention
- Linux x64: generates x86_64 assembly (System V style)

Note: this repo is currently tested in this workspace on macOS.

## BAD Language Syntax

### Statements

Each statement ends with `;` except control-flow blocks (`if`, `while`) that contain statements.

Supported statements:

- Variable declaration: `let name = expr;`
- Assignment: `name = expr;`
- Output: `write(expr);`
- Input: `read(name);`
- If/else:

```bad
if (condition) {
    // statements
} else {
    // statements
}
```

- While loop:

```bad
while (condition) {
    // statements
}
```

### Blocks

A block can be:

- Braced block: `{ ... }`
- Single statement without braces

That means these are both valid:

```bad
if (x > 0) write(x);
```

```bad
if (x > 0) {
    write(x);
}
```

### Expressions

Expression kinds:

- Integer literals: `123`
- String literals: `'hello'`
- Variables: `x`
- Parentheses: `(a + b)`
- Unary minus: `-x`, `-5`
- Binary operators:
  - Arithmetic: `+`, `-`, `*`, `/`, `%`
  - Comparison: `==`, `!=`, `<`, `>`, `<=`, `>=`

Comparison results are numeric (`0` false, non-zero true).

### Operator Precedence

Highest to lowest:

1. Primary/unary: literals, variables, `(expr)`, unary `-`
2. Multiplicative: `*`, `/`, `%`
3. Additive: `+`, `-`
4. Comparison: `==`, `!=`, `<`, `>`, `<=`, `>=`

All binary operators are left-associative in this parser.

## Strings and Escapes

Strings use single quotes:

```bad
write('hello');
```

Supported escapes inside strings:

- `\n` newline
- `\t` tab
- `\'` literal single quote
- `\\` literal backslash

## Comments

Single-line comments are supported:

```bad
// this is a comment
let x = 10;
```

## Keywords and Tokens

Language keywords:

- `write`
- `let`
- `if`
- `else`
- `while`
- `read`

Other lexical items:

- Identifiers: letters/digits/underscore (cannot start with digit)
- Integers: decimal digits only
- Punctuation: `(` `)` `{` `}` `;` `,`

## Example Program

```bad
let a = 10;
let b = 3;

write(a + b);
write(a * b);

if (a > b) {
    write('a is greater');
} else {
    write('b is greater');
}

while (b > 0) {
    write(b);
    b = b - 1;
}
```

## Error Behavior

Common compile-time errors:

- Unexpected token
- Missing expected token (for example `;` or `)`)
- Undefined variable use during code generation/interpreter execution

Errors are printed to stderr and compilation stops.

## Current Limitations

- No functions/procedures
- No arrays/objects
- No floating-point numbers (integers only)
- No multi-line comments (`/* ... */`)
- No standard library beyond `write` and `read`

## Output Artifacts

After a successful run, you should see:

- `build/out.s`: generated assembly
- `build/out` or `build/out.exe`: built executable

## Quick Syntax Cheat Sheet

```bad
// Declare
let x = 42;

// Assign
x = x + 1;

// Read integer into variable
read(x);

// Print int or string
write(x);
write('hello');

// If / else
if (x >= 10) {
    write('big');
} else {
    write('small');
}

// While
while (x > 0) {
    write(x);
    x = x - 1;
}
```
