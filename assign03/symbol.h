//
// Created by Jesse Li on 10/31/20.
//

#ifndef ASSIGN03_SYMBOL_H
#define ASSIGN03_SYMBOL_H

#include "type.h"

enum Kind {
    VARIABLE = 0,
    TYPE
};

struct Symbol {
public:
    const char* m_name;
    Type* m_type;
    int m_kind;
    Type* get_type();
};

Symbol *symbol_create(const char* name, Type* type, Kind kind);

const char* get_name_for_kind(int kind);

#endif //ASSIGN03_SYMBOL_H
