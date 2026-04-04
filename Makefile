CC := clang
CFLAGS := -std=c11 -Wall -Wextra -O2 -Iinclude

SRC := src/main.c src/lexer.c src/parser.c src/codegen.c src/codegen_asm.c src/executor.c src/runtime.c
TARGET := build/bad

BADFILE ?= examples/demo.bad

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(SRC) include/bad.h include/ast.h
	@mkdir -p build
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

run: $(TARGET)
	@badfile='$(BADFILE)'; \
	badfile=$${badfile#\"}; badfile=$${badfile%\"}; \
	badfile=$${badfile#\'}; badfile=$${badfile%\'}; \
	$(TARGET) "$$badfile"

clean:
	rm -rf build
