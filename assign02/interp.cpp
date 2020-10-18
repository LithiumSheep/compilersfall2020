#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <string>
#include <map>
#include "util.h"
#include "node.h"
#include "grammar_symbols.h"
#include "interp.h"

////////////////////////////////////////////////////////////////////////
// Interp class
////////////////////////////////////////////////////////////////////////

struct Interp {
private:
  struct Node *m_tree;
  std::map<std::string, long> m_vars;

public:
  Interp(struct Node *t);
  ~Interp();

  struct Value exec();

private:
  Value eval(struct Node *n);
};

Interp::Interp(struct Node *t) : m_tree(t) {
}

Interp::~Interp() {
}

struct Value Interp::exec() {
    struct Value result;
    struct Node *unit = m_tree;

    // evaluation of statements or functions should go on stack
    // previous values may affect later values

    int num_stmts = node_get_num_kids(unit);
    int index = 0;

    while (index < num_stmts) {
        struct Node *def = node_get_kid(unit, index);

        result = eval(def);

        // add result to stack

        index++;
    }

  return result;
}

struct Value Interp::eval(struct Node *n) {
    int tag = node_get_tag(n);

    if (tag == NODE_INT_LITERAL) {
        return val_create_ival(strtol(node_get_str(n), nullptr, 10));
    }

    // left and right operands follow
    struct Node *left = node_get_kid(n, 0);
    struct Node *right = node_get_kid(n, 1);

    switch(tag) {
        case NODE_AST_PLUS:
            return val_create_ival(eval(left).ival + eval(right).ival);
        case NODE_AST_MINUS:
            return val_create_ival(eval(left).ival - eval(right).ival);
        case NODE_AST_TIMES:
            return val_create_ival(eval(left).ival * eval(right).ival);
        case NODE_AST_DIVIDE:
            return val_create_ival(eval(left).ival / eval(right).ival);
        default:
            err_fatal("Unknown operator: %d\n", tag);
            return val_create_error();
    }
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
