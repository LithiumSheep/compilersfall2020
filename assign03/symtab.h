//
// Created by Jesse Li on 10/31/20.
//

#ifndef ASSIGN03_SYMTAB_H
#define ASSIGN03_SYMTAB_H

#include <vector>
#include "symbol.h"


struct SymbolTable {
private:
    std::vector<Symbol> tab;
public:
    void define();
    Symbol *lookup_local(char* name);
    Symbol *lookup_global(char* name);
};


#endif //ASSIGN03_SYMTAB_H
