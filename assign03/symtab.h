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
public:
    void define();
    Symbol lookup_local(const char* name);
    Symbol lookup_global(const char* name);
};


#endif //ASSIGN03_SYMTAB_H
