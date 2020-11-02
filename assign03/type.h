//
// Created by Jesse Li on 10/31/20.
//

#ifndef ASSIGN03_TYPE_H
#define ASSIGN03_TYPE_H

#include <string>

enum RealType {
    PRIMITIVE = 0,
    ARRAY,
    RECORD
};

struct Type {
    // contain all variables for all types
    // use enum for the explicit type, e.g. PRIMITIVE, ARRAY, RECORD
    int realType;

    long arraySize;
    Type* arrayElementType;

    const char* name;
public:
    Type(int realType);
    ~Type();
    std::string to_string();
};

Type* type_create_primitive(const char* name);

Type* type_create_array(long size, Type* elementType);

Type* type_create_record(const char* name);

#endif //ASSIGN03_TYPE_H
