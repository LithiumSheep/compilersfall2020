//
// Created by Jesse Li on 10/31/20.
//

#ifndef ASSIGN03_SYMBOL_H
#define ASSIGN03_SYMBOL_H

#include "type.h"

struct Type;

enum Kind {
    VARIABLE = 5000,
    CONST,
    TYPE
};

struct Symbol {
public:
    const char* m_name;
    Type* m_type;
    int m_kind;
    long m_offset;
    long ival;
    const char* get_name();
    Type* get_type();
    int get_kind();
    long get_size();
    long get_offset();
    long get_ival();
    void set_ival(long val);
};

Symbol *symbol_create(const char* name, Type* type, int kind, long offset);

const char* get_name_for_kind(int kind);

#endif //ASSIGN03_SYMBOL_H
