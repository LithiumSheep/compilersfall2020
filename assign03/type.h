//
// Created by Jesse Li on 10/31/20.
//

#ifndef ASSIGN03_TYPE_H
#define ASSIGN03_TYPE_H


struct Type {
private:
    char* name;
public:
    char* get_name();
};

struct PrimitiveType: Type {
};

struct ArrayType: Type {
private:
    int size;
    Type elementType;
public:
    int get_size();
    Type get_type();
};

struct RecordType: Type {
private:
    // list of fields
};

Type create_primitive(char* name);

Type type_create_array(char* name, int size);

Type type_create_record(char* name);

#endif //ASSIGN03_TYPE_H
