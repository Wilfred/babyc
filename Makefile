CC = gcc
CFLAGS = -Wall -g

all: babyc

lex.yy.c: babyc_lex.l
	lex $<

y.tab.c: babyc_parse.y
	yacc -d $<

stack.o: stack.c
	$(CC) $(CFLAGS) -c $^

.INTERMEDIATE: y.tab.c lex.yy.c stack.o

babyc: lex.yy.c y.tab.c syntax.c assembly.c stack.o
	$(CC) $(CFLAGS) lex.yy.c y.tab.c stack.o -o babyc

.PHONY: clean
clean:
	rm -f babyc y.tab.c y.tab.h lex.yy.c test

.PHONY: test
test: run_tests.c babyc
	$(CC) $(CFLAGS) $< -o $@
	./test
