GCC=gcc

all: parser.tab.c stack.c
	$(GCC) -g parser.tab.c stack.c supervize.c -o svz

parser: parser.tab.c

parser.tab.c: parser.y
	bison --defines=parser.tab.h $<

clean:
	$(RM) parser.tab.c
	$(RM) parser.tab.h
	$(RM) svz

tests: stack-test

stack-test:
	$(GCC) -g stack.c stack.main.c -o $@
	valgrind -v ./$@
	$(RM) $@
