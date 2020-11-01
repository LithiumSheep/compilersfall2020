#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <set>
#include "util.h"
#include "cpputil.h"
#include "node.h"
#include "type.h"
#include "symbol.h"
#include "symtab.h"
#include "ast.h"
#include "astvisitor.h"
#include "context.h"

////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////

struct Context {
private:
    Node *root;
    SymbolTable *symtab;
    bool flag_print;

public:
  Context(struct Node *ast);
  ~Context();

  void set_flag(char flag);

  void build_symtab();

  // TODO: additional methods
};

class SymbolTableBuilder : public ASTVisitor {
public:
    void visit_constant_def(struct Node *ast) override {
        ASTVisitor::visit_constant_def(ast);
    }

    void visit_type_def(struct Node *ast) override {
        ASTVisitor::visit_type_def(ast);
    }

    void visit_named_type(struct Node *ast) override {
        // set the current type to the child type
        Node* type = node_get_kid(ast, 0);

        const char* type_str = node_get_str(type);
        Type* named_type = type_create_primitive(type_str);

        ast->set_type(named_type);
        // set the type of the current node
    }

    void visit_array_type(struct Node *ast) override {
        ASTVisitor::visit_array_type(ast);
    }

    void visit_record_type(struct Node *ast) override {
        ASTVisitor::visit_record_type(ast);
    }

    void visit_var_def(struct Node *ast) override {
        recur_on_children(ast);

        // find out type on right
        Node* right = node_get_kid(ast, 1);
        Type* type = right->get_type();

        // get left identifier(s)
        Node* left = node_get_kid(ast, 0);
        int num_kids = node_get_num_kids(left);

        printf("Set %d kids to type %s", num_kids, type->get_name());
    }

    void visit_int_literal(struct Node *ast) override {
        // set type to integer
        ast->set_type(type_create_primitive("INTEGER"));
    }

    void visit_var_ref(struct Node *ast) override {
        const char* varname = node_get_str(ast);

        // create symbol with current scope
        // Symbol *sym = m_cur_scope->lookup(varname)

        // report errors here or annotate
    }

    void visit_identifier(struct Node *ast) override {
        ASTVisitor::visit_identifier(ast);
    }

    void visit_add(struct Node *ast) override {
        recur_on_children(ast);

        // left_type = ast->get_kid(0)->get_type()
        // right_type = ast->get_kid(1)->get_type()

        // type check
    }
};

////////////////////////////////////////////////////////////////////////
// Context class implementation
////////////////////////////////////////////////////////////////////////

Context::Context(struct Node *ast) {
    root = ast;
    symtab = new SymbolTable();
    flag_print = false;
}

Context::~Context() {
}

void Context::set_flag(char flag) {
  if (flag == 's') {
      flag_print = true;
  }
}

void Context::build_symtab() {

    // give symtabbuilder a symtab in constructor?
    SymbolTableBuilder *visitor = new SymbolTableBuilder();
    visitor->visit(root);

    if (flag_print) {
      // print symbol table
    }
}

// TODO: implementation of additional Context member functions

// TODO: implementation of member functions for helper classes

////////////////////////////////////////////////////////////////////////
// Context API functions
////////////////////////////////////////////////////////////////////////

struct Context *context_create(struct Node *ast) {
  return new Context(ast);
}

void context_destroy(struct Context *ctx) {
  delete ctx;
}

void context_set_flag(struct Context *ctx, char flag) {
  ctx->set_flag(flag);
}

void context_build_symtab(struct Context *ctx) {
  ctx->build_symtab();
}

void context_check_types(struct Context *ctx) {
}
