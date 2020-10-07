%{
#include "util.h"
#include "node.h"
#include "parse.tab.h"

void yyerror(const char *fmt, ...);

// global variable pointing to string containing name of input file
char *g_srcfile;
%}

%option yylineno
%option noyywrap

%%

[A-Za-z][A-Za-z0-9]*         { return create_token(TOK_IDENTIFIER, yytext); }

.                            { yyerror("Unknown character: %c\n", yytext[0]); }

%%

void lexer_set_source_file(const char *filename) {
  g_srcfile = xstrdup(filename);
}

int create_token(int tag, const char *lexeme) {
  yylval.node = node_alloc_str_copy(tag, yytext);
  // TODO: set source info
  return tag;
}
