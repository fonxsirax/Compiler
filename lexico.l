%{
#include <cstdio>
#include <iostream>
#include "tree.h"
using namespace std;
#define YY_DECL extern "C" int yylex()

#include "gram.tab.h"  // to get the token types that we return

int line = 1;
int startedCommentOnLine;
%}

%option yylineno
%x COMMENT

ID      [a-zA-Z][a-zA-Z0-9]*
NUM        [0-9][0-9]*
%%


"if" {return T_IF;}
"else" {return T_ELSE;}
"int" {return T_INT;}
"return" {return T_RETURN;}
"void" {return T_VOID;}
"while" {return T_WHILE;}
"+" { return T_PLUS;}
"-" { return T_MINUS;}
"*" { return T_MUL;}
"/" { return T_DIV;}
")" { return T_CLOSE;}
"(" { return T_OPEN;}
">" { return T_GREATER;}
"<" { return T_LESSER;}
"<=" { return T_LESEQ;}
">=" { return T_GREEQ;}
"==" { return T_EQUAL;}
"!=" { return T_NOTEQ;}
"=" { return T_ASSIGN;}
";" { return T_SEMI;}
"," { return T_COMMA;}
"[" { return T_OPENCOLCHETE;}
"]" { return T_CLOSECOLCHETE;}
"{" { return T_OPENCHAVE;}
"}" { return T_CLOSECHAVE;}


\n                         {line++;} /* Line counting */

[ \t]+                     {} /* ignore whitespace */

{NUM}+{ID}+                {printf("(%d,%s,\"%s\")\n", line, "ERROR", yytext); return 0;} /*number followed by letter*/

"/*"                    {BEGIN(COMMENT); startedCommentOnLine = line;} /* multiline comment */

<COMMENT>[^*\n]*         /* ignores anything that's not '*' */

<COMMENT>"*"+[^*/\n]*    /* ignores '*' not closing comments */

<COMMENT><<EOF>>        {printf("(%d,%s,\"%s\")\n", startedCommentOnLine, "ERROR", "/*"); return 0;}/* end of file without closing comment */

<COMMENT>\n             {line++;}

<COMMENT>"*/"             BEGIN(0);    /* returns to standard behaviour */


{ID} {
    // printf("(%d,%s,\"%s\")\n", line, "ID", yytext);
    yylval.idval = strdup(yytext);
    return ID;
}
{NUM} {
    // printf("(%d,%s,\"%s\")\n", line, "NUM", yytext);
    yylval.numval = atoi(yytext);
    return NUM;
}
. {
    // printf("(%d,%s,\"%s\")\n", line, "ERROR", yytext); return 0;
}
%%