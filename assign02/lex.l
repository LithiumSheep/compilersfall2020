%{
#include "util.h"
#include "node.h"
#include "parse.tab.h"

void yyerror(const char *fmt, ...);
void lexer_set_source_file(const char *filename);
int create_token(int tag, const char *lexeme);

// global variable pointing to string containing name of input file
char *g_srcfile;
%}

%option yylineno
%option noyywrap

%%

"var"                        { return create_token(TOK_KW_VAR, yytext); }
"function"                   { return create_token(TOK_KW_FUNC, yytext); }
"if"                         { return create_token(TOK_KW_IF, yytext); }
"else"                       { return create_token(TOK_KW_ELSE, yytext); }
"while"                      { return create_token(TOK_KW_WHILE, yytext); }

[ \t\n\r\v]+                 { /* do nothing */ }
[A-Za-z][A-Za-z0-9]*         { return create_token(TOK_IDENTIFIER, yytext); }
[0-9]+                       { return create_token(TOK_INT_LITERAL, yytext); }
"+"                          { return create_token(TOK_PLUS, yytext); }
"-"                          { return create_token(TOK_MINUS, yytext); }
"*"                          { return create_token(TOK_TIMES, yytext); }
"/"                          { return create_token(TOK_DIVIDE, yytext); }
"=="                         { return create_token(TOK_EQ, yytext); }
"!="                         { return create_token(TOK_NE, yytext); }
"<"                          { return create_token(TOK_LT, yytext); }
">"                          { return create_token(TOK_GT, yytext); }
"<="                         { return create_token(TOK_LE, yytext); }
">="                         { return create_token(TOK_GE, yytext); }
"&&"                         { return create_token(TOK_AND, yytext); }
"||"                         { return create_token(TOK_OR, yytext); }
"="                          { return create_token(TOK_ASSIGN, yytext); }
"("                          { return create_token(TOK_LPAREN, yytext); }
")"                          { return create_token(TOK_RPAREN, yytext); }
"{"                          { return create_token(TOK_LBRACE, yytext); }
"}"                          { return create_token(TOK_RBRACE, yytext); }
"."                          { return create_token(TOK_COMMA, yytext); }
";"                          { return create_token(TOK_SEMICOLON, yytext); }
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
