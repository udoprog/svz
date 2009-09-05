GCC=gcc
TESTV=valgrind

all: src/parser.tab.c
	$(GCC) -g -I./src src/parser.tab.c src/libs/core.c src/lexer.c src/frun.c src/stack.c src/supervize.c -o svz

parser: parser.tab.c

src/parser.tab.c: src/parser.y
	bison --verbose --defines=src/parser.tab.h --output=src/parser.tab.c $<

clean:
	$(RM) src/parser.tab.c
	$(RM) src/parser.tab.h
	$(RM) svz

tests: stack-test frun-test

stack-test:
	$(GCC) -g stack.c stack.main.c -o $@
	-$(TESTV) -v ./$@
	-./$@
	$(RM) $@

frun-test:
	$(GCC) -g frun.c frun.main.c -o $@
	-$(TESTV) -v ./$@
	-./$@
	$(RM) $@
