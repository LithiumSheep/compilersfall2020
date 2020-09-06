#ifndef TOKEN_H
#define TOKEN_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

enum TokenKind {
  TOK_IDENTIFIER,
  TOK_INTEGER_LITERAL,
  TOK_PLUS,    //precendence 2
  TOK_MINUS,   //precendence 2
  TOK_TIMES,   //precendence 3
  TOK_DIVIDE,  //precendence 3
  TOK_POWER,   //precedence 4
  TOK_ASSIGN,  //precedence 1
  TOK_LPAREN,
  TOK_RPAREN,
  TOK_SEMICOLON,
};

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // TOKEN_H
