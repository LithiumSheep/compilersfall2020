//
// Created by Jesse Li on 10/31/20.
//

#ifndef ASSIGN03_TYPE_H
#define ASSIGN03_TYPE_H

// TODO: Is this the right way to define Type by creating inherited classes of the base type which always has a name?

struct Type {
    virtual const char* describe();
};

struct PrimitiveType: Type {
    // INTEGER
    // CHAR
public:
    const char* name;
    const char* get_name();
    const char* describe() override;
};

struct ArrayType: Type {
private:
public:
    long size;
    Type* elementType;
    long get_size();
    Type* get_type();
    const char* describe() override;
};

struct RecordType: Type {
private:
    // list of fields
};

Type* type_create_primitive(const char* name);

Type* type_create_array(long size, Type* elementType);

Type* type_create_record(const char* name);

#endif //ASSIGN03_TYPE_H
