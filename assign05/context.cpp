#include <cassert>
#include <cstdio>
#include <cstdarg>
#include <cstdlib>
#include <cstring>
#include <string>
#include <map>
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
#include "cfg_transform.h"
#include "live_vregs.h"

////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////

struct Context {
private:
    Node *root;
    SymbolTable *global;
    bool flag_print_symtab;
    bool flag_print_hins;
    bool flag_optimize;
    bool flag_compile;

public:
  Context(struct Node *ast);
  ~Context();

  void set_flag(char flag);

  void build_symtab();
  void print_err(Node* node, const char *fmt, ...);

  void gen_code();
};

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

    long get_curr_offset() {
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
        //Node* right = node_get_kid(ast, 1);
        //Type* type = right->get_type();

        // Consts are always going to be INTEGER
        Type* type = integer_type;

        // get left identifier
        Node* left = node_get_kid(ast, 0);
        const char* name = node_get_str(left);

        // get right value
        Node *right = node_get_kid(ast, 1);
        if (!right->is_const()) {
            SourceInfo info = node_get_source_info(left);
            err_fatal("%s:%d:%d: Error: Non-constant in constant expression\n", info.filename, info.line, info.col);
        }
        long val = right->get_ival();

        // set entry in symtab for name, type
        long offset = get_curr_offset();
        Symbol* sym = symbol_create(name, type, CONST, offset);
        sym->set_ival(val);
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
            long offset = get_curr_offset();
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
        long offset = get_curr_offset();
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
        if (!left->is_const()) {
            SourceInfo info = node_get_source_info(left);
            err_fatal("%s:%d:%d: Error: Non-constant in constant expression\n", info.filename, info.line, info.col);
        }
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
        if (sym.get_kind() == CONST) {
            ast->set_is_const(true);
            ast->set_ival(sym.get_ival());
        }
    }

    void visit_int_literal(struct Node *ast) override {
        // set literal value
        ast->set_ival(strtol(node_get_str(ast), nullptr, 10));
        // set type to integer
        ast->set_type(integer_type);
        ast->set_is_const(true);
    }

    void visit_add(struct Node *ast) override {
        ASTVisitor::visit_add(ast);

        Node *left = node_get_kid(ast, 0);
        Node *right = node_get_kid(ast, 1);

        if (!left->is_const() || !right->is_const()) {
            // skip evaluation if calculation is not constant
            return;
        }

        long lval = left->get_ival();
        long rval = right->get_ival();

        ast->set_ival(lval + rval);
        ast->set_is_const(true);
    }

    void visit_subtract(struct Node *ast) override {
        ASTVisitor::visit_subtract(ast);

        Node *left = node_get_kid(ast, 0);
        Node *right = node_get_kid(ast, 1);

        if (!left->is_const() || !right->is_const()) {
            // skip evaluation if calculation is not constant
            return;
        }

        long lval = left->get_ival();
        long rval = right->get_ival();

        ast->set_ival(lval - rval);
        ast->set_is_const(true);
    }

    void visit_multiply(struct Node *ast) override {
        ASTVisitor::visit_multiply(ast);

        Node *left = node_get_kid(ast, 0);
        Node *right = node_get_kid(ast, 1);

        if (!left->is_const() || !right->is_const()) {
            // skip evaluation if calculation is not constant
            return;
        }

        long lval = left->get_ival();
        long rval = right->get_ival();

        ast->set_ival(lval * rval);
        ast->set_is_const(true);
    }

    void visit_divide(struct Node *ast) override {
        ASTVisitor::visit_divide(ast);

        Node *left = node_get_kid(ast, 0);
        Node *right = node_get_kid(ast, 1);

        if (!left->is_const() || !right->is_const()) {
            // skip evaluation if calculation is not constant
            return;
        }

        long lval = left->get_ival();
        long rval = right->get_ival();

        ast->set_ival(lval / rval);
        ast->set_is_const(true);
    }

    void visit_modulus(struct Node *ast) override {
        ASTVisitor::visit_modulus(ast);

        Node *left = node_get_kid(ast, 0);
        Node *right = node_get_kid(ast, 1);

        if (!left->is_const() || !right->is_const()) {
            // skip evaluation if calculation is not constant
            return;
        }

        long lval = left->get_ival();
        long rval = right->get_ival();

        ast->set_ival(lval % rval);
        ast->set_is_const(true);
    }
};

class HighLevelCodeGen : public ASTVisitor {

private:
    long m_vreg = -1;
    long m_vreg_max = -1;
    long loop_index = 0;
    long initial_vreg = -1;
    SymbolTable* m_symtab;
    std::map<std::string, Operand> scalars;
    InstructionSequence* code;

public:
    HighLevelCodeGen(SymbolTable* symbolTable)
        : m_symtab(symbolTable),
        scalars() {
        code = new InstructionSequence();
    }

    InstructionSequence* get_iseq() {
        return code;
    }

    long get_storage_size() {
        return m_symtab->get_total_size();
    }

    long get_vreg_max() {
        // if N is the index of vreg used, e.g. vrN, then the number of registers is N + 1
        return m_vreg_max + 1;
    }

private:
    long next_vreg() {
        m_vreg += 1;
        if (m_vreg_max < (m_vreg)) {
            m_vreg_max = m_vreg;
        }
        return m_vreg;
    }

    void set_initial_vreg(long initial) {
        initial_vreg = initial;
    }

    void reset_vreg() {
        m_vreg = initial_vreg;
    }

    std::string next_label() {
        std::string label = cpputil::format(".L%ld", loop_index);
        loop_index++;
        return label;
    }

public:

    void visit_declarations(struct Node *ast) override {
        for (auto symbol : m_symtab->get_symbols()) {
            if (symbol.get_kind() == VARIABLE && symbol.get_type()->realType == PRIMITIVE) {
                // this is a scalar variable
                long next = next_vreg();
                Operand scalar_vreg(OPERAND_VREG, next);
                scalar_vreg.set_is_scalar(true);
                scalars[symbol.get_name()] = scalar_vreg;
            }
        }

        set_initial_vreg(scalars.size() - 1);

        // specifically disable visits to declarations so no vregs are incremented
        //ASTVisitor::visit_declarations(ast);
        reset_vreg();
    }

    void visit_if(struct Node *ast) override {
        Node *cond = ast->get_kid(0);
        Node *iftrue = ast->get_kid(1);

        std::string out_label = next_label();

        cond->set_inverted(true);
        Operand op_out(out_label);
        cond->set_operand(op_out);

        visit(cond);
        visit(iftrue);
        code->define_label(out_label);
    }

    void visit_if_else(struct Node *ast) override {
        Node *condition = node_get_kid(ast, 0);
        Node *iftrue = node_get_kid(ast, 1);
        Node *otherwise = node_get_kid(ast, 2);

        std::string else_label = next_label();
        std::string out_label = next_label();

        condition->set_inverted(true);
        Operand op_else(else_label);
        condition->set_operand(op_else);

        visit(condition);
        visit(iftrue);
        Operand op_out(out_label);
        auto *jumpins = new Instruction(HINS_JUMP, op_out);  // jump after iftrue to skip else
        code->add_instruction(jumpins);
        code->define_label(else_label);
        visit(otherwise);
        code->define_label(out_label);

        // add no-op to resolve define_label assertion error
        auto *noopins = new Instruction(HINS_NOP);
        code->add_instruction(noopins);
    }

