GCC=gcc
TESTV=valgrind

all: parser.tab.c
	$(GCC) -g -I./src parser.tab.c src/libs/core.c src/lexer.c src/frun.c src/stack.c src/supervize.c -o svz

parser: parser.tab.c

parser.tab.c: parser.y
	bison --verbose --defines=parser.tab.h $<

clean:
	$(RM) parser.tab.c
	$(RM) parser.tab.h
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
