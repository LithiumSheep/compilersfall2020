#include <cassert>
#include <cstdio>
#include <cstdarg>
#include <cstdlib>
#include <cstring>
#include <string>
#include <set>
#include "util.h"
#include "cpputil.h"
#include "node.h"
#include "type.h"
#include "symbol.h"
#include "symtab.h"
#include "ast.h"
#include "astvisitor.h"
#include "context.h"
#include "cfg.h"
#include "highlevel.h"
#include "x86_64.h"

////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////

struct Context {
private:
    Node *root;
    SymbolTable *global;
    bool flag_print_symtab;
    bool flag_print_hins;

public:
  Context(struct Node *ast);
  ~Context();

  void set_flag(char flag);

  void build_symtab();
  void print_err(Node* node, const char *fmt, ...);

  void gen_code();
};

// Known issues:
// bug: Modifying enum start value in enums Kind and RealType change the behavior of record printing (???)
// unimplemented: Consts can be dereferenced and used in subsequent declarations
// unimplemented: Consts can be checked for variable references and throw an error
// unimplemented: array and field references are not being type checked
// unimolemented: READ and WRITE operands are not being checked
//
class SymbolTableBuilder : public ASTVisitor {
private:
    SymbolTable* scope;
    Type* integer_type;
    Type* char_type;
    long curr_offset = 0;
public:

    void print_err(Node* node, const char* fmt, ...) {

        SourceInfo info = node_get_source_info(node);
        //std::string error_start_str = cpputil::format("%s:%d:%d: Error: %s", info.filename, info.line, info.col, fmt);

        fprintf(stderr, "%s:%d:%d: Error: ", info.filename, info.line, info.col);

        va_list args;
        va_start(args, fmt);
        err_fatal(fmt, args);
        va_end(args);
    }

    SymbolTable* get_symtab() {
        return scope;
    }

    int get_curr_offset() {
        return curr_offset;
    }

    void incr_curr_offset(long offset) {
        curr_offset += offset;
    }

    SymbolTableBuilder(SymbolTable* symbolTable) {
        scope = symbolTable;
        integer_type = type_create_integer();
        char_type = type_create_char();
    }

    void visit_constant_def(struct Node *ast) override {
        recur_on_children(ast);

        // find out type on right
        Node* right = node_get_kid(ast, 1);
        Type* type = right->get_type();

        // get left identifier
        Node* left = node_get_kid(ast, 0);
        const char* name = node_get_str(left);

        // set entry in symtab for name, type
        int offset = get_curr_offset();
        Symbol* sym = symbol_create(name, type, CONST, offset);
        incr_curr_offset(type->get_size());

        if (scope->s_exists(name)) {
            SourceInfo info = node_get_source_info(left);
            err_fatal("%s:%d:%d: Error: Name '%s' is already defined\n", info.filename, info.line, info.col, name);
        } else {
            scope->insert(*sym);
        }
    }

    void visit_var_def(struct Node *ast) override {
        recur_on_children(ast);

        // find out type on right
        Node* right = node_get_kid(ast, 1);
        Type* type = right->get_type();

        // get left identifier(s)
        Node* left = node_get_kid(ast, 0);
        int num_kids = node_get_num_kids(left);

        for (int i = 0; i < num_kids; i++) {
            Node* id = node_get_kid(left, i);
            const char* name = node_get_str(id);
            // set entry in symtab for name, type
            int offset = get_curr_offset();
            Symbol* sym = symbol_create(name, type, VARIABLE, offset);
            incr_curr_offset(type->get_size());
            if (scope->s_exists(name)) {
                SourceInfo info = node_get_source_info(left);
                err_fatal("%s:%d:%d: Error: Name '%s' is already defined\n", info.filename, info.line, info.col, name);
            } else {
                scope->insert(*sym);
            }
        }
    }

    void visit_type_def(struct Node *ast) override {
        recur_on_children(ast);

        // find out type on right
        Node* right = node_get_kid(ast, 1);
        Type* type = right->get_type();

        // get left identifier
        Node* left = node_get_kid(ast, 0);
        const char* name = node_get_str(left);

        // set entry in symtab for name, type
        int offset = get_curr_offset();
        Symbol* sym = symbol_create(name, type, TYPE, offset);
        incr_curr_offset(type->get_size());
        if (scope->s_exists(name)) {
            SourceInfo info = node_get_source_info(left);
            err_fatal("%s:%d:%d: Error: Name '%s' is already defined\n", info.filename, info.line, info.col, name);
        } else {
            scope->insert(*sym);
        }
    }