    void visit_repeat(struct Node *ast) override {
        Node *instructions = node_get_kid(ast, 0);
        Node *condition = node_get_kid(ast, 1);

        std::string loop_body_label = next_label();         // .L0
        std::string loop_condition_label = next_label();    // .L1

        Operand op_loop_body(loop_body_label);
        Operand op_loop_condition(loop_condition_label);

        // no need to jump, will flow right into loop body for first loop iteration

        code->define_label(loop_body_label);
        visit(instructions);

        code->define_label(loop_condition_label);
        condition->set_inverted(true);
        condition->set_operand(op_loop_body);
        visit(condition);
    }

    void visit_while(struct Node *ast) override {
        Node *condition = node_get_kid(ast, 0);
        Node *instructions = node_get_kid(ast, 1);

        std::string loop_body_label = next_label();         // .L0
        std::string loop_condition_label = next_label();    // .L1

        Operand op_loop_condition(loop_condition_label);
        auto *jumpins = new Instruction(HINS_JUMP, op_loop_condition);
        code->add_instruction(jumpins);

        // loop body
        Operand op_loop_body(loop_body_label);
        code->define_label(loop_body_label);
        visit(instructions);

        // loop condition
        code->define_label(loop_condition_label);
        condition->set_operand(op_loop_body);
        visit(condition);
    }

    void visit_compare_eq(struct Node *ast) override {
        ASTVisitor::visit_compare_eq(ast);

        Node *lhs = node_get_kid(ast, 0);
        Node *rhs = node_get_kid(ast, 1);

        Operand l_op = lhs->get_operand();
        Operand r_op = rhs->get_operand();

        int tag = node_get_tag(lhs);
        if (l_op.get_is_scalar()) {
            // just use l_op directly
        } else if (tag == AST_VAR_REF || tag == AST_ARRAY_ELEMENT_REF) {
            // ldi vr3, (vr1)
            long lreg = next_vreg();
            Operand ldest(OPERAND_VREG, lreg);
            Operand lfrom(OPERAND_VREG_MEMREF, l_op.get_base_reg());
            auto *lload = new Instruction(HINS_LOAD_INT, ldest, lfrom);
            l_op = ldest;
            code->add_instruction(lload);
        }

        if (!rhs->is_const()) {
            tag = node_get_tag(rhs);
            if (r_op.get_is_scalar()) {
                // just use r_op directly
            } else if (tag == AST_VAR_REF || tag == AST_ARRAY_ELEMENT_REF) {
                // ldi vr4, (vr2)
                long rreg = next_vreg();
                Operand rdest(OPERAND_VREG, rreg);
                Operand rfrom(OPERAND_VREG_MEMREF, r_op.get_base_reg());
                auto *rload = new Instruction(HINS_LOAD_INT, rdest, rfrom);
                r_op = rdest;
                code->add_instruction(rload);
            }
        }

        auto *cmpins = new Instruction(HINS_INT_COMPARE, l_op, r_op);
        code->add_instruction(cmpins);

        Instruction *jumpins;
        if (ast->is_inverted()) {
            jumpins = new Instruction(HINS_JNE, ast->get_operand());
        } else {
            jumpins = new Instruction(HINS_JE, ast->get_operand());
        }
        code->add_instruction(jumpins);
    }

    void visit_compare_neq(struct Node *ast) override {
        ASTVisitor::visit_compare_neq(ast);

        Node *lhs = node_get_kid(ast, 0);
        Node *rhs = node_get_kid(ast, 1);

        Operand l_op = lhs->get_operand();
        Operand r_op = rhs->get_operand();

        int tag = node_get_tag(lhs);
        if (l_op.get_is_scalar()) {
            // just use l_op directly
        } else if (tag == AST_VAR_REF || tag == AST_ARRAY_ELEMENT_REF) {
            // ldi vr3, (vr1)
            long lreg = next_vreg();
            Operand ldest(OPERAND_VREG, lreg);
            Operand lfrom(OPERAND_VREG_MEMREF, l_op.get_base_reg());
            auto* lload = new Instruction(HINS_LOAD_INT, ldest, lfrom);
            l_op = ldest;
            code->add_instruction(lload);
        }

        if (!rhs->is_const()) {
            tag = node_get_tag(rhs);
            if (r_op.get_is_scalar()) {
                // just use r_op directly
            } else if (tag == AST_VAR_REF || tag == AST_ARRAY_ELEMENT_REF) {
                // ldi vr4, (vr2)
                long rreg = next_vreg();
                Operand rdest(OPERAND_VREG, rreg);
                Operand rfrom(OPERAND_VREG_MEMREF, r_op.get_base_reg());
                auto *rload = new Instruction(HINS_LOAD_INT, rdest, rfrom);
                r_op = rdest;
                code->add_instruction(rload);
            }
        }

        auto *cmpins = new Instruction(HINS_INT_COMPARE, l_op, r_op);
        code->add_instruction(cmpins);

        Instruction *jumpins;
        if (ast->is_inverted()) {
            jumpins = new Instruction(HINS_JE, ast->get_operand());
        } else {
            jumpins = new Instruction(HINS_JNE, ast->get_operand());
        }
        code->add_instruction(jumpins);
    }

    void visit_compare_lt(struct Node *ast) override {
        ASTVisitor::visit_compare_lt(ast);

        Node *lhs = node_get_kid(ast, 0);
        Node *rhs = node_get_kid(ast, 1);

        Operand l_op = lhs->get_operand();
        Operand r_op = rhs->get_operand();

        int tag = node_get_tag(lhs);
        if (l_op.get_is_scalar()) {
            // just use l_op directly
        } else if (tag == AST_VAR_REF || tag == AST_ARRAY_ELEMENT_REF) {
            // ldi vr3, (vr1)
            long lreg = next_vreg();
            Operand ldest(OPERAND_VREG, lreg);
            Operand lfrom(OPERAND_VREG_MEMREF, l_op.get_base_reg());
            auto* lload = new Instruction(HINS_LOAD_INT, ldest, lfrom);
            l_op = ldest;
            code->add_instruction(lload);
        }

        if (!rhs->is_const()) {
            tag = node_get_tag(rhs);
            if (r_op.get_is_scalar()) {
                // just use r_op directly
            } else if (tag == AST_VAR_REF || tag == AST_ARRAY_ELEMENT_REF) {
                // ldi vr4, (vr2)
                long rreg = next_vreg();
                Operand rdest(OPERAND_VREG, rreg);
                Operand rfrom(OPERAND_VREG_MEMREF, r_op.get_base_reg());
                auto *rload = new Instruction(HINS_LOAD_INT, rdest, rfrom);
                r_op = rdest;
                code->add_instruction(rload);
            }
        }

        auto *cmpins = new Instruction(HINS_INT_COMPARE, l_op, r_op);
        code->add_instruction(cmpins);

        Instruction *jumpins;
        if (ast->is_inverted()) {
            jumpins = new Instruction(HINS_JGTE, ast->get_operand());
        } else {
            jumpins = new Instruction(HINS_JLT, ast->get_operand());
        }
        code->add_instruction(jumpins);
    }

