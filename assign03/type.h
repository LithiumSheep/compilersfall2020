//
// Created by Jesse Li on 10/31/20.
//

#ifndef ASSIGN03_TYPE_H
#define ASSIGN03_TYPE_H

// TODO: Is this the right way to define Type by creating inherited classes of the base type which always has a name?

struct Type {
public:
    const char* name;
    const char* get_name();
};

struct PrimitiveType: Type {
    // INTEGER
    // CHAR
};

struct ArrayType: Type {
private:
public:
    int size;
    Type* elementType;
    int get_size();
    Type* get_type();
};

struct RecordType: Type {
private:
    // list of fields
};

Type* type_create_primitive(const char* name);

Type* type_create_array(const char* name, int size, Type* elementType);

Type* type_create_record(const char* name);

#endif //ASSIGN03_TYPE_H
