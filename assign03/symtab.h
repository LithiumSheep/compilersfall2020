//
// Created by Jesse Li on 10/31/20.
//

#ifndef ASSIGN03_SYMTAB_H
#define ASSIGN03_SYMTAB_H

#include <map>
#include <string>
#include <vector>
#include "symbol.h"


struct SymbolTable {
private:
    std::vector<Symbol> tab;
    SymbolTable* parent;
public:
    void insert(Symbol symbol);

    Symbol lookup_local(const char* name);

    // find types created in the outer scope while creating fields in records
    Symbol lookup_global(const char* name);

    void print_sym_tab();
private:
    bool s_exists(const char* name);
};


#endif //ASSIGN03_SYMTAB_H
