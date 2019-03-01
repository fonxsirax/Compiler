%{
#include <stdio.h>
#define ID 	1 //identificador
#define DEC 	2 //literal decimal (inteiro)
#define KEY 	3 //palavra-chave
#define SYM	4 //símbolos léxicos
#define ERROR 	5 //erro - lexema não esperado
#define NEWLINE 6 //nova linha
int yylex();
int yylineno;
char* yytext;
FILE* yyin;
FILE* yyout;
%}
%option noyywrap
%%
	
"//".*						;
[ \t]						;

"let"						return KEY;
"def"						return KEY;
"if"						return KEY;
"else"						return KEY;
"while"						return KEY;
"return"					return KEY;
"break"						return KEY;
"continue"					return KEY;

"("						return SYM;
")"						return SYM;
"{"						return SYM;
"}"						return SYM;
","						return SYM;
";"						return SYM;
"="						return SYM;
"+"						return SYM;
"-"						return SYM;
"*"						return SYM;
"/"						return SYM;
"<"						return SYM;
">"						return SYM;
"<="						return SYM;
">="						return SYM;
"=="						return SYM;
"!="						return SYM;
"&&"						return SYM;
"||"						return SYM;  
"!"						return SYM;

[a-z|A-Z][_|a-z|A-Z|0-9]*				return ID;

[0-9][0-9]*					return DEC;

[\n]						return NEWLINE;
.						return ERROR;

%%

int main(int c, char *argv[]) {
	int token;
	int error = 0;
	char* text;
	yyin = fopen(argv[1], "r");
	token = yylex();
	yyout = fopen(argv[2], "a");

		if (yyin == NULL || yyout == NULL) {
			return 0;
		}
		if (token == 6) {
			yylineno++;
		}
		else {			
			switch (token) {

			case 1:
				text = "ID";
				fprintf(yyout, "%s \"%s\" %d\n", text, yytext, yylineno);
				//printf("ID ");
				break;
			case 2:
				text = "DEC";
				fprintf(yyout, "%s \"%s\" %d\n", text, yytext, yylineno);
				//printf("DEC ");
				break; 
			case 3:
				text = "KEY";
				fprintf(yyout, "%s \"%s\" %d\n", text, yytext, yylineno);
				//printf("KEY ");
				break; 
			case 4:
				text = "SYM";
				fprintf(yyout, "%s \"%s\" %d\n", text, yytext, yylineno);
				//printf("SYM ");
				break; 
			case 5:
				fprintf(yyout, "ERRO: Lexema não esperado ");
				//printf("ERRO: Lexema não esperado ");
				error = 1;
				break; 
		}

		}
		if (error == 1) {
			return 0;
		}
		token = yylex();
	
	fclose(yyout);
	return 0;
}