    void visit_named_type(struct Node *ast) override {
        Node* type = node_get_kid(ast, 0);

        const char* type_str = node_get_str(type);
        Type* named_type;

        if (std::strcmp(type_str, "INTEGER") == 0) {
            named_type = integer_type;
        } else if (std::strcmp(type_str, "CHAR") == 0) {
            named_type = char_type;
        } else {
            // perform lookup
            if (scope->s_exists(type_str)) {
                Symbol typeSymbol = scope->lookup(type_str);
                named_type = typeSymbol.get_type();
            } else {
                SourceInfo info = node_get_source_info(type);
                err_fatal("%s:%d:%d: Error: Unknown type '%s'\n", info.filename, info.line, info.col, type_str);
                named_type = nullptr;
            }
        }
        // set the type of the current node
        ast->set_type(named_type);
    }

    void visit_array_type(struct Node *ast) override {
        recur_on_children(ast);

        Node* right = node_get_kid(ast, 1);
        Type* type = right->get_type();

        // get left size
        Node* left = node_get_kid(ast, 0);
        long size = node_get_ival(left);

        Type* arrayType = type_create_array(size, type);
        ast->set_type(arrayType);
    }

    void visit_record_type(struct Node *ast) override {
        // Records will have their own "scope"
        // records store their fields in an ordered list aka <vector>
        // records print their "inner fields" before printing the record type line

        SymbolTable* nestedSymTab = new SymbolTable(scope);
        scope = nestedSymTab;

        recur_on_children(ast); // will populate the nested scope with values

        scope = scope->get_parent();    // bring it back to parent scope

        Type* recordType = type_create_record(nestedSymTab);
        ast->set_type(recordType);
    }

    void visit_var_ref(struct Node *ast) override {
        Node* ident = node_get_kid(ast, 0);
        const char* varname = node_get_str(ident);

        if (scope->s_exists(varname)) {
            // if name references a TYPE or RECORD, is also wrong
        } else {
            SourceInfo info = node_get_source_info(ident);
            err_fatal("%s:%d:%d: Error: Undefined variable '%s'\n", info.filename, info.line, info.col, varname);
        }
        Symbol sym = scope->lookup(varname);
        ast->set_str(varname);
        ast->set_type(sym.get_type());
        ast->set_source_info(node_get_source_info(ident));
    }

    void visit_identifier(struct Node *ast) override {
        ASTVisitor::visit_identifier(ast);

        // TODO: Consts can be deferenced and have a long value set to the node
        const char* identifier = node_get_str(ast);
    }

    void visit_int_literal(struct Node *ast) override {
        // set literal value
        ast->set_ival(strtol(node_get_str(ast), nullptr, 10));
        // set type to integer
        ast->set_type(integer_type);
    }
};

class HighLevelCodeGen : public ASTVisitor {

private:
    long m_vreg = -1;
    long m_vreg_max = 0;
    SymbolTable m_symtab;
    InstructionSequence* code;
    Operand rsp = Operand(OPERAND_MREG, MREG_RSP);
    Operand printf_label = Operand("printf");
    Operand scanf_label = Operand("scanf");

public:
    HighLevelCodeGen(const SymbolTable &mSymtab) : m_symtab(mSymtab) {
        code = new InstructionSequence();
    }

    long next_vreg() {
        m_vreg += 1;
        if (m_vreg_max < (m_vreg)) {
            m_vreg_max = m_vreg;
        }
        return m_vreg;
    }

    void reset_vreg() {
        m_vreg = -1;
    }

    InstructionSequence* get_iseq() {
        return code;
    }

    long get_storage_size() {
        return m_symtab.get_total_size();
    }

    long get_vreg_max() {
        return m_vreg_max;
    }

public:

    void visit_declarations(struct Node *ast) override {
        //ASTVisitor::visit_declarations(ast);
        // fixme: Does codegen need to visit any declarations?
    }

    void visit_instructions(struct Node *ast) override {
        // fixme: Call reset_vregs before instructions are visited
        ASTVisitor::visit_instructions(ast);
        // additinally, for any given instruction, if the parent is the a node AST_INSTRUCTIONS, we can call reset_vregs();
    }

