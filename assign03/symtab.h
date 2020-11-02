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
    std::map<std::string, Symbol> tab;
    // TODO: table would best be implemented as a vector
    // TODO: pointer to parent table
public:
    void insert(const char* name, Symbol);
    Symbol lookup_local(const char* name);
    Symbol lookup_global(const char* name);
    void print_sym_tab();
private:
    bool s_exists(const char* name);
};


#endif //ASSIGN03_SYMTAB_H
