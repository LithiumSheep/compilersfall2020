#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <string>
#include "util.h"
#include "node.h"
#include "grammar_symbols.h"
#include "interp.h"

////////////////////////////////////////////////////////////////////////
// Interp class
////////////////////////////////////////////////////////////////////////

struct Interp {
private:
  // TODO: private fields

public:
  Interp(struct Node *t);
  ~Interp();

  struct Value exec();

private:
  // TODO: private member functions
};

Interp::Interp(struct Node *t) {
}

Interp::~Interp() {
}

struct Value Interp::exec() {
  // TODO: implement
  return val_create_ival(0L);
}

////////////////////////////////////////////////////////////////////////
// API functions
////////////////////////////////////////////////////////////////////////

struct Interp *interp_create(struct Node *t) {
  Interp *interp = new Interp(t);
  return interp;
}

void interp_destroy(struct Interp *interp) {
  delete interp;
}

struct Value interp_exec(struct Interp *interp) {
  return interp->exec();
}