    bool check_parent_node(struct Node *ast) {
        return true;
    }

    void visit_read(struct Node *ast) override {
        ASTVisitor::visit_read(ast);

        // readint into vreg
        // readi vr1
        long readreg = next_vreg();
        Operand readdest(OPERAND_VREG, readreg);
        auto *readins = new Instruction(HINS_READ_INT, readdest);
        code->add_instruction(readins);

        // storeint into loaded addr
        // sti (vr0), vr1
        Node *varref = node_get_kid(ast, 0);
        Operand destreg = varref->get_operand();    // don't use this one
        Operand toaddr(OPERAND_VREG_MEMREF, destreg.get_base_reg());   // use this one
        auto *storeins = new Instruction(HINS_STORE_INT, toaddr, readdest);
        code->add_instruction(storeins);

        reset_vreg();
    }

    void visit_write(struct Node *ast) override {
        ASTVisitor::visit_write(ast);

        // loadint from addr to vreg
        // ldi vr1, (vr0)
        long loadreg = next_vreg();
        Operand writedest(OPERAND_VREG, loadreg);
        Node *varref = node_get_kid(ast, 0);
        Operand fromreg = varref->get_operand();    // don't use this one
        Operand fromaddr(OPERAND_VREG_MEMREF, fromreg.get_base_reg()); // use this one
        auto *loadins = new Instruction(HINS_LOAD_INT, writedest, fromaddr);
        code->add_instruction(loadins);

        // writeint
        // writei vr1
        auto *writeins = new Instruction(HINS_WRITE_INT, writedest);
        code->add_instruction(writeins);

        reset_vreg();
    }

    void visit_assign(struct Node *ast) override {
        ASTVisitor::visit_assign(ast);

        // storeint into loaded addr
        // sti (vr0), vr1
        Node* lhs = node_get_kid(ast, 0);
        Node* rhs = node_get_kid(ast, 1);

        Operand r_vreg = rhs->get_operand();
        Operand valop(OPERAND_VREG, r_vreg.get_base_reg());

        Operand l_vreg = lhs->get_operand();
        Operand refop(OPERAND_VREG_MEMREF, l_vreg.get_base_reg());

        auto *storeins = new Instruction(HINS_STORE_INT, refop, valop);
        code->add_instruction(storeins);

        reset_vreg();
    }

    void visit_add(struct Node *ast) override {
        ASTVisitor::visit_add(ast);

        Node* lhs = node_get_kid(ast, 0);
        Node* rhs = node_get_kid(ast, 0);

        Operand l_op = lhs->get_operand();
        Operand r_op = rhs->get_operand();

        // ldi vr3, (vr1)
        long lreg = next_vreg();
        Operand ldest(OPERAND_VREG, lreg);
        Operand lfrom(OPERAND_VREG_MEMREF, l_op.get_base_reg());
        auto* lload = new Instruction(HINS_LOAD_INT, ldest, lfrom);
        code->add_instruction(lload);

        // ldi vr4, (vr2)
        long rreg = next_vreg();
        Operand rdest(OPERAND_VREG, rreg);
        Operand rfrom(OPERAND_VREG_MEMREF, r_op.get_base_reg());
        auto* rload = new Instruction(HINS_LOAD_INT, rdest, rfrom);
        code->add_instruction(rload);

        // addi vr5, vr3, vr4
        long result_reg = next_vreg();
        Operand adddest(OPERAND_VREG, result_reg);
        auto* divins = new Instruction(HINS_INT_ADD, adddest, ldest, rdest);
        code->add_instruction(divins);

        ast->set_operand(adddest);
    }

