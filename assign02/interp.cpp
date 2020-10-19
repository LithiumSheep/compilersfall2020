#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <string>
#include <map>
#include "cpputil.h"
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
  std::map<std::string, Value> m_vars;

public:
  Interp(struct Node *t);
  ~Interp();

  struct Value exec();

private:
  struct Value eval(struct Node *n);
  struct Value exec_from_node(struct Node *n);
  bool val_is_truthy(Value val);
};

Interp::Interp(struct Node *t) : m_tree(t) {
}

Interp::~Interp() {
}

// TODO: exec with scope?
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

struct Value Interp::eval(struct Node *statement) {
    int tag = node_get_tag(statement);

    if (tag == NODE_INT_LITERAL) {
        return val_create_ival(strtol(node_get_str(statement), nullptr, 10));
    }

    if (tag == NODE_AST_VAR_DEC) {
        int num_kids = node_get_num_kids(statement);
        // TODO: for all declared variables, assign to 0;
        return val_create_void();
    }

    if (tag == NODE_IDENTIFIER) {
        const char* lexeme = node_get_str(statement);
        std::map<std::string, Value>::const_iterator i = m_vars.find(lexeme);
        if (i == m_vars.end()) {
            err_fatal("Undefined variable '%s'", lexeme);
        }
        return i->second;
    }

    // TODO: verify if statements are executing correctly
    if (tag == NODE_AST_IF) {
        struct Node *condition = node_get_kid(statement, 0);
        struct Node *if_clause = node_get_kid(statement, 1);
        struct Node *else_clause = nullptr;
        if (node_get_num_kids(statement) == 3) {    // there is an else clause
            else_clause = node_get_kid(statement, 2);
        }

        if (val_is_truthy(eval(condition))) {
            exec_from_node(if_clause);
        } else {
            if (else_clause) {
                exec_from_node(if_clause);
            }
        }
        // the result of evaluating an if or if/else statement is a VAL_VOID value
        return val_create_void();
    }

    // left and right operands follow
    struct Node *left = node_get_kid(statement, 0);
    struct Node *right = node_get_kid(statement, 1);

    if (tag == NODE_AST_ASSIGN) {
        std::string varname = node_get_str(left);
        struct Value val = eval(right);
        if (val.kind != VAL_INT) {
            err_fatal("Cannot assign non-int value to variable");
        }
        m_vars[varname] = val;

        return val_create_void(); // assignment is a void val type
    }

    switch (tag) {
        case NODE_AST_PLUS:
            return val_create_ival(eval(left).ival + eval(right).ival);
        case NODE_AST_MINUS:
            return val_create_ival(eval(left).ival - eval(right).ival);
        case NODE_AST_TIMES:
            return val_create_ival(eval(left).ival * eval(right).ival);
        case NODE_AST_DIVIDE:
            return val_create_ival(eval(left).ival / eval(right).ival);
        case NODE_AST_AND:
            if (val_is_truthy(eval(left)) && val_is_truthy(eval(right))) {
                return val_create_true();
            }
            return val_create_false();
        case NODE_AST_OR:
            if (val_is_truthy(eval(left)) || val_is_truthy(eval(right))) {
                return val_create_true();
            }
            return val_create_false();
        case NODE_AST_EQ:
            if (eval(left).ival == eval(right).ival) {
                return val_create_true();
            }
            return val_create_false();
        case NODE_AST_NE:
            if (eval(left).ival != eval(right).ival) {
                return val_create_true();
            }
            return val_create_false();
        case NODE_AST_LT:
            if (eval(left).ival < eval(right).ival) {
                return val_create_true();
            }
            return val_create_false();
        case NODE_AST_LE:
            if (eval(left).ival <= eval(right).ival) {
                return val_create_true();
            }
            return val_create_false();
        case NODE_AST_GT:
            if (eval(left).ival > eval(right).ival) {
                return val_create_true();
            }
            return val_create_false();
        case NODE_AST_GE:
            if (eval(left).ival >= eval(right).ival) {
                return val_create_true();
            }
            return val_create_false();
        default:
            err_fatal("Unknown operator: %d\n", tag);
            return val_create_error();
    }
}

bool Interp::val_is_truthy(Value val) {
    // TODO: more types of truthiness
    return val.ival >= 1;
}

struct Value Interp::exec_from_node(struct Node *n) {
    // evaluate a list of statements underneath n

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