    void visit_compare_lte(struct Node *ast) override {
        ASTVisitor::visit_compare_lte(ast);

        Node *lhs = node_get_kid(ast, 0);
        Node *rhs = node_get_kid(ast, 1);

        Operand l_op = lhs->get_operand();
        Operand r_op = rhs->get_operand();

        int tag = node_get_tag(lhs);
        if (l_op.get_is_scalar()) {
            // just use l_op directly
        } else if (tag == AST_VAR_REF || tag == AST_ARRAY_ELEMENT_REF) {
            // ldi vr3, (vr1)
            long lreg = next_vreg();
            Operand ldest(OPERAND_VREG, lreg);
            Operand lfrom(OPERAND_VREG_MEMREF, l_op.get_base_reg());
            auto* lload = new Instruction(HINS_LOAD_INT, ldest, lfrom);
            l_op = ldest;
            code->add_instruction(lload);
        }

        if (!rhs->is_const()) {
            tag = node_get_tag(rhs);
            if (r_op.get_is_scalar()) {
                // just use r_op directly
            } else if (tag == AST_VAR_REF || tag == AST_ARRAY_ELEMENT_REF) {
                // ldi vr4, (vr2)
                long rreg = next_vreg();
                Operand rdest(OPERAND_VREG, rreg);
                Operand rfrom(OPERAND_VREG_MEMREF, r_op.get_base_reg());
                auto *rload = new Instruction(HINS_LOAD_INT, rdest, rfrom);
                r_op = rdest;
                code->add_instruction(rload);
            }
        }

        auto *cmpins = new Instruction(HINS_INT_COMPARE, l_op, r_op);
        code->add_instruction(cmpins);

        Instruction *jumpins;
        if (ast->is_inverted()) {
            jumpins = new Instruction(HINS_JGT, ast->get_operand());
        } else {
            jumpins = new Instruction(HINS_JLTE, ast->get_operand());
        }
        code->add_instruction(jumpins);
    }

    void visit_compare_gt(struct Node *ast) override {
        ASTVisitor::visit_compare_gt(ast);

        Node *lhs = node_get_kid(ast, 0);
        Node *rhs = node_get_kid(ast, 1);

        Operand l_op = lhs->get_operand();
        Operand r_op = rhs->get_operand();

        int tag = node_get_tag(lhs);
        if (l_op.get_is_scalar()) {
            // just use l_op directly
        } else if (tag == AST_VAR_REF || tag == AST_ARRAY_ELEMENT_REF) {
            // ldi vr3, (vr1)
            long lreg = next_vreg();
            Operand ldest(OPERAND_VREG, lreg);
            Operand lfrom(OPERAND_VREG_MEMREF, l_op.get_base_reg());
            auto* lload = new Instruction(HINS_LOAD_INT, ldest, lfrom);
            l_op = ldest;
            code->add_instruction(lload);
        }

        if (!rhs->is_const()) {
            tag = node_get_tag(rhs);
            if (r_op.get_is_scalar()) {
                // just use r_op directly
            } else if (tag == AST_VAR_REF || tag == AST_ARRAY_ELEMENT_REF) {
                // ldi vr4, (vr2)
                long rreg = next_vreg();
                Operand rdest(OPERAND_VREG, rreg);
                Operand rfrom(OPERAND_VREG_MEMREF, r_op.get_base_reg());
                auto *rload = new Instruction(HINS_LOAD_INT, rdest, rfrom);
                r_op = rdest;
                code->add_instruction(rload);
            }
        }

        auto *cmpins = new Instruction(HINS_INT_COMPARE, l_op, r_op);
        code->add_instruction(cmpins);

        Instruction *jumpins;
        if (ast->is_inverted()) {
            jumpins = new Instruction(HINS_JLTE, ast->get_operand());
        } else {
            jumpins = new Instruction(HINS_JGT, ast->get_operand());
        }
        code->add_instruction(jumpins);
    }

    void visit_compare_gte(struct Node *ast) override {
        ASTVisitor::visit_compare_gte(ast);

        Node *lhs = node_get_kid(ast, 0);
        Node *rhs = node_get_kid(ast, 1);

        Operand l_op = lhs->get_operand();
        Operand r_op = rhs->get_operand();

        int tag = node_get_tag(lhs);
        if (l_op.get_is_scalar()) {
            // just use l_op directly
        } else if (tag == AST_VAR_REF || tag == AST_ARRAY_ELEMENT_REF) {
            // ldi vr3, (vr1)
            long lreg = next_vreg();
            Operand ldest(OPERAND_VREG, lreg);
            Operand lfrom(OPERAND_VREG_MEMREF, l_op.get_base_reg());
            auto* lload = new Instruction(HINS_LOAD_INT, ldest, lfrom);
            l_op = ldest;
            code->add_instruction(lload);
        }

        if (!rhs->is_const()) {
            tag = node_get_tag(rhs);
            if (r_op.get_is_scalar()) {
                // just use r_op directly
            } else if (tag == AST_VAR_REF || tag == AST_ARRAY_ELEMENT_REF) {
                // ldi vr4, (vr2)
                long rreg = next_vreg();
                Operand rdest(OPERAND_VREG, rreg);
                Operand rfrom(OPERAND_VREG_MEMREF, r_op.get_base_reg());
                auto *rload = new Instruction(HINS_LOAD_INT, rdest, rfrom);
                r_op = rdest;
                code->add_instruction(rload);
            }
        }

        auto *cmpins = new Instruction(HINS_INT_COMPARE, l_op, r_op);
        code->add_instruction(cmpins);

        Instruction *jumpins;
        if (ast->is_inverted()) {
            jumpins = new Instruction(HINS_JLT, ast->get_operand());
        } else {
            jumpins = new Instruction(HINS_JGTE, ast->get_operand());
        }
        code->add_instruction(jumpins);
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
        if (destreg.get_is_scalar()) {
            auto *movins = new Instruction(HINS_MOV, destreg, readdest);
            code->add_instruction(movins);
        } else {
            Operand toaddr(OPERAND_VREG_MEMREF, destreg.get_base_reg());   // use this one
            auto *storeins = new Instruction(HINS_STORE_INT, toaddr, readdest);
            code->add_instruction(storeins);
        }

        reset_vreg();
    }

    void visit_write(struct Node *ast) override {
        ASTVisitor::visit_write(ast);

        Node* kid = node_get_kid(ast, 0);
        Operand op = kid->get_operand();

        int tag = node_get_tag(kid);
        if (op.get_is_scalar()) {
            // just use op directly
        } else if (tag == AST_VAR_REF || tag == AST_ARRAY_ELEMENT_REF) {
            // loadint from addr to vreg
            // ldi vr1, (vr0)
            long toreg = next_vreg();
            Operand writedest(OPERAND_VREG, toreg);
            op = writedest;
            Operand fromreg = kid->get_operand();    // don't use this one
            Operand fromaddr(OPERAND_VREG_MEMREF, fromreg.get_base_reg()); // use this one
            auto *loadins = new Instruction(HINS_LOAD_INT, writedest, fromaddr);
            code->add_instruction(loadins);
        }

        // writeint
        // writei vr1
        auto *writeins = new Instruction(HINS_WRITE_INT, op);
        code->add_instruction(writeins);

        reset_vreg();
    }

