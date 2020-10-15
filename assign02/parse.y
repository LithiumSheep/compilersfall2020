%{
#include <stdio.h>
#include <stdarg.h>
#include "util.h"
#include "node.h"
#include "grammar_symbols.h"

int yylex(void);
void yyerror(const char *fmt, ...);

// global variable to point to the root of the parse tree
struct Node *g_translation_unit;
%}

%union {
    struct Node *node;
}

/* TODO: define terminal and nonterminal symbols */

%token<node> TOK_IDENTIFIER
%token<node> TOK_INT_LITERAL

// keywords
%token<node> TOK_KW_VAR
%token<node> TOK_KW_FUNC
%token<node> TOK_KW_IF
%token<node> TOK_KW_ELSE
%token<node> TOK_KW_WHILE

// operators
%token<node> TOK_PLUS TOK_MINUS
%token<node> TOK_TIMES TOK_DIVIDE
%token<node> TOK_ASSIGN
%token<node> TOK_EQ TOK_NE
%token<node> TOK_LT TOK_LE TOK_GT TOK_GE
%token<node> TOK_AND TOK_OR

// grouping
%token<node> TOK_LPAREN TOK_RPAREN TOK_LBRACE TOK_RBRACE
%token<node> TOK_COMMA
%token<node> TOK_SEMICOLON

%type<node> translation_unit
//%type<node> definition_list
//%type<node> definition
// TODO: statements
%type<node> statement
%type<node> expression
//%type<node> assignment_expression
//%type<node> logical_or_expression
//%type<node> logical_and_expression
//%type<node> additive_expression
//%type<node> multiplicative_expression
//%type<node> unary_expression
//%type<node> primary_expression
// TODO: functions, arguments


%right TOK_TIMES TOK_DIVIDE
%left TOK_PLUS TOK_MINUS
%left TOK_EQ TOK_NE TOK_LT TOK_LE TOK_GT TOK_GE
%left TOK_AND TOK_OR

// TODO: precendence of && over ||?

%%

/* TODO: add actual grammar rules */
translation_unit
	: statement { $$ = g_translation_unit = node_build0(NODE_translation_unit); }
	;

statement
    : expression TOK_SEMICOLON { $$ = node_build2(NODE_statement, $1, $2); }
    ;

expression
    : TOK_INT_LITERAL TOK_PLUS TOK_INT_LITERAL { $$ = node_build3(NODE_expression, $1, $2, $3); }
    ;


%%

void yyerror(const char *fmt, ...) {
  extern char *g_srcfile;
  extern int yylineno, g_col;

  va_list args;

  va_start(args, fmt);
  int error_col = 1; // TODO: determine column number
  fprintf(stderr, "%s:%d:%d: Error: ", g_srcfile, yylineno, error_col);
  verr_fatal(fmt, args);
  va_end(args);
}
