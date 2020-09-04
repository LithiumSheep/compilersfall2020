#include <string>
#include <cassert>
#include "util.h"
#include "cpputil.h"
#include "token.h"
#include "lexer.h"
#include "error.h"
#include "treeprint.h"
#include "parser.h"

////////////////////////////////////////////////////////////////////////
// Parser implementation
////////////////////////////////////////////////////////////////////////

struct Parser {
private:
  // TODO: add member variables

public:
  // TODO: add constructor, destructor, member functions, etc.
};

const char *minicalc_stringify_node_tag(int tag) {
  switch (tag) {
  // TODO: add cases for all of your terminal and nonterminal symbols

/*
  case TOK_IDENTIFIER:
    return "IDENTIFIER";

  case NODE_PRIMARY_EXPR:
    return "PRIMARY_EXPR";
*/

  default:
    err_fatal("Unknown node tag %d\n", tag);
    return nullptr;
  }

}

////////////////////////////////////////////////////////////////////////
// Parser API functions
////////////////////////////////////////////////////////////////////////

struct Parser *parser_create(struct Lexer *lexer_to_adopt) {
  return new Parser(lexer_to_adopt);
}

void parser_destroy(struct Parser *parser) {
  delete parser;
}

struct Node *parser_parse(struct Parser *parser) {
  return parser->parse();
}

void parser_print_parse_tree(struct Node *tree) {
  treeprint(tree, minicalc_stringify_node_tag);
}
