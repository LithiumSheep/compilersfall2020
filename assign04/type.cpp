//
// Created by Jesse Li on 10/31/20.
//

#include "cpputil.h"
#include "type.h"

static int INTEGER_SIZE = 8;
static int CHAR_SIZE = 1;

Type::Type(int realType) : realType(realType) {}

long Type::get_size() {
    return size;
}

Type* type_create_integer() {
    Type* integer = new Type(PRIMITIVE);
    integer->name = "INTEGER";
    integer->size = INTEGER_SIZE;
    return integer;
}

Type* type_create_char() {
    Type* integer = new Type(PRIMITIVE);
    integer->name = "CHAR";
    integer->size = CHAR_SIZE;
    return integer;
}

Type* type_create_array(long size, Type* elementType) {
    Type* arr = new Type(ARRAY);
    arr->arraySize = size;
    arr->arrayElementType = elementType;
    arr->size = size * elementType->get_size();
    return arr;
}

Type* type_create_record(SymbolTable* symbolTable) {
    Type* record = new Type(RECORD);
    record->symtab = symbolTable;
    // TODO: set size of record
    return record;
}

std::string Type::to_string() {
    switch(realType){
        case PRIMITIVE:
            return name;
        case ARRAY:
            return cpputil::format("ARRAY %ld OF %s", arraySize, arrayElementType->to_string().c_str());
        case RECORD: {
            std::string record_str = "RECORD (";
            std::vector<Symbol> symbols = symtab->get_symbols();
            for (int i = 0; i < symbols.size(); i++) {
                if (i == 0) {
                    record_str += symbols[i].get_type()->to_string();
                } else {
                    record_str += " x " + symbols[i].get_type()->to_string();
                }
            }
            record_str += ")";
            return record_str;
        }
        default:
            return "<<unknown>>";
    }
}
