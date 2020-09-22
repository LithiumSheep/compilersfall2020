#ifndef TOKEN_H
#define TOKEN_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

enum TokenKind {
  TOK_IDENTIFIER,       // 0
  TOK_INTEGER_LITERAL,  // 1
  TOK_PLUS,             // 2
  TOK_MINUS,            // 3
  TOK_TIMES,            // 4
  TOK_DIVIDE,           // 5
  TOK_POWER,            // 6
  TOK_ASSIGN,           // 7
  TOK_LPAREN,           // 8
  TOK_RPAREN,           // 9
  TOK_SEMICOLON,        // 10
};

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // TOKEN_H
