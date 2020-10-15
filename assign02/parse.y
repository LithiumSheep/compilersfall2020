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

%token<node> IDENTIFIER
%token<node> INT_LITERAL

// keywords
%token<node> KW_VAR
%token<node> KW_FUNC
%token<node> KW_IF
%token<node> KW_ELSE
%token<node> KW_WHILE

// operators
%token<node> PLUS MINUS
%token<node> TIMES DIVIDE
%token<node> ASSIGN
%token<node> EQ NE LT LE GT GE
%token<node> AND OR

// grouping
%token<node> LPAREN RPAREN LBRACE RBRACE
%token<node> COMMA
%token<node> SEMICOLON

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


%right TIMES DIVIDE
%left PLUS MINUS
%left EQ NE LT LE GT GE
%left AND OR

// TODO: precendence of && over ||?

%%

/* TODO: add actual grammar rules */
translation_unit
	: statement { $$ = g_translation_unit = node_build1(NODE_translation_unit, $1); }
	;

statement
    : expression SEMICOLON { $$ = node_build2(NODE_statement, $1, $2); }
    ;

expression
    : assignment_expression { $$ = node_build1(NODE_expression, $1); }
    ;

assignment_expression
    : IDENTIFIER ASSIGN additive_expression { $$ = node_build3(NODE_assignment_expression, $1, $2, $3); }
    | logical_or_expression { $$ = node_build1(NODE_assignment_expression, $1); }
    ;

logical_or_expression
    : logical_and_expression OR logical_and_expression { $$ = node_build3(NODE_logical_or_expression, $1, $2, $3); }
    | logical_and_expression { $$ = node_build1(NODE_logical_or_expression, $1); }
    ;

logical_and_expression
    : additive_expression AND additive_expression { $$ = node_build3(NODE_logical_and_expression, $1, $2, $3); }
    | additive_expression { $$ = node_build1(NODE_logical_and_expression, $1); }
    ;

additive_expression
    : multiplicative_expression PLUS multiplicative_expression { $$ = node_build3(NODE_additive_expression, $1, $2, $3); }
    | multiplicative_expression MINUS multiplicative_expression { $$ = node_build3(NODE_additive_expression, $1, $2, $3); }
    | multiplicative_expression { $$ = node_build1(NODE_additive_expression, $1); }
    ;

multiplicative_expression
    : unary_expression TIMES unary_expression { $$ = node_build3(NODE_multiplicative_expression, $1, $2, $3); }
    | unary_expression DIVIDE unary_expression { $$ = node_build3(NODE_multiplicative_expression, $1, $2, $3); }
    | unary_expression { $$ = node_build1(NODE_multiplicative_expression, $1); }
    ;

unary_expression
    : primary_expression { $$ = node_build1(NODE_unary_expression, $1); }
    ;

primary_expression
    : IDENTIFIER { $$ = node_build1(NODE_primary_expression, $1); }
    | INT_LITERAL { $$ = node_build1(NODE_primary_expression, $1); }
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
