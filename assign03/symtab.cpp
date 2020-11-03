//
// Created by Jesse Li on 10/31/20.
//

#include "symtab.h"
#include "util.h"

SymbolTable::SymbolTable(SymbolTable *outer) {
    parent = outer;
    if (outer == nullptr) {
        depth = 0;
    } else {
        depth = outer->depth + 1;
    }
}

void SymbolTable::insert(Symbol symbol) {
    if (s_exists(symbol.get_name())) {
        err_fatal("Symbol with name %s already exists", symbol.get_name());
    }
    tab.push_back(symbol);
}

Symbol SymbolTable::lookup(const char *name) {
    // searching for names in the current scope
    // redefining a name in local scope is an error

    // for loop
    for (auto sym : tab) {
        if (sym.get_name() == name) {
            return sym;
        }
    }
    if (parent != nullptr) {
        return parent->lookup(name);
    }
    err_fatal("Undefined variable '%s'\n", name);
}

bool SymbolTable::s_exists(const char* name) {
    std::vector<Symbol>::iterator i;
    for (i = tab.begin(); i != tab.end(); i++) {
        if (i->get_name() == name) {
            return true;
        }
    }
    // then search parent

    return false;
}

void SymbolTable::print_sym_tab() {
    for (auto sym : tab) {
        //TODO: Depth
        //using SymbolTable.depth

        if (sym.get_kind() == RECORD) {
            // print record internals first
            // record.get_sym_tab().print_sym_tab();
        }

        // kind
        std::string kind_name = get_name_for_kind(sym.get_kind());
        // name
        std::string name = sym.get_name();
        //type
        std::string type_name = sym.get_type()->to_string();
        // depth,kind,name,type
        printf("%d,%s,%s,%s\n", depth, kind_name.c_str(), name.c_str(), type_name.c_str());
    }
}