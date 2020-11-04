//
// Created by Jesse Li on 10/31/20.
//

#ifndef ASSIGN03_SYMTAB_H
#define ASSIGN03_SYMTAB_H

#include <string>
#include <vector>
#include "symbol.h"

struct Symbol;

struct SymbolTable {
private:
    std::vector<Symbol> tab;
    SymbolTable* parent;
    int depth;
public:
    SymbolTable(SymbolTable* outer);
    void insert(Symbol symbol);
    Symbol lookup(const char* name);
    std::vector<Symbol> get_symbols();
    SymbolTable* get_parent();

    // find types created in the outer scope while creating fields in records
    Symbol lookup_global(const char* name);
    void print_sym_tab();
private:
    bool s_exists(const char* name);
};


#endif //ASSIGN03_SYMTAB_H
