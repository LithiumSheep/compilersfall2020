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

%type<node> statement_or_function

%type<node> statement

// TODO: function def, call
//%type<node> func_def
//%type<node> func_call

%type<node> expression
%type<node> var_dec_statement
%type<node> if_statement
//%type<node> if_else_statement
//%type<node> while_statement

%type<node> assignment_expression
%type<node> logical_or_expression
%type<node> logical_and_expression
%type<node> additive_expression
%type<node> multiplicative_expression
%type<node> unary_expression
%type<node> primary_expression

%type<node> identifier_list
%type<node> opt_statement_list
%type<node> statement_list

%right TIMES DIVIDE
%left PLUS MINUS
%left EQ NE LT LE GT GE
%left AND OR

// TODO: precendence of && over ||?

%%

/* TODO: add actual grammar rules */
translation_unit
	: statement_or_function { $$ = g_translation_unit = node_build1(NODE_translation_unit, $1); }
	;

// TODO: do I need definition list?

statement_or_function
    : statement { $$ = node_build1(NODE_statement_or_function, $1); }    ;

statement
    : expression SEMICOLON { $$ = node_build2(NODE_statement, $1, $2); }
    | if_statement { $$ = node_build1(NODE_statement, $1); }
    | var_dec_statement SEMICOLON { $$ = node_build1(NODE_statement, $1); }
    ;

var_dec_statement
    : KW_VAR identifier_list { $$ = node_build1(NODE_var_dec_statement, $2); }
    ;

if_statement
    : KW_IF RPAREN expression LPAREN RBRACE opt_statement_list LBRACE { $$ = node_build2(NODE_if_statement, $3, $6); }
    ;

opt_statement_list
    : statement_list { $$ = node_build1(NODE_opt_statement_list, $1); }
    | /* epsilon */ { $$ = node_build0(NODE_opt_statement_list); }
    ;

statement_list
    : statement { $$ = node_build1(NODE_statement_list, $1); }
    | statement statement_list { $$ = node_build2(NODE_statement_list, $1, $2); }
    ;

identifier_list
    : IDENTIFIER { $$ = node_build1(NODE_identifier_list, $1); }
    | IDENTIFIER COMMA identifier_list { $$ = node_build3(NODE_identifier_list, $1, $2, $3); }
    ;

expression
    : assignment_expression { $$ = node_build1(NODE_expression, $1); }
    // TODO: function call expression
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

// TODO: handle unary "-"
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
