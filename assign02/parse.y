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
%type<node> definition_list
%type<node> definition
%type<node> statement_or_function
%type<node> statement

// functions
%type<node> function
//%type<node> func_call

// statements
%type<node> expression
%type<node> var_dec_statement
%type<node> if_statement
%type<node> while_statement

// expressions
%type<node> assignment_expression
%type<node> logical_or_expression
%type<node> logical_and_expression
%type<node> relational_expression
%type<node> additive_expression
%type<node> multiplicative_expression
%type<node> unary_expression
%type<node> primary_expression

// list types
%type<node> identifier_list
%type<node> opt_statement_list
%type<node> opt_arg_list
%type<node> statement_list

// AST types
%token<node> AST_PLUS AST_MINUS AST_TIMES AST_DIVIDE
%token<node> AST_AND AST_OR
%token<node> AST_EQ AST_NE AST_LT AST_LE AST_GT AST_GE
%token<node> AST_ASSIGN
%token<node> AST_VAR_DEC
%token<node> AST_IF AST_WHILE

%right ASSIGN
%left PLUS MINUS TIMES DIVIDE
%left EQ NE LT LE GT GE
%left AND OR

%%

translation_unit
	// : definition_list { $$ = g_translation_unit = node_build1(NODE_translation_unit, $1); }
	: definition_list { $$ = g_translation_unit = $1; }
	;

definition_list
    : /* epsilon */ { $$ = node_build0(NODE_definition_list); }
    | definition_list definition { $$ = $1;  node_add_kid($1, $2); }
    ;

definition
	: statement_or_function
	;

statement_or_function
    : statement /*{ $$ = node_build1(NODE_statement_or_function, $1); }*/
    | function /*{ $$ = node_build1(NODE_statement_or_function, $1); }*/
    ;

function
    : KW_FUNC IDENTIFIER LPAREN opt_arg_list RPAREN LBRACE opt_statement_list RBRACE { $$ = node_build3(NODE_function, $2, $4, $7); }
    ;

statement
    : expression SEMICOLON /*{ $$ = node_build2(NODE_statement, $1, $2); }*/
    | var_dec_statement SEMICOLON /*{ $$ = node_build1(NODE_statement, $1); }*/
    | if_statement /*{ $$ = node_build1(NODE_statement, $1); }*/
    | while_statement { $$ = node_build1(NODE_statement, $1); }
    ;

var_dec_statement
    : KW_VAR identifier_list { $$ = $2 }
    ;

identifier_list
    : /* epsilon */ { $$ = node_build0(NODE_AST_VAR_DEC); }
    | IDENTIFIER { $$ = node_build1(NODE_AST_VAR_DEC, $1); }
    | IDENTIFIER COMMA identifier_list { $$ = $3; node_add_kid($3, $1); }
    ;

if_statement
    : KW_IF LPAREN expression RPAREN LBRACE opt_statement_list RBRACE { $$ = node_build2(NODE_AST_IF, $3, $6); }
    | KW_IF LPAREN expression RPAREN LBRACE opt_statement_list RBRACE KW_ELSE LBRACE opt_statement_list RBRACE  { $$ = node_build3(NODE_AST_IF, $3, $6, $10); }
    ;

while_statement
    : KW_WHILE LPAREN expression RPAREN LBRACE opt_statement_list RBRACE { $$ = node_build2(NODE_AST_WHILE, $3, $6); }
    ;

opt_statement_list
    : statement_list { $$ = $1; }
    ;

statement_list
    : statement { $$ = node_build1(NODE_opt_statement_list, $1); }
    | statement_list statement { $$ = $1;  node_add_kid($1, $2); }
    | /* epsilon */ { $$ = node_build0(NODE_opt_statement_list); }
    ;

// TODO: opt_arg_list might need to use new "arg_list" non-terminal
opt_arg_list
    : identifier_list { $$ = node_build1(NODE_opt_arg_list, $1); }
    | /* epsilon */ { $$ = node_build0(NODE_opt_arg_list); }
    ;

expression
    : assignment_expression /*{ $$ = node_build1(NODE_expression, $1); }*/
    // TODO: function call expression
    // TODO: handle parentesized subexpression
    ;

// TODO might need to recursively go right on assignment?
assignment_expression
    : IDENTIFIER ASSIGN additive_expression { $$ = node_build2(NODE_AST_ASSIGN, $1, $3); }
    | logical_or_expression /*{ $$ = node_build1(NODE_assignment_expression, $1); }*/
    ;

logical_or_expression
    : logical_or_expression OR logical_and_expression { $$ = node_build2(NODE_AST_OR, $1, $3); }
    | logical_and_expression /*{ $$ = node_build1(NODE_logical_or_expression, $1); }*/
    ;

logical_and_expression
    : logical_and_expression AND relational_expression { $$ = node_build2(NODE_AST_AND, $1, $3); }
    | relational_expression /*{ $$ = node_build1(NODE_logical_and_expression, $1); }*/
    ;

relational_expression
    : relational_expression EQ additive_expression { $$ = node_build2(NODE_AST_EQ, $1, $3); }
    | relational_expression NE additive_expression { $$ = node_build2(NODE_AST_NE, $1, $3); }
    | relational_expression LT additive_expression { $$ = node_build2(NODE_AST_LT, $1, $3); }
    | relational_expression LE additive_expression { $$ = node_build2(NODE_AST_LE, $1, $3); }
    | relational_expression GT additive_expression { $$ = node_build2(NODE_AST_GT, $1, $3); }
    | relational_expression GE additive_expression { $$ = node_build2(NODE_AST_GE, $1, $3); }
    | additive_expression
    ;

additive_expression
    : additive_expression PLUS multiplicative_expression { $$ = node_build2(NODE_AST_PLUS, $1, $3); }
    | additive_expression MINUS multiplicative_expression { $$ = node_build2(NODE_AST_MINUS, $1, $3); }
    | multiplicative_expression /*{ $$ = node_build1(NODE_additive_expression, $1); }*/
    ;

multiplicative_expression
    : multiplicative_expression TIMES unary_expression { $$ = node_build2(NODE_AST_TIMES, $1, $3); }
    | multiplicative_expression DIVIDE unary_expression { $$ = node_build2(NODE_AST_DIVIDE, $1, $3); }
    | unary_expression /*{ $$ = node_build1(NODE_multiplicative_expression, $1); }*/
    ;

// TODO: handle unary "-"
unary_expression
    : primary_expression /*{ $$ = node_build1(NODE_unary_expression, $1); }*/
    ;

primary_expression
    : IDENTIFIER { $$ = $1; }
    | INT_LITERAL { $$ = $1; }
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
