CC = gcc
CFLAGS = -Wall

all: babyc

lex.yy.c: babyc.l
	lex babyc.l

y.tab.c: babyc.y
	yacc -d babyc.y

babyc: lex.yy.c y.tab.c syntax.c
	$(CC) $(CFLAGS) lex.yy.c y.tab.c -o babyc

.PHONY: clean
clean:
	rm -f babyc y.tab.c lex.yy.c
