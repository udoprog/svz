GCC=gcc
TESTV=valgrind

all: parser.tab.c stack.c
	$(GCC) -g parser.tab.c frun.c stack.c supervize.c -o svz

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
