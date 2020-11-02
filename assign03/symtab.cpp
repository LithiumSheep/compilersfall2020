//
// Created by Jesse Li on 10/31/20.
//

#include "symtab.h"
#include "util.h"

void SymbolTable::insert(const char *name, Symbol symbol) {
    // init vs set?
    tab.push_back(symbol);
}
/*
Symbol SymbolTable::lookup_local(const char *name) {
    std::map<std::string, Symbol>::const_iterator i = tab.find(name);
    if (i == tab.end()) {
        err_fatal("Undefined variable '%s'\n", name);
    }
    return i->second;
}

Symbol SymbolTable::lookup_global(const char *name) {
    std::vector<Symbol>::iterator i = tab.begin();
    if (i == tab.end()) {
        // did not find the value
        //if (parent == nullptr) {
            err_fatal("Undefined variable '%s'\n", name);
        //}
        //return parent->find_val(name);
    }
    return i;
}
*/

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
        long depth = 0;

        // kind
        std::string kind_name = get_name_for_kind(sym.m_kind);
        // name
        std::string name = sym.get_name();
        //type
        std::string type_name = sym.get_type()->to_string();
        // depth,kind,name,type
        printf("%ld,%s,%s,%s\n", depth, kind_name.c_str(), name.c_str(), type_name.c_str());
    }
}