    void visit_subtract(struct Node *ast) override {
        ASTVisitor::visit_subtract(ast);

        Node* lhs = node_get_kid(ast, 0);
        Node* rhs = node_get_kid(ast, 0);

        Operand l_op = lhs->get_operand();
        Operand r_op = rhs->get_operand();

        // ldi vr3, (vr1)
        long lreg = next_vreg();
        Operand ldest(OPERAND_VREG, lreg);
        Operand lfrom(OPERAND_VREG_MEMREF, l_op.get_base_reg());
        auto* lload = new Instruction(HINS_LOAD_INT, ldest, lfrom);
        code->add_instruction(lload);

        // ldi vr4, (vr2)
        long rreg = next_vreg();
        Operand rdest(OPERAND_VREG, rreg);
        Operand rfrom(OPERAND_VREG_MEMREF, r_op.get_base_reg());
        auto* rload = new Instruction(HINS_LOAD_INT, rdest, rfrom);
        code->add_instruction(rload);

        // subi vr5, vr3, vr4
        long result_reg = next_vreg();
        Operand subdest(OPERAND_VREG, result_reg);
        auto* divins = new Instruction(HINS_INT_SUB, subdest, ldest, rdest);
        code->add_instruction(divins);

        ast->set_operand(subdest);
    }

    void visit_multiply(struct Node *ast) override {
        ASTVisitor::visit_multiply(ast);

        Node* lhs = node_get_kid(ast, 0);
        Node* rhs = node_get_kid(ast, 0);

        Operand l_op = lhs->get_operand();
        Operand r_op = rhs->get_operand();

        // ldi vr3, (vr1)
        long lreg = next_vreg();
        Operand ldest(OPERAND_VREG, lreg);
        Operand lfrom(OPERAND_VREG_MEMREF, l_op.get_base_reg());
        auto* lload = new Instruction(HINS_LOAD_INT, ldest, lfrom);
        code->add_instruction(lload);

        // ldi vr4, (vr2)
        long rreg = next_vreg();
        Operand rdest(OPERAND_VREG, rreg);
        Operand rfrom(OPERAND_VREG_MEMREF, r_op.get_base_reg());
        auto* rload = new Instruction(HINS_LOAD_INT, rdest, rfrom);
        code->add_instruction(rload);

        // muli vr5, vr3, vr4
        long result_reg = next_vreg();
        Operand muldest(OPERAND_VREG, result_reg);
        auto* divins = new Instruction(HINS_INT_MUL, muldest, ldest, rdest);
        code->add_instruction(divins);

        ast->set_operand(muldest);
    }

    void visit_divide(struct Node *ast) override {
        ASTVisitor::visit_divide(ast);

        Node* lhs = node_get_kid(ast, 0);
        Node* rhs = node_get_kid(ast, 0);

        Operand l_op = lhs->get_operand();
        Operand r_op = rhs->get_operand();

        // ldi vr3, (vr1)
        long lreg = next_vreg();
        Operand ldest(OPERAND_VREG, lreg);
        Operand lfrom(OPERAND_VREG_MEMREF, l_op.get_base_reg());
        auto* lload = new Instruction(HINS_LOAD_INT, ldest, lfrom);
        code->add_instruction(lload);

        // ldi vr4, (vr2)
        long rreg = next_vreg();
        Operand rdest(OPERAND_VREG, rreg);
        Operand rfrom(OPERAND_VREG_MEMREF, r_op.get_base_reg());
        auto* rload = new Instruction(HINS_LOAD_INT, rdest, rfrom);
        code->add_instruction(rload);

        // divi vr5, vr3, vr4
        long result_reg = next_vreg();
        Operand divdest(OPERAND_VREG, result_reg);
        auto* divins = new Instruction(HINS_INT_DIV, divdest, ldest, rdest);
        code->add_instruction(divins);

        ast->set_operand(divdest);
    }

    void visit_modulus(struct Node *ast) override {
        ASTVisitor::visit_modulus(ast);
        // TODO:
    }

    void visit_var_ref(struct Node *ast) override {
        ASTVisitor::visit_var_ref(ast);

        // set Operand on Node
        Node *identifier = node_get_kid(ast, 0);
        Operand op = identifier->get_operand();
        ast->set_operand(op);
    }

    void visit_identifier(struct Node *ast) override {
        ASTVisitor::visit_identifier(ast);

        // loadaddr lhs by offset
        // localaddr vr0, $8
        long vreg = next_vreg();
        Operand destreg(OPERAND_VREG, vreg);

        const char* varname = node_get_str(ast);
        // get offset from symbol
        // instruction is an offset ref
        Symbol sym = m_symtab.lookup(varname);
        int offset = sym.get_offset();
        Operand addroffset(OPERAND_INT_LITERAL, offset);

        auto *loadaddrins = new Instruction(HINS_LOCALADDR, destreg, addroffset);
        code->add_instruction(loadaddrins);

        // set Operand to Node
        ast->set_operand(destreg);

        // don't reset virtual registers
    }

