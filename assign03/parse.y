%{
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "grammar_symbols.h"
#include "util.h"
#include "ast.h"
#include "node.h"

struct Node *g_program;

void yyerror(const char *fmt, ...);

int yylex(void);
%}

%union {
  struct Node *node;
}

%token<node> TOK_IDENT TOK_INT_LITERAL

%token<node> TOK_PROGRAM TOK_BEGIN TOK_END TOK_CONST TOK_TYPE TOK_VAR
%token<node> TOK_ARRAY TOK_OF TOK_RECORD TOK_DIV TOK_MOD TOK_IF
%token<node> TOK_THEN TOK_ELSE TOK_REPEAT TOK_UNTIL TOK_WHILE TOK_DO
%token<node> TOK_READ TOK_WRITE

%token<node> TOK_ASSIGN
%token<node> TOK_SEMICOLON TOK_EQUALS TOK_COLON TOK_PLUS TOK_MINUS TOK_TIMES
%token<node> TOK_HASH TOK_LT TOK_GT TOK_LTE TOK_GTE TOK_LPAREN
%token<node> TOK_RPAREN TOK_LBRACKET TOK_RBRACKET TOK_DOT TOK_COMMA

%type<node> program
/*
%type<node> opt_declarations declarations declaration
%type<node> constdecl constdefn_list constdefn
%type<node> typedecl typedefn_list typedefn
%type<node> vardecl type vardefn_list vardefn
%type<node> expression term factor primary
%type<node> opt_instructions instructions instruction
%type<node> assignstmt ifstmt repeatstmt whilestmt condition writestmt readstmt
%type<node> designator identifier_list expression_list
*/

%%

program
    : TOK_PROGRAM TOK_IDENT TOK_SEMICOLON opt_declarations TOK_BEGIN opt_instructions TOK_END TOK_DOT
    ;

opt_declarations
    : declarations
    | /* epsilon */
    ;

declarations
    : declarations declaration
    | declaration
    ;

declaration
    : vardecl TOK_SEMICOLON
    ;

vardecl
    : TOK_VAR vardefn_list
    ;

vardefn_list
    :
    ;

opt_instructions
    : instructions
    | /* epsilon */
    ;

instructions
    : instructions instruction
    | instruction
    ;

instruction
    :
    ;

%%

void yyerror(const char *fmt, ...) {
  extern char *g_srcfile;
  extern int yylineno, g_col;

  va_list args;

  va_start(args, fmt);
  fprintf(stderr, "%s:%d:%d: Error: ", g_srcfile, yylineno, g_col);
  vfprintf(stderr, fmt, args);
  fprintf(stderr, "\n");
  va_end(args);

  exit(1);
}
