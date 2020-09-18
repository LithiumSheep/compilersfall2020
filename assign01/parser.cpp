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
  struct Lexer *m_lexer;
  struct Node *m_next;

public:
  Parser(Lexer *lexer_to_adopt);
  ~Parser();

  struct Node *parse();

private:
  // Parse functions for nonterminal grammar symbols
  struct Node *parse_U();
  struct Node *parse_A();
  struct Node *parse_E();
  struct Node *parse_T();
  struct Node *parse_F();

  // Consume a specific token, wrapping it in a Node
  struct Node *expect(enum TokenKind tok_kind);

  // Report an error at current lexer position
  void error_at_current_pos(const std::string &msg);
};

Parser::Parser(Lexer *lexer_to_adopt) : m_lexer(lexer_to_adopt), m_next(nullptr) {
}

Parser::~Parser() {
}

struct Node *Parser::parse() {
  // U is the start symbol
  return parse_U();
}

struct Node *Parser::parse_U() {
  struct Node *u = node_build0(NODE_UNIT);

  // U -> ^ E ;
  // U -> ^ E ; U
  node_add_kid(u, parse_E());
  node_add_kid(u, expect(TOK_SEMICOLON));

  // U -> E ; ^
  // U -> E ; ^ U
  if (lexer_peek(m_lexer)) {
    // there is more input, then the sequence of expressions continues
    node_add_kid(u, parse_U());
  }

  return u;
}

struct Node *Parser::parse_A() {
  struct Node *next_terminal = lexer_next(m_lexer);
  if (!next_terminal) {
    error_at_current_pos("Parser error (missing expression)");
  }

  struct Node *e = node_build0(NODE_ASSIGN);

  int tag = node_get_tag(next_terminal);

  if (tag == TOK_ASSIGN) {

  }

  return e;
}

// parsing "+" and "-"
struct Node *Parser::parse_E() {

  struct Node *e = node_build0(NODE_EXPR);
  node_add_kid(e, parse_T());

  // read the next terminal symbol
  struct Node *next_terminal = lexer_peek(m_lexer);
  if (!next_terminal) {
    error_at_current_pos("Parser error (missing expression)");
  }

  int tag = node_get_tag(next_terminal);
  
  if (tag == TOK_PLUS || tag == TOK_MINUS) {
    // E -> T + E'
    // E -> T - E'
    node_add_kid(e, lexer_next(m_lexer));
    node_add_kid(e, parse_T());
  }

  return e;
}

struct Node *Parser::parse_T() {

  // TODO: delegate to F
  struct Node *t = node_build0(NODE_TERM);
  node_add_kid(t, parse_F());

  struct Node *next_terminal = lexer_peek(m_lexer);
  if (!next_terminal) {
    error_at_current_pos("Parser error (missing expression)");
  }

  int tag = node_get_tag(next_terminal);

  if (tag == TOK_TIMES || tag == TOK_DIVIDE) {
    node_add_kid(t, lexer_next(m_lexer));
    node_add_kid(t, parse_F());
  }

  return t;
}

struct Node *Parser::parse_F() {

  struct Node *f = node_build0(NODE_FACTOR);

  struct Node *next_terminal = lexer_next(m_lexer);
  if (!next_terminal) {
    error_at_current_pos("Parser error (missing expression)");
  }

  int tag = node_get_tag(next_terminal);

  if (tag == TOK_INTEGER_LITERAL) {   // TODO: handle identifier
    node_add_kid(f, next_terminal);
  } else if (tag == TOK_POWER) {

  } else if (tag == TOK_LPAREN) {
    node_add_kid(f, parse_E());
    expect(TOK_RPAREN);
  } else {
    // TODO: spit error, unexpected TOKEN
  }

  return f;
}

struct Node *Parser::expect(enum TokenKind tok_kind) {
  struct Node *next_terminal = lexer_next(m_lexer);
  if (node_get_tag(next_terminal) != tok_kind) {
    std::string errmsg = cpputil::format("Unexpected token '%s'", node_get_str(next_terminal));
    error_on_node(next_terminal, errmsg.c_str());
  }
  return next_terminal;
}

const char *minicalc_stringify_node_tag(int tag) {
  switch (tag) {
    case TOK_IDENTIFIER:
      return "IDENTIFIER";
  case TOK_INTEGER_LITERAL:
    return "INTEGER_LITERAL";
  case TOK_PLUS:
    return "PLUS";
  case TOK_MINUS:
    return "MINUS";
  case TOK_TIMES:
    return "TIMES";
  case TOK_DIVIDE:
    return "DIVIDE";
  case TOK_ASSIGN:
    return "ASSIGN";
  case TOK_LPAREN:
    return "LEFT_PARENTHESES";
  case TOK_RPAREN:
    return "RIGHT_PARENTHESES";
  case TOK_SEMICOLON:
    return "SEMICOLON";

  case NODE_EXPR:
    return "E";
  case NODE_TERM:
    return "T";
  case NODE_UNIT:
    return "U";
  case NODE_FACTOR:
    return "F";

  default:
    err_fatal("Unknown node tag %d\n", tag);
    return nullptr;
  }
}

void Parser::error_at_current_pos(const std::string &msg) {
  struct SourceInfo current_pos = lexer_get_current_pos(m_lexer);
  error_at_pos(current_pos, msg.c_str());
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
