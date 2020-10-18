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
    while (unit) {
        struct Node *def = node_get_kid(unit, 0);
        result = eval(def);

        if (node_get_num_kids(def) == 1) {
            unit = node_get_kid(def, 0);
        } else if (node_get_num_kids(def) == 2) {
            unit = node_get_kid(def, 1);
        } else {
            unit = nullptr;
        }
    }

  return result;
}

struct Value Interp::eval(struct Node *n) {
    int num_kids = node_get_num_kids(n);
    int tag = node_get_tag(n);

    if (tag == NODE_INT_LITERAL) {
        return val_create_ival(strtol(node_get_str(n), nullptr, 10));
    }

    if (num_kids == 0) {
        return val_create_ival(node_get_ival(n));
    }

    // left and right operands follow
    struct Node *left = node_get_kid(n, 0);
    struct Node *right = node_get_kid(n, 1);
    switch(tag) {
        case NODE_AST_PLUS:
            return val_create_ival(eval(left).ival + eval(right).ival);
        case NODE_AST_MINUS:
            return val_create_ival(eval(left).ival - eval(right).ival);
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