    void visit_int_literal(struct Node *ast) override {
        ASTVisitor::visit_int_literal(ast);

        long vreg = next_vreg();
        Operand destreg(OPERAND_VREG, vreg);    // $vr0
        Operand immval(OPERAND_INT_LITERAL, ast->get_ival());   // $1
        auto *ins = new Instruction(HINS_LOAD_INT, destreg, immval);    // movq vr0, lit1
        code->add_instruction(ins);
        ast->set_operand(destreg);

        // do we need to reset virtual registers?
    }
};

class AssemblyCodeGen {
    // needs to know about storage requirements
    // needs to know highest virtual register value
    // needs HINS instruction sequence

private:
    InstructionSequence* assembly;
    InstructionSequence* hins;
    PrintHighLevelInstructionSequence* print_helper;
    long local_storage_size;
    long num_vreg;
    // total is just local_storage_size + (WORD_SIZE * num_vreg)

    // localaddr with $N means N offset of rsp
    // N(%rsp)

    // vrN means storage size + (N * 8);
    // N = storage_size + (N * WORD_SIZE)
    // N(%rsp)
public:
    AssemblyCodeGen(InstructionSequence* highlevelins) {
        hins = highlevelins;
        assembly = new InstructionSequence();
        print_helper = new PrintHighLevelInstructionSequence(nullptr);
    }

    InstructionSequence* get_assembly_ins() {
        return assembly;
    }

    void emit() {
        printf("reached assembly code gen");
    }

private:
    void emit_preamble() {
        std::string preamble =
                "\t.section .rodata\n"
                  "s_readint_fmt: .string \"%ld\"\n"
                  "s_writeint_fmt: .string \"%ld\\n\"\n"
                  "\t.section .text\n"
                  "\t.globl main\n"
                  "main:";
    }
    // subq storage + (8 * num_vreg), rsp

    void translate_instructions() {

    }

    void emit_asm() {
        PrintX86_64InstructionSequence print_asm(assembly);
        //print_asm.print();
    }

    // addq storage + (8 * num_vreg), rsp
    void emit_epilogue() {
        std::string ret_stmt =
                "movl $0, %eax\n"
                "\tret";
    }

    std::string get_hins_comment(Instruction* hin) {
        return print_helper->format_instruction(hin);
    }
};

////////////////////////////////////////////////////////////////////////
// Context class implementation
////////////////////////////////////////////////////////////////////////

Context::Context(struct Node *ast) {
    root = ast;
    global = new SymbolTable(nullptr);
    flag_print_symtab = false;
    flag_print_hins = false;
}

Context::~Context() {
}

void Context::set_flag(char flag) {
  if (flag == 's') {
      flag_print_symtab = true;
  }
  if (flag == 'h') {
      flag_print_hins = true;
  }
}

void Context::build_symtab() {

    // give symtabbuilder a symtab in constructor?
    SymbolTableBuilder *visitor = new SymbolTableBuilder(global);
    visitor->visit(root);

    if (flag_print_symtab) {
      // print symbol table
      visitor->get_symtab()->print_sym_tab();
    }
}

void Context::gen_code() {
    auto *hlcodegen = new HighLevelCodeGen(global);
    hlcodegen->visit(root);

    if (flag_print_hins) {
        auto *hlprinter = new PrintHighLevelInstructionSequence(hlcodegen->get_iseq());
        hlprinter->print();
    }

    auto *asmcodegen = new AssemblyCodeGen(hlcodegen->get_iseq());
    asmcodegen->emit();
}

// TODO: implementation of additional Context member functions

// TODO: implementation of member functions for helper classes

////////////////////////////////////////////////////////////////////////
// Context API functions
////////////////////////////////////////////////////////////////////////

struct Context *context_create(struct Node *ast) {
  return new Context(ast);
}

void context_destroy(struct Context *ctx) {
  delete ctx;
}

void context_set_flag(struct Context *ctx, char flag) {
  ctx->set_flag(flag);
}

void context_build_symtab(struct Context *ctx) {
  ctx->build_symtab();
}

void context_check_types(struct Context *ctx) {
}

void context_gen_code(struct Context *ctx) {
    ctx->gen_code();
}
