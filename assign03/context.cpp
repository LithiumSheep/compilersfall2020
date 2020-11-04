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
private:
    // TODO: Handle symbol depth
    // TODO: Handle type checking
    SymbolTable* symtab;
    Type* integer_type;
public:
    SymbolTable* get_symtab() {
        return symtab;
    }

    SymbolTableBuilder(SymbolTable* symbolTable) {
        symtab = symbolTable;
        integer_type = type_create_primitive("INTEGER");
    }

    void visit_constant_def(struct Node *ast) override {
        recur_on_children(ast);

        // find out type on right
        Node* right = node_get_kid(ast, 1);
        Type* type = right->get_type();

        // get left identifier
        Node* left = node_get_kid(ast, 0);
        const char* name = node_get_str(left);
        // set entry in symtab for name, type
        Symbol* sym = symbol_create(name, type, CONST);
        symtab->insert(*sym);
    }

    void visit_var_def(struct Node *ast) override {
        recur_on_children(ast);

        // find out type on right
        Node* right = node_get_kid(ast, 1);
        Type* type = right->get_type();

        // get left identifier(s)
        Node* left = node_get_kid(ast, 0);
        int num_kids = node_get_num_kids(left);

        for (int i = 0; i < num_kids; i++) {
            Node* id = node_get_kid(left, i);
            const char* name = node_get_str(id);
            // set entry in symtab for name, type
            Symbol* sym = symbol_create(name, type, VARIABLE);
            symtab->insert(*sym);
        }
    }

    void visit_type_def(struct Node *ast) override {
        recur_on_children(ast);

        // find out type on right
        Node* right = node_get_kid(ast, 1);
        Type* type = right->get_type();

        // get left identifier
        Node* left = node_get_kid(ast, 0);
        const char* name = node_get_str(left);

        // set entry in symtab for name, type
        Symbol* sym = symbol_create(name, type, TYPE);
        symtab->insert(*sym);
    }

    void visit_named_type(struct Node *ast) override {
        // set the current type to the child type
        Node* type = node_get_kid(ast, 0);

        const char* type_str = node_get_str(type);
        Type* named_type = type_create_primitive(type_str);

        // perform checks
        // if primitive (INTEGER, CHAR), ok
        // TODO: else if type name, perform lookup, then set type

        ast->set_type(named_type);
        // set the type of the current node
    }

    void visit_array_type(struct Node *ast) override {
        recur_on_children(ast);

        Node* right = node_get_kid(ast, 1);
        Type* type = right->get_type();

        // get left size
        Node* left = node_get_kid(ast, 0);
        long size = strtol(node_get_str(left), nullptr, 10);

        Type* arrayType = type_create_array(size, type);
        ast->set_type(arrayType);
    }

    void visit_record_type(struct Node *ast) override {
        // Records will have their own "scope"
        // records store their fields in an ordered list aka <vector>
        // records print their "inner fields" before printing the record type line

        Type* recordType = type_create_record();
        ast->set_type(recordType);
    }

    void visit_var_ref(struct Node *ast) override {
        ASTVisitor::visit_var_ref(ast);

        /*
        const char* varname = node_get_str(ast);

        // create symbol with current scope
        // Symbol *sym = m_cur_scope->lookup(varname)

        // report errors here or annotate
         */
    }

    void visit_add(struct Node *ast) override {
        ASTVisitor::visit_add(ast);
        /*
        recur_on_children(ast);

        // left_type = ast->get_kid(0)->get_type()
        // right_type = ast->get_kid(1)->get_type()

        // type check
         */
    }

    void visit_identifier(struct Node *ast) override {
        ASTVisitor::visit_identifier(ast);
    }

    void visit_int_literal(struct Node *ast) override {
        // set type to integer
        ast->set_type(integer_type);
    }
};

////////////////////////////////////////////////////////////////////////
// Context class implementation
////////////////////////////////////////////////////////////////////////

Context::Context(struct Node *ast) {
    root = ast;
    symtab = new SymbolTable(nullptr);
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
    SymbolTableBuilder *visitor = new SymbolTableBuilder(symtab);
    visitor->visit(root);

    if (flag_print) {
      // print symbol table
      visitor->get_symtab()->print_sym_tab();
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
