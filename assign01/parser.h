#ifndef PARSER_H
#define PARSER_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

struct Lexer;
struct Parser;

// Node tags for nonterminal nodes (these need to not conflict with
// TokenKind values, so the first enumeration member should have an
// integer value that is large, e.g., 1000)
enum Nonterminal {
  NODE_EXPR_LIST = 1000,
  NODE_UNIT
};

struct Parser *parser_create(struct Lexer *lexer_to_adopt);
void parser_destroy(struct Parser *parser);

struct Node *parser_parse(struct Parser *parser);

void parser_print_parse_tree(struct Node *tree);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // PARSER_H
