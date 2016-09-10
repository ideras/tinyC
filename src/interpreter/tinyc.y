%{
#include <cstdio>
#include <string>
#include "ast.h"

using namespace std;

extern int line;

int yylex();

void yyerror(const char *str)
{
    printf("Line %d: %s\n", line, str);
}

#define YYERROR_VERBOSE 1

extern Statement *input;
extern DeclarationList *decl_list;

%}

%union {
    char *id_t;
    int  num_t;
    double double_t;
    Statement *statement_t;
    Expr *expr_t;
    StringList *stringlist_t;
    DataType datatype_t;
    DeclarationList *declarationlist_t;
    Declaration *declaration_t;
}

%token<num_t> INT_NUM
%token<double_t> REAL_NUM
%token<id_t> ID
%token KW_PRINT KW_IF KW_ELSE KW_WHILE KW_FOR KW_DOUBLE KW_INT
%token OP_LT OP_GT OP_LTE OP_GTE OP_NE OP_EQ OP_INC OP_DEC
%type<expr_t> expra expr term factor 
%type<statement_t> list_st st opt_else block assignment opt_assignment

%type<stringlist_t> id_list
%type<datatype_t> type;
%type<declarationlist_t> opt_declarations declarations
%type<declaration_t> declaration

%%

input: opt_declarations list_st { decl_list = $1; input = $2; }
;

opt_declarations: declarations  { $$ = $1; }
                |               { $$ = NULL; }
;

declarations: declarations declaration {
                $$ = $1;
                $$->push_back($2);
              }
            | declaration{ 
                    $$ = new DeclarationList;
                    $$->push_back($1);
              }
;

declaration: type id_list ';' { $$ = new Declaration($1, *$2); delete $2; }
;

id_list: id_list ',' ID {
            string id = $3;
                          
            free($3);
            
            $$ = $1;
            $$->push_back(id);
          }
        | ID {
                string id = $1;
                          
                free($1);
                
                $$ = new StringList;
                $$->push_back(id);
             }
;

type: KW_INT       { $$ = DT_Int; }
    | KW_DOUBLE    { $$ = DT_Double; }
;

list_st: list_st st { 
                        if ($1->getKind() == BLOCK_STATEMENT) {
                            BlockStatement *block = (BlockStatement*)$1;
                            
                            block->stList.push_back($2);
                            
                            $$ = block;
                        } else {
                            list<Statement *> l;
                            
                            l.push_back($1);
                            l.push_back($2);
                            
                            $$ = new BlockStatement(l);
                        }
                    }
                    
        | st        { $$ = $1; }
;

st: KW_PRINT expr ';'    { $$ = new PrintStatement($2); }
    | assignment ';'   { $$ = $1; }
    | KW_IF '(' expr ')' block opt_else    { $$ = new IfStatement($3, $5, $6); }
    | KW_WHILE '(' expr ')' block { $$ = new WhileStatement($3, $5); }
    | KW_FOR '(' opt_assignment ';' expr ';' opt_assignment ')' block
      { $$ = new ForStatement ($3, $5, $7, $9); }
    | expr ';' { $$ = new ExprStatement($1); }
;

opt_assignment: assignment { $$ = $1; }
              |          { $$ = NULL; }
;

assignment: ID '=' expr { 
                          string id = $1;
                          
                          free($1);
                          
                          $$ = new AssignStatement(id, $3);
                        }
;

opt_else: KW_ELSE block    { $$ = $2; }
        |               { $$ = 0; }
;

block: '{' list_st '}'  { $$ = $2; }
;

expr: expr OP_LT expra  { $$ = new LTExpr($1, $3); }
    | expr OP_GT expra  { $$ = new GTExpr($1, $3); }
    | expr OP_LTE expra { $$ = new LTEExpr($1, $3); }
    | expr OP_GTE expra { $$ = new GTEExpr($1, $3); }
    | expr OP_NE expra  { $$ = new NEExpr($1, $3); }
    | expr OP_EQ expra    { $$ = new EQExpr($1, $3);; }
    | expra             { $$ = $1; }
;

expra: expra '+' term   { $$ = new AddExpr($1, $3); }
    | expra '-' term    { $$ = new SubExpr($1, $3); }
    | term              { $$ = $1; }
;

term: term '*' factor   { $$ = new MultExpr($1, $3); }
    | term '/' factor   { $$ = new DivExpr($1, $3); }
    | factor            { $$ = $1; }
;

factor: '(' expr ')'    { $$ = $2; }
        | INT_NUM       { $$ = new NumExpr($1); }
        | REAL_NUM      { $$ = new NumExpr($1); }
        | ID            { 
                            string id = $1;
                            
                            free($1);
                            $$ = new IdExpr(id);
                        }
        | ID OP_INC     {
                            string id = $1;
                            
                            free($1);
                            $$ = new IncExpr(id);
                        }
        | ID OP_DEC     {   
                            string id = $1;
                            
                            free($1);
                            $$ = new DecExpr(id);
                        }
;

%%