    void visit_assign(struct Node *ast) override {
        ASTVisitor::visit_assign(ast);

        // storeint into loaded addr
        // sti (vr0), vr1
        Node* lhs = node_get_kid(ast, 0);
        Node* rhs = node_get_kid(ast, 1);

        Operand valop = rhs->get_operand();

        if (!rhs->is_const()) {
            int tag = node_get_tag(rhs);
            if (valop.get_is_scalar()) {
                // do nothing
            } else if (tag == AST_VAR_REF || tag == AST_ARRAY_ELEMENT_REF) {
                long vreg = next_vreg();
                Operand loaddest(OPERAND_VREG, vreg);
                auto *loadins = new Instruction(HINS_LOAD_INT, loaddest, valop.to_memref());
                code->add_instruction(loadins);
                valop = loaddest;
            }
        }

        Operand l_vreg = lhs->get_operand();

        if (node_get_tag(lhs) == AST_VAR_REF || node_get_tag(lhs) == AST_ARRAY_ELEMENT_REF) {
            if (l_vreg.get_is_scalar()) {
                auto *movins = new Instruction(HINS_MOV, l_vreg, valop);
                code->add_instruction(movins);
            } else {
                Operand refop(OPERAND_VREG_MEMREF, l_vreg.get_base_reg());
                auto *storeins = new Instruction(HINS_STORE_INT, refop, valop);
                code->add_instruction(storeins);
            }
        } else {
            auto *movins = new Instruction(HINS_MOV, l_vreg, valop);
            code->add_instruction(movins);
        }

        reset_vreg();
    }

    void visit_add(struct Node *ast) override {
        ASTVisitor::visit_add(ast);

        Node* lhs = node_get_kid(ast, 0);
        Node* rhs = node_get_kid(ast, 1);

        Operand l_op = lhs->get_operand();
        Operand r_op = rhs->get_operand();

        int tag = node_get_tag(lhs);
        if (l_op.get_is_scalar()) {
            // just use l_op directly
        } else if (tag == AST_VAR_REF || tag == AST_ARRAY_ELEMENT_REF) {
            // ldi vr3, (vr1)
            long lreg = next_vreg();
            Operand ldest(OPERAND_VREG, lreg);
            Operand lfrom(OPERAND_VREG_MEMREF, l_op.get_base_reg());
            auto* lload = new Instruction(HINS_LOAD_INT, ldest, lfrom);
            l_op = ldest;
            code->add_instruction(lload);
        }

        if (!rhs->is_const()) {
            tag = node_get_tag(rhs);
            if (r_op.get_is_scalar()) {
                // just use l_op directly
            } else if (tag == AST_VAR_REF || tag == AST_ARRAY_ELEMENT_REF) {
                // ldi vr4, (vr2)
                long rreg = next_vreg();
                Operand rdest(OPERAND_VREG, rreg);
                Operand rfrom(OPERAND_VREG_MEMREF, r_op.get_base_reg());
                auto *rload = new Instruction(HINS_LOAD_INT, rdest, rfrom);
                r_op = rdest;
                code->add_instruction(rload);
            }
        }

        // addi vr5, vr3, vr4
        long result_reg = next_vreg();
        Operand adddest(OPERAND_VREG, result_reg);
        auto* divins = new Instruction(HINS_INT_ADD, adddest, l_op, r_op);
        code->add_instruction(divins);

        ast->set_operand(adddest);
    }

    void visit_subtract(struct Node *ast) override {
        ASTVisitor::visit_subtract(ast);

        Node* lhs = node_get_kid(ast, 0);
        Node* rhs = node_get_kid(ast, 1);

        Operand l_op = lhs->get_operand();
        Operand r_op = rhs->get_operand();

        // ldi vr3, (vr1)
        int tag = node_get_tag(lhs);
        if (l_op.get_is_scalar()) {
            // just use l_op directly
        } else if (tag == AST_VAR_REF || tag == AST_ARRAY_ELEMENT_REF) {
            long lreg = next_vreg();
            Operand ldest(OPERAND_VREG, lreg);
            Operand lfrom(OPERAND_VREG_MEMREF, l_op.get_base_reg());
            auto *lload = new Instruction(HINS_LOAD_INT, ldest, lfrom);
            l_op = ldest;
            code->add_instruction(lload);
        }

        if (!rhs->is_const()) {
            tag = node_get_tag(rhs);
            if (r_op.get_is_scalar()) {
                // just use l_op directly
            } else if (tag == AST_VAR_REF || tag == AST_ARRAY_ELEMENT_REF) {
                // ldi vr4, (vr2)
                long rreg = next_vreg();
                Operand rdest(OPERAND_VREG, rreg);
                Operand rfrom(OPERAND_VREG_MEMREF, r_op.get_base_reg());
                auto *rload = new Instruction(HINS_LOAD_INT, rdest, rfrom);
                r_op = rdest;
                code->add_instruction(rload);
            }
        }

        // subi vr5, vr3, vr4
        long result_reg = next_vreg();
        Operand subdest(OPERAND_VREG, result_reg);
        auto* divins = new Instruction(HINS_INT_SUB, subdest, l_op, r_op);
        code->add_instruction(divins);

        ast->set_operand(subdest);
    }

    void visit_multiply(struct Node *ast) override {
        ASTVisitor::visit_multiply(ast);

        Node* lhs = node_get_kid(ast, 0);
        Node* rhs = node_get_kid(ast, 1);

        Operand l_op = lhs->get_operand();
        Operand r_op = rhs->get_operand();

        int tag = node_get_tag(lhs);
        if (l_op.get_is_scalar()) {
            // just use l_op directly
        } else if (tag == AST_VAR_REF || tag == AST_ARRAY_ELEMENT_REF) {
            // ldi vr3, (vr1)
            long lreg = next_vreg();
            Operand ldest(OPERAND_VREG, lreg);
            Operand lfrom(OPERAND_VREG_MEMREF, l_op.get_base_reg());
            auto *lload = new Instruction(HINS_LOAD_INT, ldest, lfrom);
            l_op = ldest;
            code->add_instruction(lload);
        }

        if (!rhs->is_const()) {
            tag = node_get_tag(rhs);
            if (r_op.get_is_scalar()) {
                // just use l_op directly
            } else if (tag == AST_VAR_REF || tag == AST_ARRAY_ELEMENT_REF) {
                // ldi vr4, (vr2)
                long rreg = next_vreg();
                Operand rdest(OPERAND_VREG, rreg);
                Operand rfrom(OPERAND_VREG_MEMREF, r_op.get_base_reg());
                auto *rload = new Instruction(HINS_LOAD_INT, rdest, rfrom);
                r_op = rdest;
                code->add_instruction(rload);
            }
        }

        // muli vr5, vr3, vr4
        long result_reg = next_vreg();
        Operand muldest(OPERAND_VREG, result_reg);
        auto* divins = new Instruction(HINS_INT_MUL, muldest, l_op, r_op);
        code->add_instruction(divins);

        ast->set_operand(muldest);
    }

