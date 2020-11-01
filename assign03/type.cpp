//
// Created by Jesse Li on 10/31/20.
//

#include "type.h"
#include <iostream>
#include <string>

const char * PrimitiveType::get_name() {
    return name;
}

long ArrayType::get_size() {
    return size;
}

Type* ArrayType::get_type() {
    return elementType;
}

Type* type_create_primitive(const char* name) {
    PrimitiveType* primitive = new PrimitiveType();
    primitive->name = name;
    return primitive;
}

Type* type_create_array(long size, Type* elementType) {
    ArrayType* arr = new ArrayType();
    arr->size = size;
    arr->elementType = elementType;
    return arr;
}

Type* type_create_record(const char* name);

const char *Type::describe() {
    return "";
}

const char *PrimitiveType::describe() {
    return get_name();
}

const char *ArrayType::describe() {
    std::string descrip = "ARRAY " + std::to_string(get_size()) + " OF " + get_type()->describe();
    return strdup(descrip.c_str());
}
