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
    // TODO: depth?
    char* m_name;
    Type* m_type;
    int m_kind;
    Type* get_type();
};

Symbol *symbol_create(char* name, Type type, Kind kind);

#endif //ASSIGN03_SYMBOL_H
