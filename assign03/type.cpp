//
// Created by Jesse Li on 10/31/20.
//

#include "cpputil.h"
#include "type.h"
#include <string>


Type* type_create_primitive(const char* name) {
    Type* primitive = new Type();
    primitive->realType = PRIMITIVE;
    primitive->name = name;
    return primitive;
}

Type* type_create_array(long size, Type* elementType) {
    Type* arr = new Type();
    arr->realType = ARRAY;
    arr->arraySize = size;
    arr->arrayElementType = elementType;
    return arr;
}

Type* type_create_record(const char* name) {
    Type* record = new Type();
    record->realType = RECORD;
    record->name = name;
    return record;
}

std::string Type::to_string() {
    if (realType == ARRAY) {
        return cpputil::format("ARRAY %ld OF %s", arraySize, arrayElementType->to_string().c_str());
    } else if (realType == RECORD) {
        // needs to print out types/symbols inside Record, then "RECORD (field_type x field_type x ...);
        return "RECORD";
    } else {
        return name;    // expected to be the name of the primitive
    }
}
