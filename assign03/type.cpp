//
// Created by Jesse Li on 10/31/20.
//

#include "type.h"

const char * Type::get_name() {
    return name;
}

int ArrayType::get_size() {
    return size;
}

Type ArrayType::get_type() {
    return elementType;
}

Type* type_create_primitive(const char* name) {
    Type* primitive = new Type();
    primitive->name = name;
    return primitive;
}

Type* type_create_array(const char* name, int size) {
    ArrayType* arr = new ArrayType();
    arr->name = name;
    arr->size = size;
    return arr;
}

Type* type_create_record(const char* name);
