#ifndef VALUE_H
#define VALUE_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

struct Interp;
struct Function;
struct Cons;

typedef struct Value IntrinsicFunction(struct Value *args, int num_args, struct Interp *interp);

enum ValueKind {
  VAL_VOID,
  VAL_ERROR,
  VAL_INT,
  VAL_FN,
  VAL_INTRINSIC,

  // You may add additional enumeration members for additional kinds
  // of values.
};

struct Function {
    struct Node *ast;
    // TODO: env
};

struct Value {
  enum ValueKind kind;
  long ival;
  struct Function *fn;
  IntrinsicFunction *intrinsic_fn;

  // You may add additional struct members for additional kinds
  // of values.
  struct Cons *cons;
};


struct Value val_create_void(void);
struct Value val_create_error(void);
struct Value val_create_ival(long ival);
struct Value val_create_true();
struct Value val_create_false();
struct Value val_create_fn(struct Function *fn);
struct Value val_create_intrinsic(IntrinsicFunction *intrinsic_fn);
// You may add additional constructor functions for additional kinds of values.

// This function returns a string representation of the given Value.
// Note that the returned buffer is allocated using malloc, so should
// be deallocated with free.
char *val_stringify(struct Value val);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // VALUE_H
