//
// Created by Jesse Li on 10/31/20.
//

#include "symbol.h"

const char* Symbol::get_name() {
    return m_name;
}

Type* Symbol::get_type() {
    return m_type;
}

int Symbol::get_kind() {
    return m_kind;
}

int Symbol::get_offset() {
    return m_offset;
}

struct Symbol *symbol_create(const char* name, Type* type, int kind, int offset) {
    Symbol *symbol = new Symbol();
    symbol->m_name = name;
    symbol->m_type = type;
    symbol->m_kind = kind;
    symbol->m_offset = offset;
    return symbol;
}

const char* get_name_for_kind(int kind) {
    if (kind == TYPE) {
        return "TYPE";
    } else if (kind == CONST) {
        return "CONST";
    } else {
        return "VAR";
    }
}
