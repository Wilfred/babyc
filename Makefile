CC = gcc
CFLAGS = -Wall -g

all: babyc

lex.yy.c: babyc_lex.l
	lex $<

y.tab.c: babyc_parse.y
	yacc -d $<

stack.o: stack.c
	$(CC) $(CFLAGS) -c $^

assembly.o: assembly.c
	$(CC) $(CFLAGS) -c $^

syntax.o: syntax.c
	$(CC) $(CFLAGS) -c $^

.INTERMEDIATE: y.tab.c lex.yy.c stack.o assembly.o syntax.o

babyc: lex.yy.c y.tab.c syntax.o assembly.o stack.o
	$(CC) $(CFLAGS) $^ -o babyc

.PHONY: clean
clean:
	rm -f babyc y.tab.c y.tab.h lex.yy.c run_tests

run_tests: run_tests.c babyc
	$(CC) $(CFLAGS) $< -o $@

.PHONY: test
test: run_tests
	./run_tests
