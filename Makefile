all: centric

lex.yy.c: minimal_c.l
	lex minimal_c.l

centric: lex.yy.c
	gcc -Wall lex.yy.c -o centric -lfl
