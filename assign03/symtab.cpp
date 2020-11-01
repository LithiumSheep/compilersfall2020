//
// Created by Jesse Li on 10/31/20.
//

#include "symtab.h"
#include "util.h"


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