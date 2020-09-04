#include <cctype>
#include <string>
#include "cpputil.h"
#include "token.h"
#include "error.h"
#include "lexer.h"

////////////////////////////////////////////////////////////////////////
// Lexer implementation
////////////////////////////////////////////////////////////////////////

struct Lexer {
private:
  // TODO: add member variables

public:
  // TODO: add constructor, destructor, member functions, etc.
};

////////////////////////////////////////////////////////////////////////
// Lexer API functions
////////////////////////////////////////////////////////////////////////

struct Lexer *lexer_create(FILE *in, const char *filename) {
  return new Lexer(in, filename);
}

void lexer_destroy(struct Lexer *lexer) {
  delete lexer;
}

struct Node *lexer_next(struct Lexer *lexer) {
  return lexer->next();
}

struct Node *lexer_peek(struct Lexer *lexer) {
  return lexer->peek();
}

struct SourceInfo lexer_get_current_pos(struct Lexer *lexer) {
  return lexer->get_current_pos();
}