    void visit_divide(struct Node *ast) override {
        ASTVisitor::visit_divide(ast);

        Node* lhs = node_get_kid(ast, 0);
        Node* rhs = node_get_kid(ast, 1);

        Operand l_op = lhs->get_operand();
        Operand r_op = rhs->get_operand();

        int tag = node_get_tag(lhs);
        if (l_op.get_is_scalar()) {
            // just use l_op directly
        } else if (tag == AST_VAR_REF || tag == AST_ARRAY_ELEMENT_REF) {
            // ldi vr3, (vr1)
            long lreg = next_vreg();
            Operand ldest(OPERAND_VREG, lreg);
            Operand lfrom(OPERAND_VREG_MEMREF, l_op.get_base_reg());
            auto *lload = new Instruction(HINS_LOAD_INT, ldest, lfrom);
            l_op = ldest;
            code->add_instruction(lload);
        }

        if (!rhs->is_const()) {
            tag = node_get_tag(rhs);
            if (r_op.get_is_scalar()) {
                // just use l_op directly
            } else if (tag == AST_VAR_REF || tag == AST_ARRAY_ELEMENT_REF) {
                // ldi vr4, (vr2)
                long rreg = next_vreg();
                Operand rdest(OPERAND_VREG, rreg);
                Operand rfrom(OPERAND_VREG_MEMREF, r_op.get_base_reg());
                auto *rload = new Instruction(HINS_LOAD_INT, rdest, rfrom);
                r_op = rdest;
                code->add_instruction(rload);
            }
        }

        // divi vr5, vr3, vr4
        long result_reg = next_vreg();
        Operand divdest(OPERAND_VREG, result_reg);
        auto* divins = new Instruction(HINS_INT_DIV, divdest, l_op, r_op);
        code->add_instruction(divins);

        ast->set_operand(divdest);
    }

    void visit_modulus(struct Node *ast) override {
        ASTVisitor::visit_modulus(ast);

        Node* lhs = node_get_kid(ast, 0);
        Node* rhs = node_get_kid(ast, 1);

        Operand l_op = lhs->get_operand();
        Operand r_op = rhs->get_operand();

        int tag = node_get_tag(lhs);
        if (l_op.get_is_scalar()) {
            // just use l_op directly
        } else if (tag == AST_VAR_REF || tag == AST_ARRAY_ELEMENT_REF) {
            // ldi vr3, (vr1)
            long lreg = next_vreg();
            Operand ldest(OPERAND_VREG, lreg);
            Operand lfrom(OPERAND_VREG_MEMREF, l_op.get_base_reg());
            auto *lload = new Instruction(HINS_LOAD_INT, ldest, lfrom);
            l_op = ldest;
            code->add_instruction(lload);
        }

        if (!rhs->is_const()) {
            tag = node_get_tag(rhs);
            if (r_op.get_is_scalar()) {
                // just use l_op directly
            } else if (tag == AST_VAR_REF || tag == AST_ARRAY_ELEMENT_REF) {
                // ldi vr4, (vr2)
                long rreg = next_vreg();
                Operand rdest(OPERAND_VREG, rreg);
                Operand rfrom(OPERAND_VREG_MEMREF, r_op.get_base_reg());
                auto *rload = new Instruction(HINS_LOAD_INT, rdest, rfrom);
                r_op = rdest;
                code->add_instruction(rload);
            }
        }

        // divi vr5, vr3, vr4
        long result_reg = next_vreg();
        Operand moddest(OPERAND_VREG, result_reg);
        auto* modins = new Instruction(HINS_INT_MOD, moddest, l_op, r_op);
        code->add_instruction(modins);

        ast->set_operand(moddest);
    }

    void visit_array_element_ref(struct Node *ast) override {
        ASTVisitor::visit_array_element_ref(ast);

        // load array start addr into vr0
        // load array accessor into into vr1
        // multiply

        // vr0 = (arr)
        // vr0 is address of arr start
        // vr1 = $index
        // vr2 = vr1 * element_size
        // vr3 = vr0 + vr2

        Node *identifier = node_get_kid(ast, 0);
        Operand arr_start = identifier->get_operand();

        Node *index = node_get_kid(ast, 1);
        Operand index_op = index->get_operand();

        if (index_op.get_is_scalar()) {
            // do nothing
        } else if (node_get_tag(index) == AST_VAR_REF) {   // dereference any identifiers passed into index
            index_op = index_op.to_memref();
        }   // otherwise, the index immediate is safe to use

        const char* varname = node_get_str(identifier);
        Type *array_type = m_symtab->lookup(varname).get_type();
        Type *element_type = array_type->arrayElementType;
        Operand element_size(OPERAND_INT_LITERAL, element_type->get_size());

        long next = next_vreg();
        Operand offset_reg(OPERAND_VREG, next);
        auto *mulins = new Instruction(HINS_INT_MUL, offset_reg, index_op, element_size);
        code->add_instruction(mulins);

        // result reg now contains offset from array start
        // add the address and offset to get address of (arr[index])
        next = next_vreg();
        Operand arr_addr_reg(OPERAND_VREG, next);
        auto *addins = new Instruction(HINS_INT_ADD, arr_addr_reg, arr_start, offset_reg);
        code->add_instruction(addins);
        ast->set_operand(arr_addr_reg);
    }

    void visit_var_ref(struct Node *ast) override {
        ASTVisitor::visit_var_ref(ast);

        // set Operand on Node
        Node *identifier = node_get_kid(ast, 0);
        ast->set_str(node_get_str(identifier));
        Operand op = identifier->get_operand();
        ast->set_operand(op);
    }

    void visit_identifier(struct Node *ast) override {
        ASTVisitor::visit_identifier(ast);

        const char* varname = node_get_str(ast);

        auto it = scalars.find(varname);
        if (it != scalars.end()) {
            Operand scalar_vreg = it->second;
            ast->set_operand(scalar_vreg);
            return;
        }

        // loadaddr lhs by offset
        // localaddr vr0, $8
        long vreg = next_vreg();
        Operand destreg(OPERAND_VREG, vreg);

        // get offset from symbol
        // instruction is an offset ref
        Symbol sym = m_symtab->lookup(varname);

        if (sym.get_kind() == CONST) {
            long value = sym.get_ival();
            Operand constval(OPERAND_INT_LITERAL, value);

            auto *loadins = new Instruction(HINS_LOAD_ICONST, destreg, constval);
            code->add_instruction(loadins);
        } else {
            long offset = sym.get_offset();
            Operand addroffset(OPERAND_INT_LITERAL, offset);

            auto *loadaddrins = new Instruction(HINS_LOCALADDR, destreg, addroffset);
            code->add_instruction(loadaddrins);
        }

        // set Operand to Node
        ast->set_operand(destreg);

        // don't reset virtual registers
    }

