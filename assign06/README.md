## Procedures

### Anatomy of procedure
A procedure in assembly is defined as a directive that looks like `.globl func`
followed by a label `func:`, which can then be called from main or another 
procedure with the instruction `call func`.  

Any arguments that the procedure
wants are passed in `%rdi`, `%rsi`, `%rdx`, `%rcx`, `%r8`, and `%r9`.  Any 
more than 6 arguments should be passed through the stack, `%rsp`.

When entering a procedure, we want to preserve the base pointer, generally
represented by `%rbp`, by pushing and popping from the stack.

To create a local scope for variables inside a procedure, one method to do so would
be to copy the location of `%rsp` onto `%rbp` and there, use `%rbp` as the base
address for local variables in the procedure.

If the function has a return value, the value will be stored in `%rax` for the
calling body to use.

```
// example procedure for add(a, b) might look like this:
FUNCTION add(a, b: INTEGER): INTEGER
BEGIN
    a + b;
END.

add(1, 1);

// generated assembly 
    .globl add
add:
    pushq %rbp
    movq %rsp, %rbp
    movq %rdi, 16(%rbp)
    movq %rsi, 24(%rbp)
    movq 16(%rbp), %rdx
    movq 24(%rbp), %rax
    addq %rdx, %rax
    popq %rbp
    ret
    .globl main
main:
    ...
    movq $1, %rdi
    movq $1, %rsi
    call add
    movq %rax, %r10
    ...
```

## Design & Implementation
In this implementation, the structure of an extremely basic procedure is 
produced in assembly. The syntax for a procedure declaration and call are 
defined as:

```
// declaration
FUNCTION func_name(): TYPE
BEGIN
    // instructions
END.

// call
func_name();

```

In the highlevel intermediate representation, a few HINS_ instructions were
added.

```
switch (opcode) {
    ...
    case HINS_MAIN_START: return "mainstart";
    case HINS_MAIN_FINISH: return "mainfinish";
    case HINS_FUNC_ENTER:  return "funcenter";
    case HINS_FUNC_LEAVE:  return "funcleave";
}
```

Special `funcenter` and `funcleave` highlevel instructions allowed for generation 
of the `%rbp` push and pop instructions.  Similarly, the mainstart and mainfinish
instructions were added to allow moving of the `.globl main` directive and define
main below function definitions, which removed the need to directly print the preamble
and epilogue. Very few assembly instructions needed to be modified.

## Limitations
Unfortunately, partially due to time constraints and primarily due to implementation 
shortcomings, I was only able to get an extremely rudimentary procedure working.

The procedure at the moment must be a 0-argument function without any instructions in the body. 
There are several issues that cause these limitations, including
- Parsing
  - An collision issue with parsing "opt_arguments_list" and "indentifier_list" caused
    issues with being able to parse a function definition with arguments
- Closure & scope
  - No special symbol table was generated for argument parameters and variables
    declared inside a function body.  In addition, a special addressing flag for 
    instructions inside a function was not provided that would let variables be defined
    as offsets of `%rbp`.
- Stack alignment
    - Stack alignment is not properly calculated inside a procedure, potentially
    causing issues with calling linked functions `printf` and `scanf`
- Directives
    - directives for main and functions were somewhat "hacked" by using labels to 
    define both directive and label, like `code->define_label(\t.globl func\nfunc)`
      
At the current time, the compiler just demonstrates that a procedure with a name
can be defined in assembly, does push and pop the `%rbp` register, and can be called
by name in main.