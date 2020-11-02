//
// Created by Jesse Li on 10/31/20.
//

#include "type.h"

Type::Type(int realType) : realType(realType) {}

Type* type_create_primitive(const char* name) {
    Type* primitive = new Type(PRIMITIVE);
    primitive->name = name;
    return primitive;
}

Type* type_create_array(long size, Type* elementType) {
    Type* arr = new Type(ARRAY);
    arr->arraySize = size;
    arr->arrayElementType = elementType;
    return arr;
}

Type* type_create_record(const char* name) {
    Type* record = new Type(RECORD);
    record->name = name;
    return record;
}

std::string Type::to_string() {
    switch(realType){
        case PRIMITIVE:
            return name;
        case ARRAY:
            return "ARRAY size OF type";
        case RECORD:
            return "RECORD";
        default:
            return "<<unknown>>";
    }
}
