#ifndef TOKEN_H
#define TOKEN_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

enum TokenKind {
  TOK_IDENTIFIER,
  TOK_INTEGER_LITERAL,
  TOK_OP_PLUS,
  TOK_OP_MINUS,
  TOK_OP_TIMES,
  TOK_OP_DIVIDE,
  TOK_OP_POWER,
  TOK_OP_ASSIGN,
  TOK_LPAREN,
  TOK_RPAREN,
  TOK_SEMICOLON,
};

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // TOKEN_H
