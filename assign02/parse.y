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
%type<node> assignment_expression
%type<node> logical_or_expression
%type<node> logical_and_expression
%type<node> additive_expression
%type<node> multiplicative_expression
%type<node> unary_expression
%type<node> primary_expression
// TODO: functions, arguments


%right TOK_TIMES TOK_DIVIDE
%left TOK_PLUS TOK_MINUS
%left TOK_EQ TOK_NE TOK_LT TOK_LE TOK_GT TOK_GE
%left TOK_AND TOK_OR

// TODO: precendence of && over ||?

%%

/* TODO: add actual grammar rules */
translation_unit
	: statement { $$ = g_translation_unit = node_build1(NODE_translation_unit, $1); }
	;

statement
    : expression TOK_SEMICOLON { $$ = node_build2(NODE_statement, $1, $2); }
    ;

expression
    : assignment_expression { $$ = node_build1(NODE_expression, $1); }
    ;

assignment_expression
    : TOK_IDENTIFIER TOK_ASSIGN additive_expression { $$ = node_build3(NODE_assignment_expression, $1, $2, $3); }
    | logical_or_expression { $$ = node_build1(NODE_assignment_expression, $1); }
    ;

logical_or_expression
    : logical_and_expression TOK_OR logical_and_expression { $$ = node_build3(NODE_logical_or_expression, $1, $2, $3); }
    | logical_and_expression { $$ = node_build1(NODE_logical_or_expression, $1); }
    ;

logical_and_expression
    : additive_expression TOK_AND additive_expression { $$ = node_build3(NODE_logical_and_expression, $1, $2, $3); }
    | additive_expression { $$ = node_build1(NODE_logical_and_expression, $1); }
    ;

additive_expression
    : multiplicative_expression TOK_PLUS multiplicative_expression { $$ = node_build3(NODE_additive_expression, $1, $2, $3); }
    | multiplicative_expression TOK_MINUS multiplicative_expression { $$ = node_build3(NODE_additive_expression, $1, $2, $3); }
    | multiplicative_expression { $$ = node_build1(NODE_additive_expression, $1); }
    ;

multiplicative_expression
    : unary_expression TOK_TIMES unary_expression { $$ = node_build3(NODE_multiplicative_expression, $1, $2, $3); }
    | unary_expression TOK_DIVIDE unary_expression { $$ = node_build3(NODE_multiplicative_expression, $1, $2, $3); }
    | unary_expression { $$ = node_build1(NODE_multiplicative_expression, $1); }
    ;

unary_expression
    : primary_expression { $$ = node_build1(NODE_unary_expression, $1); }
    ;

primary_expression
    : TOK_IDENTIFIER { $$ = node_build1(NODE_primary_expression, $1); }
    | TOK_INT_LITERAL { $$ = node_build1(NODE_primary_expression, $1); }
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
