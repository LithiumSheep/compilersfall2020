//
// Created by Jesse Li on 10/31/20.
//

#include "symbol.h"

Type* Symbol::get_type() {
    return m_type;
}

struct Symbol *symbol_create(char* name, Type* type, Kind kind) {
    Symbol *symbol = new Symbol();
    symbol->m_name = name;
    symbol->m_type = type;
    symbol->m_kind = kind;
    return symbol;
}
