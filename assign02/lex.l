%{
#include "util.h"
#include "node.h"
#include "parse.tab.h"

void yyerror(const char *fmt, ...);
void lexer_set_source_file(const char *filename);
int create_token(int tag, const char *lexeme);

// global variable pointing to string containing name of input file
char *g_srcfile;
int g_col;
// TODO: add to g_col on tokens and whitespace, reset to 0 on newline characters
%}

%option yylineno
%option noyywrap

%%

"var"                        { return create_token(KW_VAR, yytext); }
"function"                   { return create_token(KW_FUNC, yytext); }
"if"                         { return create_token(KW_IF, yytext); }
"else"                       { return create_token(KW_ELSE, yytext); }
"while"                      { return create_token(KW_WHILE, yytext); }

[ \t\n]+                     { /* do nothing */ }
[A-Za-z][A-Za-z0-9]*         { return create_token(IDENTIFIER, yytext); }
[0-9]+                       { return create_token(INT_LITERAL, yytext); }
"+"                          { return create_token(PLUS, yytext); }
"-"                          { return create_token(MINUS, yytext); }
"*"                          { return create_token(TIMES, yytext); }
"/"                          { return create_token(DIVIDE, yytext); }
"=="                         { return create_token(EQ, yytext); }
"!="                         { return create_token(NE, yytext); }
"<"                          { return create_token(LT, yytext); }
">"                          { return create_token(GT, yytext); }
"<="                         { return create_token(LE, yytext); }
">="                         { return create_token(GE, yytext); }
"&&"                         { return create_token(AND, yytext); }
"||"                         { return create_token(OR, yytext); }
"="                          { return create_token(ASSIGN, yytext); }
"("                          { return create_token(LPAREN, yytext); }
")"                          { return create_token(RPAREN, yytext); }
"{"                          { return create_token(LBRACE, yytext); }
"}"                          { return create_token(RBRACE, yytext); }
","                          { return create_token(COMMA, yytext); }
";"                          { return create_token(SEMICOLON, yytext); }
.                            { yyerror("Unknown character: %c\n", yytext[0]); }

%%

void lexer_set_source_file(const char *filename) {
  g_srcfile = xstrdup(filename);
}

int create_token(int kind, const char *lexeme) {
  struct Node *n = node_alloc_str_copy(kind, lexeme);
  // TODO: set source info
  yylval.node = n;
  return kind;
}
