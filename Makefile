CC = gcc
CFLAGS = -Wall

all: babyc

lex.yy.c: babyc_lex.l
	lex $<

y.tab.c: babyc_parse.y
	yacc -d $<

babyc: lex.yy.c y.tab.c syntax.c
	$(CC) $(CFLAGS) lex.yy.c y.tab.c -o babyc

.PHONY: clean
clean:
	rm -f babyc y.tab.c y.tab.h lex.yy.c
