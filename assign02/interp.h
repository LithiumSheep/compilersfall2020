#ifndef INTERP_H
#define INTERP_H

#include "value.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

struct Node;
struct Interp;
struct Environment;

// create an interpreter from a parse tree
struct Interp *interp_create(struct Node *t);

// destroy interpreter
void interp_destroy(struct Interp *interp);

// execute interpreter
struct Value interp_exec(struct Interp *interp);

struct Environment *env_create(struct Environment *parent);

#ifdef __cplusplus
}
#endif

#endif // INTERP_H
