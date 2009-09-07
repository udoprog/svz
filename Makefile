CC=gcc
CFLAGS=-I./src -g
TESTV=valgrind

SRC=src/parser.tab.c
SRC+=src/proc.c
SRC+=src/lexer.c
SRC+=src/frun.c
SRC+=src/stack.c
SRC+=src/supervize.c
SRC+=src/libs/core.c

OBJ=$(SRC:.c=.o)

.SUFFIXES: .o .c

all: $(OBJ)
	$(CC) -g $(OBJ) -o svz

.c.o:
	$(CC) -c $(CFLAGS) $< -o $@

debug:
	$(GCC) -D_DEBUG=1 -g -I./src src/parser.tab.c src/libs/core.c src/lexer.c src/frun.c src/stack.c src/supervize.c -o svz

src/parser.tab.c: src/parser.y
	bison --verbose --defines=src/parser.tab.h --output=src/parser.tab.c $<

clean:
	$(RM) src/parser.tab.c
	$(RM) src/parser.tab.h
	$(RM) $(SRC:.c=.o)

tests: stack-test frun-test

stack-test:
	$(GCC) -g src/stack.c src/stack.main.c -o $@
	-$(TESTV) -v ./$@
	-./$@
	$(RM) $@

frun-test:
	$(GCC) -g src/frun.c src/frun.main.c -o $@
	-$(TESTV) -v ./$@
	-./$@
	$(RM) $@
