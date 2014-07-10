all: babyc

lex.yy.c: minimal_c.l
	lex minimal_c.l

y.tab.c: minimal_c.y
	yacc -d minimal_c.y

babyc: lex.yy.c y.tab.c
	gcc -Wall lex.yy.c y.tab.c -o babyc
