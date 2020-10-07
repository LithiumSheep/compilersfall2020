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

%type<node> translation_unit

%%

/* TODO: add actual grammar rules */
translation_unit
	: TOK_IDENTIFIER { g_translation_unit = $$ = node_build1(NODE_translation_unit, $1); }
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
