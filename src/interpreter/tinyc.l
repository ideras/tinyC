%option noyywrap

%{
#include <cstdlib>
#include <cstring>
#include "ast.h"
#include "tokens.h"

int line = 1;

%}

DIGIT [0-9]
LETTER [a-zA-Z]

%%

"=" { return '='; }
"+" { return '+'; }
"-" { return '-'; }
"*" { return '*'; }
"/" { return '/'; }
"(" { return '('; }
")" { return ')'; }
";" { return ';'; }
"{" { return '{'; }
"}" { return '}'; }
"," { return ','; }
"<" { return OP_LT; }
">" { return OP_GT; }
"<=" { return OP_LTE; }
">=" { return OP_GTE; }
"!=" { return OP_NE; }
"==" { return OP_EQ; }
"++" { return OP_INC; }
"--" { return OP_DEC; }
[ \t] /* Nada */
\n  { line++; }
"print" { return KW_PRINT; }
"if"    { return KW_IF; }
"else"  { return KW_ELSE; }
"while" { return KW_WHILE; }
"for"   { return KW_FOR; }
"int"   { return KW_INT; }
"double" { return KW_DOUBLE; }
{DIGIT}+ { yylval.num_t = atoi(yytext); return INT_NUM; }
{DIGIT}+\.{DIGIT}+ {yylval.double_t = atof(yytext); return REAL_NUM; }
{LETTER}({DIGIT}|{LETTER})* { yylval.id_t = strdup(yytext); return ID; }
"//"[^\n]*
.   { printf("Simbolo no valido\n"); }

%%