    void visit_int_literal(struct Node *ast) override {
        ASTVisitor::visit_int_literal(ast);

        long vreg = next_vreg();
        Operand destreg(OPERAND_VREG, vreg);    // $vr0
        Operand immval(OPERAND_INT_LITERAL, ast->get_ival());   // $1
        auto *ins = new Instruction(HINS_LOAD_ICONST, destreg, immval);
        code->add_instruction(ins);
        ast->set_operand(destreg);
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
    const long WORD_SIZE = 8;
    long local_storage_size;
    long num_vreg;
    long total_storage_size;
    // total is just local_storage_size + (WORD_SIZE * num_vreg)

    // localaddr with $N means N offset of rsp
    // N(%rsp)

    // vrN means storage size + (N * 8);
    // N = storage_size + (N * WORD_SIZE)
    // N(%rsp)
public:
    AssemblyCodeGen(InstructionSequence* highlevelins, long storage_size, long vreg_max) {
        hins = highlevelins;
        local_storage_size = storage_size;
        num_vreg = vreg_max;

        // calculate total storage
        total_storage_size = local_storage_size + (num_vreg * WORD_SIZE);
        // stack alignment check
        if (total_storage_size % 16 != 0) {
            total_storage_size += 8;
        }
        assembly = new InstructionSequence();
        print_helper = new PrintHighLevelInstructionSequence(nullptr);
    }

    void translate_instructions() {
        // callee-owned
        Operand rsp(OPERAND_MREG, MREG_RSP);
        Operand rdi(OPERAND_MREG, MREG_RDI);
        Operand rsi(OPERAND_MREG, MREG_RSI);
        Operand r10(OPERAND_MREG, MREG_R10);
        Operand r11(OPERAND_MREG, MREG_R11);
        Operand rax(OPERAND_MREG, MREG_RAX);
        Operand rdx(OPERAND_MREG, MREG_RDX);

        // static labels
        Operand inputfmt("s_readint_fmt", true);
        Operand outputfmt("s_writeint_fmt", true);
        Operand printf_label("printf");
        Operand scanf_label("scanf");

        const long num_ins = hins->get_length();
        for (int i = 0; i < num_ins; i++) {
            auto *hin = hins->get_instruction(i);

            if (hins->has_label(i)) {
                std::string label = hins->get_label(i);
                assembly->define_label(label);
            }

            switch(hin->get_opcode()) {
                case HINS_LOCALADDR: {
                    Operand rhs = hin->get_operand(1); // offset is rhs
                    Operand locaddr(OPERAND_MREG_MEMREF_OFFSET, MREG_RSP, rhs.get_int_value());
                    auto *leaq = new Instruction(MINS_LEAQ, locaddr, r10);
                    leaq->set_comment(get_hins_comment(hin));
                    assembly->add_instruction(leaq);

                    // vrN is offset 8N(rsp);
                    Operand lhs = hin->get_operand(0);
                    Operand dest = get_mreg(lhs);
                    auto *movq = new Instruction(MINS_MOVQ, r10, dest);
                    assembly->add_instruction(movq);
                    break;
                }
                case HINS_LOAD_INT:{
                    Operand rhs = hin->get_operand(1);
                    Operand loadsrc = get_mreg_or_lit(rhs);

                    Operand lhs = hin->get_operand(0);
                    Operand loaddest = get_mreg(lhs);

                    auto *mov1 = new Instruction(MINS_MOVQ, loadsrc, r11);
                    mov1->set_comment(get_hins_comment(hin));
                    assembly->add_instruction(mov1);

                    if (rhs.get_kind() != OPERAND_INT_LITERAL) {
                        Operand r11memref(OPERAND_MREG_MEMREF, MREG_R11);
                        auto *mov2 = new Instruction(MINS_MOVQ, r11memref, r11);
                        assembly->add_instruction(mov2);
                    }

                    auto *mov3 = new Instruction(MINS_MOVQ, r11, loaddest);
                    assembly->add_instruction(mov3);
                    break;
                }
                case HINS_LOAD_ICONST: {
                    Operand vreg = hin->get_operand(0);
                    Operand lit = hin->get_operand(1);

                    Operand dest = get_mreg(vreg);
                    auto *movins = new Instruction(MINS_MOVQ, lit, dest);
                    movins->set_comment(get_hins_comment(hin));
                    assembly->add_instruction(movins);
                    break;
                }
                case HINS_STORE_INT: {
                    Operand rhs = hin->get_operand(1);
                    Operand src = get_mreg_or_lit(rhs);

                    Operand lhs = hin->get_operand(0);
                    Operand dest = get_mreg(lhs);

                    auto *mov1 = new Instruction(MINS_MOVQ, src, r11);
                    mov1->set_comment(get_hins_comment(hin));
                    assembly->add_instruction(mov1);

                    auto *mov2 = new Instruction(MINS_MOVQ, dest, r10);
                    assembly->add_instruction(mov2);

                    Operand memrefdest(OPERAND_MREG_MEMREF, MREG_R10);
                    auto *mov3 = new Instruction(MINS_MOVQ, r11, memrefdest);
                    assembly->add_instruction(mov3);
                    break;
                }
                case HINS_MOV: {
                    Operand rhs = hin->get_operand(1);
                    Operand src = get_mreg_or_lit(rhs);
                    Operand lhs = hin->get_operand(0);
                    Operand dest = get_mreg(lhs);

                    auto *movins = new Instruction(MINS_MOVQ, src, r11);
                    movins->set_comment(get_hins_comment(hin));
                    assembly->add_instruction(movins);

                    auto *movins2 = new Instruction(MINS_MOVQ, r11, dest);
                    assembly->add_instruction(movins2);
                    break;
                }
                case HINS_WRITE_INT: {
                    // move outputfmt to first argument register
                    auto *movfmt = new Instruction(MINS_MOVQ, outputfmt, rdi);
                    movfmt->set_comment(get_hins_comment(hin));
                    assembly->add_instruction(movfmt);

                    // load addr of vreg into second argument register
                    Operand op = hin->get_operand(0);
                    Operand src = get_mreg_or_lit(op);
                    auto *leaq = new Instruction(MINS_MOVQ, src, rsi);
                    assembly->add_instruction(leaq);

                    // call the printf function
                    auto *printf = new Instruction(MINS_CALL, printf_label);
                    assembly->add_instruction(printf);
                    break;
                }
                case HINS_READ_INT: {
                    // move inputfmt to first argument register
                    auto *movfmt = new Instruction(MINS_MOVQ, inputfmt, rdi);
                    movfmt->set_comment(get_hins_comment(hin));
                    assembly->add_instruction(movfmt);

                    // load addr of vreg into second argument register
                    Operand op = hin->get_operand(0);
                    Operand dest = get_mreg(op);
                    auto *leaq = new Instruction(MINS_LEAQ, dest, rsi);
                    assembly->add_instruction(leaq);

                    // call the scanf function
                    auto *scanf = new Instruction(MINS_CALL, scanf_label);
                    assembly->add_instruction(scanf);
                    break;
                }
                case HINS_INT_ADD: {
                    Operand dest = hin->get_operand(0);
                    Operand arg1 = hin->get_operand(1);
                    Operand arg2 = hin->get_operand(2);

                    Operand addend = get_mreg_or_lit(arg1);
                    auto *movarg1 = new Instruction(MINS_MOVQ,addend, r11);
                    movarg1->set_comment(get_hins_comment(hin));
                    assembly->add_instruction(movarg1);

                    Operand destination = get_mreg_or_lit(arg2);
                    auto *movarg2 = new Instruction(MINS_MOVQ, destination, r10);
                    assembly->add_instruction(movarg2);

                    auto *addins = new Instruction(MINS_ADDQ, r11, r10);
                    assembly->add_instruction(addins);

                    Operand resultdestination = get_mreg(dest);
                    auto *movins = new Instruction(MINS_MOVQ, r10, resultdestination);
                    assembly->add_instruction(movins);
                    break;
                }
                case HINS_INT_SUB: {
                    Operand dest = hin->get_operand(0);
                    Operand arg1 = hin->get_operand(1);
                    Operand arg2 = hin->get_operand(2);

                    Operand destination = get_mreg_or_lit(arg1);
                    auto *movarg1 = new Instruction(MINS_MOVQ, destination, r10);
                    movarg1->set_comment(get_hins_comment(hin));
                    assembly->add_instruction(movarg1);

                    Operand subtrahend = get_mreg_or_lit(arg2);
                    auto *movarg2 = new Instruction(MINS_MOVQ, subtrahend, r11);
                    assembly->add_instruction(movarg2);

                    // https://en.wikibooks.org/wiki/X86_Assembly/Arithmetic#Addition_and_Subtraction
                    // sub subtrahend, dest
                    // dest -= subtrahend
                    // dest = dest - subtrahend
                    // HINS_INT_SUB d, a1, a2
                    // SUBQ a2, a1 // places result in a1
                    auto *subins = new Instruction(MINS_SUBQ, r11, r10);
                    assembly->add_instruction(subins);

                    // r10 contains the result now
                    Operand resultdestination = get_mreg(dest);
                    auto *movins = new Instruction(MINS_MOVQ, r10, resultdestination);
                    assembly->add_instruction(movins);
                    break;
                }
                case HINS_INT_MUL: {
                    Operand dest = hin->get_operand(0);
                    Operand arg1 = hin->get_operand(1);
                    Operand arg2 = hin->get_operand(2);

                    Operand multiplicand = get_mreg_or_lit(arg1);
                    auto *movarg1 = new Instruction(MINS_MOVQ, multiplicand, r11);
                    movarg1->set_comment(get_hins_comment(hin));
                    assembly->add_instruction(movarg1);

                    if (arg1.is_memref()) {
                        auto *deref = new Instruction(MINS_MOVQ, r11.to_memref(), r11);
                        assembly->add_instruction(deref);
                    }

                    Operand destination = get_mreg_or_lit(arg2);
                    auto *movarg2 = new Instruction(MINS_MOVQ, destination, r10);
                    assembly->add_instruction(movarg2);

                    if (arg2.is_memref()) {
                        auto *deref = new Instruction(MINS_MOVQ, r10.to_memref(), r10);
                        assembly->add_instruction(deref);
                    }

                    auto *mulins = new Instruction(MINS_IMULQ, r11, r10);
                    assembly->add_instruction(mulins);

                    Operand resultdestination = get_mreg(dest);
                    auto *movdest = new Instruction(MINS_MOVQ, r10, resultdestination);
                    assembly->add_instruction(movdest);
                    break;
                }
                case HINS_INT_DIV: {
                    Operand dest = hin->get_operand(0);
                    Operand divarg1 = hin->get_operand(1);
                    Operand divarg2 = hin->get_operand(2);

                    Operand op1 = get_mreg_or_lit(divarg1);
                    auto *movarg1 = new Instruction(MINS_MOVQ, op1, rax);
                    movarg1->set_comment(get_hins_comment(hin));
                    assembly->add_instruction(movarg1);

                    auto *convertins = new Instruction(MINS_CQTO);
                    assembly->add_instruction(convertins);

                    Operand op2 = get_mreg_or_lit(divarg2);
                    auto *movarg2 = new Instruction(MINS_MOVQ, op2, r10);
                    assembly->add_instruction(movarg2);

                    auto *divins = new Instruction(MINS_IDIVQ, r10);
                    assembly->add_instruction(divins);

                    Operand resultdestination = get_mreg(dest);
                    auto *movdest = new Instruction(MINS_MOVQ, rax, resultdestination);
                    assembly->add_instruction(movdest);
                    break;
                }
                case HINS_INT_MOD: {
                    Operand dest = hin->get_operand(0);
                    Operand modarg1 = hin->get_operand(1);
                    Operand modarg2 = hin->get_operand(2);

                    Operand op1 = get_mreg_or_lit(modarg1);
                    auto *movarg1 = new Instruction(MINS_MOVQ, op1, rax);
                    movarg1->set_comment(get_hins_comment(hin));
                    assembly->add_instruction(movarg1);

                    auto *convertins = new Instruction(MINS_CQTO);
                    assembly->add_instruction(convertins);

                    Operand op2 = get_mreg_or_lit(modarg2);
                    auto *movarg2 = new Instruction(MINS_MOVQ, op2, r10);
                    assembly->add_instruction(movarg2);

                    auto *divins = new Instruction(MINS_IDIVQ, r10);
                    assembly->add_instruction(divins);

                    Operand resultdestination = get_mreg(dest);
                    auto *movdest = new Instruction(MINS_MOVQ, rdx, resultdestination);   // different from DIV, check %rdx for remainder
                    assembly->add_instruction(movdest);
                    break;
                }
                case HINS_INT_COMPARE: {
                    Operand l_op = hin->get_operand(0);
                    Operand r_op = hin->get_operand(1);

                    Operand l_arg = get_mreg_or_lit(l_op);
                    auto *movarg1 = new Instruction(MINS_MOVQ, l_arg, r10);
                    movarg1->set_comment(get_hins_comment(hin));
                    assembly->add_instruction(movarg1);

                    Operand r_arg = get_mreg_or_lit(r_op);
                    auto *movarg2 = new Instruction(MINS_MOVQ, r_arg, r11);
                    assembly->add_instruction(movarg2);

                    auto *compareins = new Instruction(MINS_CMPQ, r11, r10);
                    assembly->add_instruction(compareins);
                    break;
                }
                case HINS_JUMP: {
                    Operand label = hin->get_operand(0);
                    auto *jumpins = new Instruction(MINS_JMP, label);
                    jumpins->set_comment(get_hins_comment(hin));
                    assembly->add_instruction(jumpins);
                    break;
                }
                case HINS_JE: {
                    Operand label = hin->get_operand(0);
                    auto *jumpins = new Instruction(MINS_JE, label);
                    jumpins->set_comment(get_hins_comment(hin));
                    assembly->add_instruction(jumpins);
                    break;
                }
                case HINS_JNE: {
                    Operand label = hin->get_operand(0);
                    auto *jumpins = new Instruction(MINS_JNE, label);
                    jumpins->set_comment(get_hins_comment(hin));
                    assembly->add_instruction(jumpins);
                    break;
                }
                case HINS_JLT: {
                    Operand label = hin->get_operand(0);
                    auto *jumpins = new Instruction(MINS_JL, label);
                    jumpins->set_comment(get_hins_comment(hin));
                    assembly->add_instruction(jumpins);
                    break;
                }
                case HINS_JLTE: {
                    Operand label = hin->get_operand(0);
                    auto *jumpins = new Instruction(MINS_JLE, label);
                    jumpins->set_comment(get_hins_comment(hin));
                    assembly->add_instruction(jumpins);
                    break;
                }
                case HINS_JGT: {
                    Operand label = hin->get_operand(0);
                    auto *jumpins = new Instruction(MINS_JG, label);
                    jumpins->set_comment(get_hins_comment(hin));
                    assembly->add_instruction(jumpins);
                    break;
                }
                case HINS_JGTE: {
                    Operand label = hin->get_operand(0);
                    auto *jumpins = new Instruction(MINS_JGE, label);
                    jumpins->set_comment(get_hins_comment(hin));
                    assembly->add_instruction(jumpins);
                    break;
                }
                case HINS_NOP: {
                    auto *nopins = new Instruction(MINS_NOP);
                    nopins->set_comment(get_hins_comment(hin));
                    assembly->add_instruction(nopins);
                }
                default:
                    break;
            }
        }

        if (hins->has_label_at_end()) {
            assembly->define_label(hins->get_label_at_end());
        }
    }

    void emit() {
        emit_preamble();
        emit_asm();
        emit_epilogue();
    }

private:
    void emit_preamble() {
        printf("/* %ld vregs used */\n", num_vreg);
        printf("\t.section .rodata\n");
        printf("s_readint_fmt: .string \"%%ld\"\n");
        printf("s_writeint_fmt: .string \"%%ld\\n\"\n");
        printf("\t.section .text\n");
        printf("\t.globl main\n");
        printf("main:\n");
        printf("\tpushq %%rbx\n");
        printf("\tpushq %%r12\n");
        printf("\tpushq %%r13\n");
        printf("\tpushq %%r14\n");
        printf("\tpushq %%r15\n");
        printf("\tsubq $%ld, %%rsp\n", total_storage_size);
    }

    void emit_asm() {
        PrintX86_64InstructionSequence print_asm(assembly);
        print_asm.print();
    }

    // addq storage + (8 * num_vreg), rsp
    void emit_epilogue() {
        printf("\taddq $%ld, %%rsp\n", total_storage_size);
        printf("\tpopq %%r15\n");
        printf("\tpopq %%r14\n");
        printf("\tpopq %%r13\n");
        printf("\tpopq %%r12\n");
        printf("\tpopq %%rbx\n");
        printf("\tmovl $0, %%eax\n");
        printf("\tret\n");
    }

    std::string get_hins_comment(Instruction* hin) {
        return print_helper->format_instruction(hin);
    }

    Operand get_mreg(Operand vreg) {
        assert(vreg.has_base_reg());

        if (vreg.get_does_map_mreg()) {
            // naively map vr0 - vr4 to rbx, r12, r13, r14, r15
            switch(vreg.get_base_reg()) {
                case 0:
                    return Operand(OPERAND_MREG, MREG_RBX);
                case 1:
                    return Operand(OPERAND_MREG, MREG_R12);
                case 2:
                    return Operand(OPERAND_MREG, MREG_R13);
                case 3:
                    return Operand(OPERAND_MREG, MREG_R14);
                case 4:
                    return Operand(OPERAND_MREG, MREG_R15);
            }
        }

        long offset = local_storage_size + (vreg.get_base_reg() * WORD_SIZE);
        Operand rspwithoffset(OPERAND_MREG_MEMREF_OFFSET, MREG_RSP, offset);
        return rspwithoffset;
    }

    Operand get_mreg_or_lit(Operand vreg_or_lit) {
        if (vreg_or_lit.get_kind() == OPERAND_INT_LITERAL) {
            return vreg_or_lit;
        } else {
            return get_mreg(vreg_or_lit);
        }
    }
};

class ConstantPropagation : public ControlFlowGraphTransform {

public:
    ConstantPropagation(ControlFlowGraph *cfg) : ControlFlowGraphTransform(cfg) {}

public:
    InstructionSequence *transform_basic_block(InstructionSequence *iseq) override {
        auto out = new InstructionSequence();

        std::map<int, Operand> const_values;

        for (auto ins : *iseq) {
            Instruction *hin = ins->duplicate();
            int opcode = hin->get_opcode();

            // the instruction that may be modified
            Instruction *instruction = hin->duplicate();

            if (opcode == HINS_LOAD_ICONST) {
                // lhs is virtual register, rhs is const literal
                Operand dest = hin->get_operand(0);
                int vreg = dest.get_base_reg();
                Operand lit = hin->get_operand(1);

                const_values[vreg] = lit;

                // for further instructions, replace all usages of vreg with $lit
                // remove (aka don't add to ins) HINS_LOAD_ICONST instructions
            } else {

                for (int j = 0; j < hin->get_num_operands(); j++) {
                    Operand operand = hin->get_operand(j);

                    if (operand.has_base_reg()) {
                        auto it = const_values.find(operand.get_base_reg());
                        if (it != const_values.end()) {
                            if (opcode == HINS_LOCALADDR || opcode == HINS_LOAD_INT) {
                                // if used in LOCALADDR or LOAD_INT, vreg is now in use for a scalar variable
                                const_values.erase(it);
                            } else {
                                // vreg representing constant found
                                // replace vreg with literal
                                instruction->operator[](j) = it->second;
                                if (opcode == HINS_MOV) {
                                    // we can stop constant propagation after HINS_MOV
                                    const_values.erase(it);
                                }
                            }
                        }
                    }
                }
                out->add_instruction(instruction);
            }
        }
        return out;
    }
};

class NaiveRegisterAllocation : public ControlFlowGraphTransform {

public:
    NaiveRegisterAllocation(ControlFlowGraph *cfg) : ControlFlowGraphTransform(cfg) {}

public:
    InstructionSequence *transform_basic_block(InstructionSequence *iseq) override {
        auto out = new InstructionSequence();

        for (auto ins : *iseq) {
            Instruction *hin = ins->duplicate();
            int opcode = hin->get_opcode();

            // transform operands that are scalars contained in vregs
            // set them to want mregs
            for (int j = 0; j < hin->get_num_operands(); j++) {
                Operand operand = hin->get_operand(j);
                if (operand.get_is_scalar()) {
                    operand.set_does_map_mreg(true);
                    hin->operator[](j) = operand;
                }
            }

            out->add_instruction(hin);
        }

        return out;
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
  if (flag == 'o') {
      flag_optimize = true;
  }
  if (flag == 'c') {
      flag_compile = true;
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

    InstructionSequence *iseq = hlcodegen->get_iseq();

    if (flag_optimize) {
        HighLevelControlFlowGraphBuilder cfg_builder(iseq);
        ControlFlowGraph *cfg = cfg_builder.build();

        // CFG Printer
        //HighLevelControlFlowGraphPrinter cfg_printer(cfg);
        //cfg_printer.print();

        // Live Vregs Printer
        // auto live_vregs = new LiveVregs(cfg);
        // LiveVregsControlFlowGraphPrinter live_vregs_printer(cfg, live_vregs);
        //live_vregs_printer.print();

        NaiveRegisterAllocation registerAllocation(cfg);
        cfg = registerAllocation.transform_cfg();

        ConstantPropagation constantPropagation(cfg);
        cfg = constantPropagation.transform_cfg();

        iseq = cfg->create_instruction_sequence();
    }

    if (flag_print_hins) {
        auto *hlprinter = new PrintHighLevelInstructionSequence(iseq);
        hlprinter->print();
    }

    if (flag_compile) {
        auto *asmcodegen = new AssemblyCodeGen(
                iseq,
                hlcodegen->get_storage_size(),
                hlcodegen->get_vreg_max()
                );
        asmcodegen->translate_instructions();
        asmcodegen->emit();
    }
}

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
