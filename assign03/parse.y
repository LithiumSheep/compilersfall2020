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
%type<node> opt_declarations declarations declaration
%type<node> constdecl constdefn_list constdefn
%type<node> typedecl typedefn_list typedefn
%type<node> vardecl vardefn_list vardefn
%type<node> type named_type array_type record_type
%type<node> opt_instructions instructions instruction
%type<node> expression term factor primary
%type<node> assignstmt ifstmt repeatstmt whilestmt condition writestmt readstmt
%type<node> designator identifier_list
/*
%type<node> expression_list
*/

%%

program
    : TOK_PROGRAM TOK_IDENT TOK_SEMICOLON opt_declarations TOK_BEGIN opt_instructions TOK_END TOK_DOT
        { $$ = g_program = node_build2(AST_PROGRAM, $4, $6); }
    ;

opt_declarations
    : declarations
    | /* epsilon */ { $$ = node_build0(AST_DECLARATIONS);  }
    ;

declarations
    : declarations declaration { $$ = $1, node_add_kid($1, $2); }
    | declaration { $$ = node_build1(AST_DECLARATIONS, $1); }
    ;

declaration
    : constdecl { $$ = $1; }
    | typedecl { $$ = $1; }
    | vardecl { $$ = $1; }
    ;

constdecl
    : constdefn_list { $$ = $1; }
    ;

constdefn_list
    : constdefn_list constdefn { $$ = $1; node_add_kid($1, $2); }
    | constdefn { $$ = node_build1(AST_CONSTANT_DECLARATIONS, $1); }
    ;

constdefn
    :  TOK_CONST TOK_IDENT TOK_EQUALS expression TOK_SEMICOLON { $$ = node_build2(AST_CONSTANT_DEF, $2, $4); }
    ;

typedecl
    : typedefn_list { $$ = $1; }
    ;

typedefn_list
    : typedefn_list typedefn { $$ = $1; node_add_kid($1, $2); }
    | typedefn { $$ = node_build1(AST_TYPE_DECLARATIONS, $1); }
    ;

typedefn
    : TOK_TYPE TOK_IDENT TOK_EQUALS type TOK_SEMICOLON { $$ = node_build2(AST_TYPE_DEF, $2, $4); }
    ;

vardecl
    : TOK_VAR vardefn_list { $$ = $2; }
    ;

vardefn_list
    : vardefn_list vardefn { $$ = $1; node_add_kid($1, $2); }
    | vardefn { $$ = node_build1(AST_VAR_DECLARATIONS, $1); }
    ;

vardefn
    : identifier_list TOK_COLON type TOK_SEMICOLON { $$ = node_build2(AST_VAR_DEF, $1, $3); }
    ;

identifier_list
    : identifier_list TOK_COMMA TOK_IDENT { $$ = $1; node_add_kid($1, $3); }
    | TOK_IDENT { $$ = node_build1(AST_IDENTIFIER_LIST, $1); }
    ;

type
    : named_type { $$ = $1; }
    | array_type { $$ =  $1; }
    | record_type { $$ = $1; }
    ;

named_type
    : TOK_IDENT { $$ = node_build1(AST_NAMED_TYPE, $1); }
    ;

array_type
    : TOK_ARRAY expression TOK_OF type { $$ = node_build2(AST_ARRAY_TYPE, $2, $4); }
    ;

record_type
    : TOK_RECORD vardefn_list TOK_END { $$ = node_build1(AST_RECORD_TYPE, $2); }
    ;

opt_instructions
    : instructions
    | /* epsilon */ { $$ = node_build0(AST_INSTRUCTIONS); }
    ;

instructions
    : instructions instruction { $$ = $1; node_add_kid($1, $2); }
    | instruction { $$ = node_build1(AST_INSTRUCTIONS, $1); }
    ;

instruction
    : assignstmt TOK_SEMICOLON
    | ifstmt TOK_SEMICOLON
    | repeatstmt TOK_SEMICOLON
    | whilestmt TOK_SEMICOLON
    | writestmt TOK_SEMICOLON
    | readstmt TOK_SEMICOLON
    ;

assignstmt
    : designator TOK_ASSIGN expression { $$ = node_build2(AST_ASSIGN, $1, $3); }
    ;

ifstmt
    : TOK_IF condition TOK_THEN opt_instructions TOK_END { $$ = node_build2(AST_IF, $2, $4); }
    | TOK_IF condition TOK_THEN opt_instructions TOK_ELSE opt_instructions TOK_END { $$ = node_build3(AST_IF, $2, $4, $6); }
    ;

repeatstmt
    : TOK_REPEAT opt_instructions TOK_UNTIL condition TOK_END { $$ = node_build2(AST_REPEAT, $2, $4); }
    ;

whilestmt
    : TOK_WHILE condition TOK_DO opt_instructions TOK_END { $$ = node_build2(AST_WHILE, $2, $4); }
    ;

writestmt
    : TOK_WRITE designator { $$ = node_build1(AST_WRITE, $2); }
    ;

readstmt
    : TOK_READ designator { $$ = node_build1(AST_READ, $2); }
    ;

condition
    : expression TOK_EQUALS expression { $$ = node_build2(AST_COMPARE_EQ, $1, $3); }
    | expression TOK_HASH expression { $$ = node_build2(AST_COMPARE_NEQ, $1, $3); }
    | expression TOK_LT expression { $$ = node_build2(AST_COMPARE_LT, $1, $3); }
    | expression TOK_LTE expression { $$ = node_build2(AST_COMPARE_LTE, $1, $3); }
    | expression TOK_GT expression { $$ = node_build2(AST_COMPARE_GT, $1, $3); }
    | expression TOK_GTE expression { $$ = node_build2(AST_COMPARE_GTE, $1, $3); }
    ;

designator
    : TOK_IDENT { $$ = node_build1(AST_VAR_REF, $1); }
    | designator TOK_LBRACKET expression TOK_RBRACKET { $$ = node_build2(AST_ARRAY_ELEMENT_REF, $1, $3); }
    | designator TOK_DOT TOK_IDENT { $$ = node_build2(AST_FIELD_REF, $1, $3); }
    ;

/*
expression_list
    : expression_list TOK_COMMA expression { $$ = $1; node_add_kid($1, $3); }
    | expression { $$ = node_build1(NODE_expression_list, $1); }
    ;
*/

expression
    : expression TOK_PLUS term { $$ = node_build2(AST_ADD, $1, $3); }
    | expression TOK_MINUS term { $$ = node_build2(AST_SUBTRACT, $1, $3); }
    | term
    ;

term
    : term TOK_TIMES factor { $$ = node_build2(AST_MULTIPLY, $1, $3); }
    | term TOK_DIV factor { $$ = node_build2(AST_DIVIDE, $1, $3); }
    | term TOK_MOD factor { $$ = node_build2(AST_MODULUS, $1, $3); }
    | factor
    ;

factor
    : primary { $$ = $1; }
    ;

primary
    : TOK_INT_LITERAL { $$ = $1; }
    | designator { $$ = $1; }
    | TOK_LPAREN expression TOK_RPAREN { $$ = $2; }
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
