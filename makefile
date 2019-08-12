all:
	bison -d gram.y
	flex lexico.l
	g++ -std=c++11 semantic.cpp tree.cpp gram.tab.c -g lex.yy.c -lfl -o compilador 