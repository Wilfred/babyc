all: babyc

lex.yy.c: babyc.l
	lex babyc.l

y.tab.c: babyc.y
	yacc -d babyc.y

babyc: lex.yy.c y.tab.c syntax.c
	gcc -Wall lex.yy.c y.tab.c -o babyc
