//
// Created by Jesse Li on 10/31/20.
//

#include "symtab.h"
#include "util.h"

void SymbolTable::insert(const char *name, Symbol symbol) {
    // init vs set?
    tab[name] = symbol;
}

Symbol SymbolTable::lookup_local(const char *name) {
    std::map<std::string, Symbol>::const_iterator i = tab.find(name);
    if (i == tab.end()) {
        err_fatal("Undefined variable '%s'\n", name);
    }
    return i->second;
}

Symbol SymbolTable::lookup_global(const char *name) {
    std::map<std::string, Symbol>::const_iterator i = tab.find(name);
    if (i == tab.end()) {
        // did not find the value
        //if (parent == nullptr) {
            err_fatal("Undefined variable '%s'\n", name);
        //}
        //return parent->find_val(name);
    }
    return i->second;
}

bool SymbolTable::s_exists(const char* name) {
    std::map<std::string, Symbol>::const_iterator i = tab.find(name);
    if (i == tab.end()) {
        return false;
    }
    return true;
}

void SymbolTable::print_sym_tab() {
    std::map<std::string, Symbol>::const_iterator i;
    for (i = tab.begin(); i != tab.end(); i++) {
        std::string name = i->first;
        Symbol sym = i->second;
        // depth,kind,name,type
        printf("%s,%s,%s\n", get_name_for_kind(sym.m_kind), name.c_str(), sym.get_type()->get_name());
    }
}