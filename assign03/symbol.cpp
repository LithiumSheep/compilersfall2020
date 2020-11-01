//
// Created by Jesse Li on 10/31/20.
//

#include "symbol.h"

struct Symbol *symbol_create(char* name, Type type, Kind kind) {
    Symbol *symbol = new Symbol();
    symbol->name = name;
    symbol->type = type;
    symbol->kind = kind;
    return symbol;
}
