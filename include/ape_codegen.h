/*
 * This is free and unencumbered software released into the public domain.
 * 
 * Anyone is free to copy, modify, publish, use, compile, sell, or
 * distribute this software, either in source code form or as a compiled
 * binary, for any purpose, commercial or non-commercial, and by any
 * means.
 * 
 * In jurisdictions that recognize copyright laws, the author or authors
 * of this software dedicate any and all copyright interest in the
 * software to the public domain. We make this dedication for the benefit
 * of the public at large and to the detriment of our heirs and
 * successors. We intend this dedication to be an overt act of
 * relinquishment in perpetuity of all present and future rights to this
 * software under copyright law.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 * 
 * For more information, please refer to <https://unlicense.org>
 */

#ifndef APE_CODEGEN_INCLUDED
#define APE_CODEGEN_INCLUDED

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#define APE_CODEGEN_WINDOWS
#if defined(_WIN64)
#define APE_CODEGEN_WINDOWS_X64
#endif
#elif defined(__linux__) || defined(__unix__)
#define APE_CODEGEN_LINUX
#elif defined(__APPLE__)
#define APE_CODEGEN_APPLE
#endif

#ifndef APE_CODEGEN_MALLOC
#if defined(APE_CODEGEN_REALLOC) || defined(APE_CODEGEN_FREE)
#pragma GCC error "Need to define MALLOC, REALLOC and FREE or none of them"
#else
#include <stdlib.h>
#define APE_CODEGEN_MALLOC malloc
#define APE_CODEGEN_REALLOC realloc
#define APE_CODEGEN_FREE free
#endif
#else
#if !defined(APE_CODEGEN_REALLOC) || !defined(APE_CODEGEN_FREE)
#pragma GCC error "Need to define MALLOC, REALLOC and FREE or none of them"
#endif
#endif

#ifndef APE_CODEGEN_ASSERT
#ifdef APE_CODEGEN_USE_STDLIB_ASSERT
#include <assert.h>
#define APE_CODEGEN_ASSERT(c) assert(c)
#else
#include <stdio.h>
#include <stdlib.h>
#define APE_CODEGEN_ASSERT(c)                                                              \
	if (!(c)) {                                                                        \
		fprintf(stderr, "%s:%d Assertion '%s' failed\n", __FILE__, __LINE__, ##c); \
		exit(1);                                                                   \
	}
#endif
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#if defined(__cplusplus)
extern "C" {
#endif

/* ============================================================================
 * Configuration
 * ============================================================================ */

#ifndef APE_CG_INITIAL_BUFFER_SIZE
#define APE_CG_INITIAL_BUFFER_SIZE 4096
#endif

#ifndef APE_CG_MAX_INDENT
#define APE_CG_MAX_INDENT 64
#endif

#ifndef APE_CG_INDENT_STRING
#define APE_CG_INDENT_STRING "    "
#endif

/* ============================================================================
 * Output Buffer - Accumulates generated code
 * ============================================================================ */

typedef struct {
	char *data;
	size_t length;
	size_t capacity;
	int indent_level;
	char indent_str[APE_CG_MAX_INDENT * 4 + 1];
} ApeCgBuffer;

void ape_cg_buf_init(ApeCgBuffer *buf);
void ape_cg_buf_free(ApeCgBuffer *buf);
void ape_cg_buf_clear(ApeCgBuffer *buf);
char *ape_cg_buf_to_string(ApeCgBuffer *buf);
const char *ape_cg_buf_data(ApeCgBuffer *buf);
size_t ape_cg_buf_length(ApeCgBuffer *buf);

void ape_cg_buf_append(ApeCgBuffer *buf, const char *str);
void ape_cg_buf_append_len(ApeCgBuffer *buf, const char *str, size_t len);
void ape_cg_buf_append_char(ApeCgBuffer *buf, char c);
void ape_cg_buf_append_fmt(ApeCgBuffer *buf, const char *fmt, ...);
void ape_cg_buf_append_line(ApeCgBuffer *buf, const char *str);
void ape_cg_buf_append_indent(ApeCgBuffer *buf);
void ape_cg_buf_newline(ApeCgBuffer *buf);

void ape_cg_indent_push(ApeCgBuffer *buf);
void ape_cg_indent_pop(ApeCgBuffer *buf);
void ape_cg_indent_set(ApeCgBuffer *buf, int level);

/* ============================================================================
 * Type System - Represent C types
 * ============================================================================ */

typedef enum {
	APE_CG_TYPE_VOID,
	APE_CG_TYPE_BOOL,
	APE_CG_TYPE_CHAR,
	APE_CG_TYPE_SCHAR,
	APE_CG_TYPE_UCHAR,
	APE_CG_TYPE_SHORT,
	APE_CG_TYPE_USHORT,
	APE_CG_TYPE_INT,
	APE_CG_TYPE_UINT,
	APE_CG_TYPE_LONG,
	APE_CG_TYPE_ULONG,
	APE_CG_TYPE_LLONG,
	APE_CG_TYPE_ULLONG,
	APE_CG_TYPE_FLOAT,
	APE_CG_TYPE_DOUBLE,
	APE_CG_TYPE_PTR,
	APE_CG_TYPE_ARRAY,
	APE_CG_TYPE_FUNC,
	APE_CG_TYPE_STRUCT,
	APE_CG_TYPE_UNION,
	APE_CG_TYPE_ENUM,
	APE_CG_TYPE_TYPEDEF,
	APE_CG_TYPE_CUSTOM
} ApeCgTypeKind;

typedef struct ApeCgType ApeCgType;

typedef struct {
	ApeCgType *base;
	size_t size;
} ApeCgArrayInfo;

typedef struct {
	ApeCgType *return_type;
	ApeCgType **param_types;
	const char **param_names;
	size_t param_count;
	bool is_variadic;
} ApeCgFuncInfo;

typedef struct {
	const char *name;
	ApeCgType *type;
} ApeCgStructField;

typedef struct {
	const char *name;
	ApeCgStructField *fields;
	size_t field_count;
} ApeCgStructInfo;

struct ApeCgType {
	ApeCgTypeKind kind;
	const char *name;
	bool is_const;
	bool is_volatile;
	union {
		ApeCgType *base;
		ApeCgArrayInfo array;
		ApeCgFuncInfo func;
		ApeCgStructInfo struct_info;
	} info;
};

ApeCgType *ape_cg_type_new(ApeCgTypeKind kind);
void ape_cg_type_free(ApeCgType *type);

ApeCgType *ape_cg_type_void(void);
ApeCgType *ape_cg_type_bool(void);
ApeCgType *ape_cg_type_char(void);
ApeCgType *ape_cg_type_int(void);
ApeCgType *ape_cg_type_uint(void);
ApeCgType *ape_cg_type_long(void);
ApeCgType *ape_cg_type_float(void);
ApeCgType *ape_cg_type_double(void);
ApeCgType *ape_cg_type_ptr(ApeCgType *base);
ApeCgType *ape_cg_type_const_ptr(ApeCgType *base);
ApeCgType *ape_cg_type_array(ApeCgType *base, size_t size);
ApeCgType *ape_cg_type_func(ApeCgType *return_type, ApeCgType **params, const char **names, size_t count, bool variadic);
ApeCgType *ape_cg_type_struct(const char *name, ApeCgStructField *fields, size_t count);
ApeCgType *ape_cg_type_custom(const char *name);

void ape_cg_type_set_const(ApeCgType *type, bool is_const);
void ape_cg_type_set_volatile(ApeCgType *type, bool is_volatile);

void ape_cg_type_emit(ApeCgType *type, ApeCgBuffer *buf, const char *var_name);
char *ape_cg_type_to_string(ApeCgType *type, const char *var_name);

size_t ape_cg_type_size(ApeCgType *type);
size_t ape_cg_type_align(ApeCgType *type);

/* ============================================================================
 * Symbol Table / Scope
 * ============================================================================ */

typedef struct ApeCgSymbol ApeCgSymbol;

struct ApeCgSymbol {
	char *name;
	ApeCgType *type;
	bool is_param;
	bool is_local;
	int stack_offset;
	struct ApeCgSymbol *next;
};

typedef struct ApeCgScope ApeCgScope;

struct ApeCgScope {
	ApeCgSymbol *symbols;
	ApeCgScope *parent;
	int stack_size;
	int param_offset;
};

ApeCgScope *ape_cg_scope_new(ApeCgScope *parent);
void ape_cg_scope_free(ApeCgScope *scope);

ApeCgSymbol *ape_cg_scope_add(ApeCgScope *scope, const char *name, ApeCgType *type);
ApeCgSymbol *ape_cg_scope_add_param(ApeCgScope *scope, const char *name, ApeCgType *type);
ApeCgSymbol *ape_cg_scope_lookup(ApeCgScope *scope, const char *name);
ApeCgSymbol *ape_cg_scope_lookup_local(ApeCgScope *scope, const char *name);

/* ============================================================================
 * Expression AST Nodes
 * ============================================================================ */

typedef enum {
	APE_CG_EXPR_LITERAL_INT,
	APE_CG_EXPR_LITERAL_UINT,
	APE_CG_EXPR_LITERAL_LONG,
	APE_CG_EXPR_LITERAL_FLOAT,
	APE_CG_EXPR_LITERAL_DOUBLE,
	APE_CG_EXPR_LITERAL_STRING,
	APE_CG_EXPR_LITERAL_CHAR,
	APE_CG_EXPR_LITERAL_BOOL,
	APE_CG_EXPR_IDENT,
	APE_CG_EXPR_BINARY,
	APE_CG_EXPR_UNARY,
	APE_CG_EXPR_CALL,
	APE_CG_EXPR_INDEX,
	APE_CG_EXPR_MEMBER,
	APE_CG_EXPR_PTR_MEMBER,
	APE_CG_EXPR_CAST,
	APE_CG_EXPR_TERNARY,
	APE_CG_EXPR_COMPOUND,
	APE_CG_EXPR_SIZEOF,
	APE_CG_EXPR_ALIGNOF,
	APE_CG_EXPR_ADDR,
	APE_CG_EXPR_DEREF
} ApeCgExprKind;

typedef enum {
	APE_CG_BINOP_ADD,
	APE_CG_BINOP_SUB,
	APE_CG_BINOP_MUL,
	APE_CG_BINOP_DIV,
	APE_CG_BINOP_MOD,
	APE_CG_BINOP_AND,
	APE_CG_BINOP_OR,
	APE_CG_BINOP_XOR,
	APE_CG_BINOP_SHL,
	APE_CG_BINOP_SHR,
	APE_CG_BINOP_EQ,
	APE_CG_BINOP_NE,
	APE_CG_BINOP_LT,
	APE_CG_BINOP_LE,
	APE_CG_BINOP_GT,
	APE_CG_BINOP_GE,
	APE_CG_BINOP_LOG_AND,
	APE_CG_BINOP_LOG_OR
} ApeCgBinOp;

typedef enum {
	APE_CG_UNOP_NEG,
	APE_CG_UNOP_POS,
	APE_CG_UNOP_NOT,
	APE_CG_UNOP_BNOT,
	APE_CG_UNOP_PRE_INC,
	APE_CG_UNOP_PRE_DEC,
	APE_CG_UNOP_POST_INC,
	APE_CG_UNOP_POST_DEC,
	APE_CG_UNOP_ADDR,
	APE_CG_UNOP_DEREF
} ApeCgUnOp;

typedef struct ApeCgExpr ApeCgExpr;

struct ApeCgExpr {
	ApeCgExprKind kind;
	ApeCgType *type;
	union {
		int64_t int_val;
		uint64_t uint_val;
		double float_val;
		const char *string_val;
		char char_val;
		bool bool_val;
		struct {
			ApeCgExpr *left;
			ApeCgBinOp op;
			ApeCgExpr *right;
		} binary;
		struct {
			ApeCgUnOp op;
			ApeCgExpr *operand;
		} unary;
		struct {
			ApeCgExpr *func;
			ApeCgExpr **args;
			size_t arg_count;
		} call;
		struct {
			ApeCgExpr *array;
			ApeCgExpr *index;
		} index;
		struct {
			ApeCgExpr *struct_expr;
			const char *member;
		} member;
		struct {
			ApeCgType *target_type;
			ApeCgExpr *operand;
		} cast;
		struct {
			ApeCgExpr *cond;
			ApeCgExpr *then_expr;
			ApeCgExpr *else_expr;
		} ternary;
		struct {
			ApeCgType *type;
			ApeCgExpr **elements;
			size_t count;
		} compound;
		struct {
			ApeCgType *type;
			ApeCgExpr *expr;
		} type_expr;
		struct {
			ApeCgExpr *operand;
		} addr_deref;
		const char *ident;
	} data;
};

ApeCgExpr *ape_cg_expr_new(ApeCgExprKind kind);
void ape_cg_expr_free(ApeCgExpr *expr);

ApeCgExpr *ape_cg_expr_int(int64_t val);
ApeCgExpr *ape_cg_expr_uint(uint64_t val);
ApeCgExpr *ape_cg_expr_float(double val);
ApeCgExpr *ape_cg_expr_string(const char *val);
ApeCgExpr *ape_cg_expr_char(char val);
ApeCgExpr *ape_cg_expr_bool(bool val);
ApeCgExpr *ape_cg_expr_ident(const char *name);
ApeCgExpr *ape_cg_expr_binary(ApeCgBinOp op, ApeCgExpr *left, ApeCgExpr *right);
ApeCgExpr *ape_cg_expr_unary(ApeCgUnOp op, ApeCgExpr *operand);
ApeCgExpr *ape_cg_expr_call(ApeCgExpr *func, ApeCgExpr **args, size_t count);
ApeCgExpr *ape_cg_expr_index(ApeCgExpr *array, ApeCgExpr *index);
ApeCgExpr *ape_cg_expr_member(ApeCgExpr *struct_expr, const char *member);
ApeCgExpr *ape_cg_expr_ptr_member(ApeCgExpr *ptr, const char *member);
ApeCgExpr *ape_cg_expr_cast(ApeCgType *type, ApeCgExpr *operand);
ApeCgExpr *ape_cg_expr_ternary(ApeCgExpr *cond, ApeCgExpr *then_expr, ApeCgExpr *else_expr);
ApeCgExpr *ape_cg_expr_addr(ApeCgExpr *operand);
ApeCgExpr *ape_cg_expr_deref(ApeCgExpr *operand);
ApeCgExpr *ape_cg_expr_sizeof_type(ApeCgType *type);
ApeCgExpr *ape_cg_expr_sizeof_expr(ApeCgExpr *expr);

void ape_cg_expr_emit(ApeCgExpr *expr, ApeCgBuffer *buf);
char *ape_cg_expr_to_string(ApeCgExpr *expr);

/* ============================================================================
 * Statement AST Nodes
 * ============================================================================ */

typedef enum {
	APE_CG_STMT_EMPTY,
	APE_CG_STMT_EXPR,
	APE_CG_STMT_RETURN,
	APE_CG_STMT_IF,
	APE_CG_STMT_WHILE,
	APE_CG_STMT_DO_WHILE,
	APE_CG_STMT_FOR,
	APE_CG_STMT_SWITCH,
	APE_CG_STMT_CASE,
	APE_CG_STMT_DEFAULT,
	APE_CG_STMT_BREAK,
	APE_CG_STMT_CONTINUE,
	APE_CG_STMT_GOTO,
	APE_CG_STMT_LABEL,
	APE_CG_STMT_BLOCK,
	APE_CG_STMT_DECL,
	APE_CG_STMT_ASM
} ApeCgStmtKind;

typedef struct ApeCgStmt ApeCgStmt;
typedef struct ApeCgSwitchCase ApeCgSwitchCase;

struct ApeCgStmt {
	ApeCgStmtKind kind;
	union {
		ApeCgExpr *expr;
		struct {
			ApeCgExpr *cond;
			ApeCgStmt *then_stmt;
			ApeCgStmt *else_stmt;
		} if_stmt;
		struct {
			ApeCgExpr *cond;
			ApeCgStmt *body;
		} while_stmt;
		struct {
			ApeCgExpr *init;
			ApeCgExpr *cond;
			ApeCgExpr *update;
			ApeCgStmt *body;
		} for_stmt;
		struct {
			ApeCgExpr *expr;
			ApeCgSwitchCase *cases;
			size_t case_count;
			ApeCgStmt *default_case;
		} switch_stmt;
		struct {
			const char *label;
		} goto_stmt;
		struct {
			const char *name;
			ApeCgType *type;
			ApeCgExpr *init;
		} decl;
		struct {
			ApeCgStmt **stmts;
			size_t count;
		} block;
		struct {
			const char *asm_str;
		} asm_stmt;
	} data;
};

struct ApeCgSwitchCase {
	ApeCgExpr *value;
	ApeCgStmt *stmt;
};

ApeCgStmt *ape_cg_stmt_new(ApeCgStmtKind kind);
void ape_cg_stmt_free(ApeCgStmt *stmt);

ApeCgStmt *ape_cg_stmt_empty(void);
ApeCgStmt *ape_cg_stmt_expr(ApeCgExpr *expr);
ApeCgStmt *ape_cg_stmt_return(ApeCgExpr *expr);
ApeCgStmt *ape_cg_stmt_if(ApeCgExpr *cond, ApeCgStmt *then_stmt, ApeCgStmt *else_stmt);
ApeCgStmt *ape_cg_stmt_while(ApeCgExpr *cond, ApeCgStmt *body);
ApeCgStmt *ape_cg_stmt_do_while(ApeCgExpr *cond, ApeCgStmt *body);
ApeCgStmt *ape_cg_stmt_for(ApeCgExpr *init, ApeCgExpr *cond, ApeCgExpr *update, ApeCgStmt *body);
ApeCgStmt *ape_cg_stmt_switch(ApeCgExpr *expr, ApeCgSwitchCase *cases, size_t case_count, ApeCgStmt *default_body);
ApeCgStmt *ape_cg_stmt_break(void);
ApeCgStmt *ape_cg_stmt_continue(void);
ApeCgStmt *ape_cg_stmt_goto(const char *label);
ApeCgStmt *ape_cg_stmt_label(const char *name);
ApeCgStmt *ape_cg_stmt_block(ApeCgStmt **stmts, size_t count);
ApeCgStmt *ape_cg_stmt_decl(const char *name, ApeCgType *type, ApeCgExpr *init);
ApeCgStmt *ape_cg_stmt_asm(const char *asm_str);

void ape_cg_stmt_emit(ApeCgStmt *stmt, ApeCgBuffer *buf);

/* ============================================================================
 * Function/Top-level Declarations
 * ============================================================================ */

typedef struct {
	const char *name;
	ApeCgType *type;
	ApeCgStmt *body;
	bool is_static;
	bool is_inline;
} ApeCgFunction;

ApeCgFunction *ape_cg_func_new(const char *name, ApeCgType *type);
void ape_cg_func_free(ApeCgFunction *func);

void ape_cg_func_set_body(ApeCgFunction *func, ApeCgStmt *body);
void ape_cg_func_set_static(ApeCgFunction *func, bool is_static);
void ape_cg_func_set_inline(ApeCgFunction *func, bool is_inline);

void ape_cg_func_emit(ApeCgFunction *func, ApeCgBuffer *buf);
void ape_cg_func_emit_forward_decl(ApeCgFunction *func, ApeCgBuffer *buf);

/* ============================================================================
 * Module/File Generation
 * ============================================================================ */

typedef struct {
	const char *name;
	const char **includes;
	size_t include_count;
	ApeCgFunction **functions;
	size_t func_count;
	const char **typedefs;
	size_t typedef_count;
	const char **structs;
	size_t struct_count;
} ApeCgModule;

ApeCgModule *ape_cg_module_new(const char *name);
void ape_cg_module_free(ApeCgModule *module);

void ape_cg_module_add_include(ApeCgModule *module, const char *path);
void ape_cg_module_add_function(ApeCgModule *module, ApeCgFunction *func);
void ape_cg_module_add_typedef(ApeCgModule *module, const char *name, const char *def);
void ape_cg_module_add_struct(ApeCgModule *module, const char *struct_def);

void ape_cg_module_emit(ApeCgModule *module, ApeCgBuffer *buf);

/* ============================================================================
 * Label Generator
 * ============================================================================ */

typedef struct {
	int counter;
	char prefix[32];
} ApeCgLabelGen;

void ape_cg_labelgen_init(ApeCgLabelGen *gen, const char *prefix);
char *ape_cg_labelgen_new(ApeCgLabelGen *gen);
char *ape_cg_labelgen_named(ApeCgLabelGen *gen, const char *name);

/* ============================================================================
 * Utility Functions
 * ============================================================================ */

const char *ape_cg_binop_str(ApeCgBinOp op);
const char *ape_cg_unop_str(ApeCgUnOp op);
const char *ape_cg_typekind_str(ApeCgTypeKind kind);

char *ape_cg_escape_string(const char *str);
char *ape_cg_mangle_name(const char *prefix, const char *name);

int64_t ape_cg_eval_const_int(ApeCgExpr *expr);
bool ape_cg_is_const_expr(ApeCgExpr *expr);

#if defined(__cplusplus)
}
#endif

#if defined(APE_CODEGEN_STRIP_PREFIX)

#endif

#endif

#ifdef APE_CODEGEN_IMPLEMENTATION

#ifndef APE_CODEGEN_IMPLEMENTATION_INCLUDED
#define APE_CODEGEN_IMPLEMENTATION_INCLUDED

/* User can define a custom function prefix (eg. static) */
#ifndef APE_CODEGEN_DEF
#define APE_CODEGEN_DEF
#endif

/* Can also define custom private function prefix */
#ifndef APE_CODEGEN_PRIVATE
#define APE_CODEGEN_PRIVATE static
#endif

#ifndef APE_CODEGEN_TRUE
#define APE_CODEGEN_TRUE (1)
#define APE_CODEGEN_FALSE (0)
#else
#if !defined(APE_CODEGEN_FALSE)
#pragma GCC error "Need to define both APE_CODEGEN_TRUE and APE_CODEGEN_FALSE or neither"
#endif
#endif

#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <ctype.h>

/* BEGIN ape_codegen.c */
/*
 * ape_codegen.c - Code generation library implementation
 *
 * Provides utilities for programmatically generating C source code,
 * including a type system, expression/statement AST, scope management,
 * and text emission with indentation tracking.
 */

/* ============================================================================
 * Internal Helpers
 * ============================================================================ */

APE_CODEGEN_PRIVATE char *ape_cg_strdup(const char *str) {
	if (!str) return NULL;
	size_t len = strlen(str);
	char *dup = (char *)APE_CODEGEN_MALLOC(len + 1);
	if (!dup) return NULL;
	memcpy(dup, str, len + 1);
	return dup;
}

APE_CODEGEN_PRIVATE void ape_cg_buf_grow(ApeCgBuffer *buf, size_t needed) {
	if (buf->length + needed + 1 <= buf->capacity) return;
	size_t new_cap = buf->capacity * 2;
	if (new_cap < buf->length + needed + 1) new_cap = buf->length + needed + 1;
	buf->data = (char *)APE_CODEGEN_REALLOC(buf->data, new_cap);
	buf->capacity = new_cap;
}

APE_CODEGEN_PRIVATE void ape_cg_rebuild_indent_str(ApeCgBuffer *buf) {
	buf->indent_str[0] = '\0';
	const char *unit = APE_CG_INDENT_STRING;
	size_t unit_len = strlen(unit);
	size_t total = 0;
	for (int i = 0; i < buf->indent_level && total + unit_len < sizeof(buf->indent_str) - 1; i++) {
		memcpy(buf->indent_str + total, unit, unit_len);
		total += unit_len;
	}
	buf->indent_str[total] = '\0';
}

/* ============================================================================
 * Output Buffer
 * ============================================================================ */

APE_CODEGEN_DEF void ape_cg_buf_init(ApeCgBuffer *buf) {
	memset(buf, 0, sizeof(*buf));
	buf->capacity = APE_CG_INITIAL_BUFFER_SIZE;
	buf->data = (char *)APE_CODEGEN_MALLOC(buf->capacity);
	buf->data[0] = '\0';
	buf->indent_level = 0;
	buf->indent_str[0] = '\0';
}

APE_CODEGEN_DEF void ape_cg_buf_free(ApeCgBuffer *buf) {
	if (!buf) return;
	APE_CODEGEN_FREE(buf->data);
	buf->data = NULL;
	buf->length = 0;
	buf->capacity = 0;
}

APE_CODEGEN_DEF void ape_cg_buf_clear(ApeCgBuffer *buf) {
	buf->length = 0;
	if (buf->data) buf->data[0] = '\0';
}

APE_CODEGEN_DEF char *ape_cg_buf_to_string(ApeCgBuffer *buf) { return ape_cg_strdup(buf->data); }

APE_CODEGEN_DEF const char *ape_cg_buf_data(ApeCgBuffer *buf) { return buf->data; }

APE_CODEGEN_DEF size_t ape_cg_buf_length(ApeCgBuffer *buf) { return buf->length; }

APE_CODEGEN_DEF void ape_cg_buf_append(ApeCgBuffer *buf, const char *str) {
	if (!str) return;
	size_t len = strlen(str);
	ape_cg_buf_grow(buf, len);
	memcpy(buf->data + buf->length, str, len);
	buf->length += len;
	buf->data[buf->length] = '\0';
}

APE_CODEGEN_DEF void ape_cg_buf_append_len(ApeCgBuffer *buf, const char *str, size_t len) {
	if (!str || len == 0) return;
	ape_cg_buf_grow(buf, len);
	memcpy(buf->data + buf->length, str, len);
	buf->length += len;
	buf->data[buf->length] = '\0';
}

APE_CODEGEN_DEF void ape_cg_buf_append_char(ApeCgBuffer *buf, char c) {
	ape_cg_buf_grow(buf, 1);
	buf->data[buf->length++] = c;
	buf->data[buf->length] = '\0';
}

APE_CODEGEN_DEF void ape_cg_buf_append_fmt(ApeCgBuffer *buf, const char *fmt, ...) {
	char tmp[1024];
	va_list args;
	va_start(args, fmt);
	int len = vsnprintf(tmp, sizeof(tmp), fmt, args);
	va_end(args);
	if (len > 0) {
		if ((size_t)len < sizeof(tmp)) {
			ape_cg_buf_append_len(buf, tmp, (size_t)len);
		} else {
			char *big = (char *)APE_CODEGEN_MALLOC((size_t)len + 1);
			va_start(args, fmt);
			vsnprintf(big, (size_t)len + 1, fmt, args);
			va_end(args);
			ape_cg_buf_append_len(buf, big, (size_t)len);
			APE_CODEGEN_FREE(big);
		}
	}
}

APE_CODEGEN_DEF void ape_cg_buf_append_indent(ApeCgBuffer *buf) {
	if (buf->indent_level > 0) ape_cg_buf_append(buf, buf->indent_str);
}

APE_CODEGEN_DEF void ape_cg_buf_append_line(ApeCgBuffer *buf, const char *str) {
	ape_cg_buf_append_indent(buf);
	ape_cg_buf_append(buf, str);
	ape_cg_buf_append_char(buf, '\n');
}

APE_CODEGEN_DEF void ape_cg_buf_newline(ApeCgBuffer *buf) { ape_cg_buf_append_char(buf, '\n'); }

APE_CODEGEN_DEF void ape_cg_indent_push(ApeCgBuffer *buf) {
	if (buf->indent_level < APE_CG_MAX_INDENT) buf->indent_level++;
	ape_cg_rebuild_indent_str(buf);
}

APE_CODEGEN_DEF void ape_cg_indent_pop(ApeCgBuffer *buf) {
	if (buf->indent_level > 0) buf->indent_level--;
	ape_cg_rebuild_indent_str(buf);
}

APE_CODEGEN_DEF void ape_cg_indent_set(ApeCgBuffer *buf, int level) {
	if (level < 0) level = 0;
	if (level > APE_CG_MAX_INDENT) level = APE_CG_MAX_INDENT;
	buf->indent_level = level;
	ape_cg_rebuild_indent_str(buf);
}

/* ============================================================================
 * Type System
 * ============================================================================ */

APE_CODEGEN_DEF ApeCgType *ape_cg_type_new(ApeCgTypeKind kind) {
	ApeCgType *type = (ApeCgType *)APE_CODEGEN_MALLOC(sizeof(ApeCgType));
	memset(type, 0, sizeof(*type));
	type->kind = kind;
	return type;
}

APE_CODEGEN_DEF void ape_cg_type_free(ApeCgType *type) {
	if (!type) return;
	switch (type->kind) {
	case APE_CG_TYPE_PTR: ape_cg_type_free(type->info.base); break;
	case APE_CG_TYPE_ARRAY: ape_cg_type_free(type->info.array.base); break;
	case APE_CG_TYPE_FUNC:
		ape_cg_type_free(type->info.func.return_type);
		for (size_t i = 0; i < type->info.func.param_count; i++) ape_cg_type_free(type->info.func.param_types[i]);
		APE_CODEGEN_FREE(type->info.func.param_types);
		APE_CODEGEN_FREE(type->info.func.param_names);
		break;
	case APE_CG_TYPE_STRUCT:
	case APE_CG_TYPE_UNION:
		for (size_t i = 0; i < type->info.struct_info.field_count; i++) ape_cg_type_free(type->info.struct_info.fields[i].type);
		APE_CODEGEN_FREE(type->info.struct_info.fields);
		break;
	default: break;
	}
	APE_CODEGEN_FREE(type);
}

APE_CODEGEN_DEF ApeCgType *ape_cg_type_void(void) { return ape_cg_type_new(APE_CG_TYPE_VOID); }
APE_CODEGEN_DEF ApeCgType *ape_cg_type_bool(void) { return ape_cg_type_new(APE_CG_TYPE_BOOL); }
APE_CODEGEN_DEF ApeCgType *ape_cg_type_char(void) { return ape_cg_type_new(APE_CG_TYPE_CHAR); }
APE_CODEGEN_DEF ApeCgType *ape_cg_type_int(void) { return ape_cg_type_new(APE_CG_TYPE_INT); }
APE_CODEGEN_DEF ApeCgType *ape_cg_type_uint(void) { return ape_cg_type_new(APE_CG_TYPE_UINT); }
APE_CODEGEN_DEF ApeCgType *ape_cg_type_long(void) { return ape_cg_type_new(APE_CG_TYPE_LONG); }
APE_CODEGEN_DEF ApeCgType *ape_cg_type_float(void) { return ape_cg_type_new(APE_CG_TYPE_FLOAT); }
APE_CODEGEN_DEF ApeCgType *ape_cg_type_double(void) { return ape_cg_type_new(APE_CG_TYPE_DOUBLE); }

APE_CODEGEN_DEF ApeCgType *ape_cg_type_ptr(ApeCgType *base) {
	ApeCgType *type = ape_cg_type_new(APE_CG_TYPE_PTR);
	type->info.base = base;
	return type;
}

APE_CODEGEN_DEF ApeCgType *ape_cg_type_const_ptr(ApeCgType *base) {
	ApeCgType *type = ape_cg_type_ptr(base);
	type->is_const = true;
	return type;
}

APE_CODEGEN_DEF ApeCgType *ape_cg_type_array(ApeCgType *base, size_t size) {
	ApeCgType *type = ape_cg_type_new(APE_CG_TYPE_ARRAY);
	type->info.array.base = base;
	type->info.array.size = size;
	return type;
}

APE_CODEGEN_DEF ApeCgType *ape_cg_type_func(ApeCgType *return_type, ApeCgType **params, const char **names, size_t count, bool variadic) {
	ApeCgType *type = ape_cg_type_new(APE_CG_TYPE_FUNC);
	type->info.func.return_type = return_type;
	type->info.func.param_count = count;
	type->info.func.is_variadic = variadic;
	if (count > 0) {
		type->info.func.param_types = (ApeCgType **)APE_CODEGEN_MALLOC(sizeof(ApeCgType *) * count);
		memcpy(type->info.func.param_types, params, sizeof(ApeCgType *) * count);
		if (names) {
			type->info.func.param_names = (const char **)APE_CODEGEN_MALLOC(sizeof(const char *) * count);
			for (size_t i = 0; i < count; i++) type->info.func.param_names[i] = names[i];
		}
	}
	return type;
}

APE_CODEGEN_DEF ApeCgType *ape_cg_type_struct(const char *name, ApeCgStructField *fields, size_t count) {
	ApeCgType *type = ape_cg_type_new(APE_CG_TYPE_STRUCT);
	type->name = name;
	type->info.struct_info.name = name;
	type->info.struct_info.field_count = count;
	if (count > 0) {
		type->info.struct_info.fields = (ApeCgStructField *)APE_CODEGEN_MALLOC(sizeof(ApeCgStructField) * count);
		memcpy(type->info.struct_info.fields, fields, sizeof(ApeCgStructField) * count);
	}
	return type;
}

APE_CODEGEN_DEF ApeCgType *ape_cg_type_custom(const char *name) {
	ApeCgType *type = ape_cg_type_new(APE_CG_TYPE_CUSTOM);
	type->name = name;
	return type;
}

APE_CODEGEN_DEF void ape_cg_type_set_const(ApeCgType *type, bool is_const) {
	if (type) type->is_const = is_const;
}

APE_CODEGEN_DEF void ape_cg_type_set_volatile(ApeCgType *type, bool is_volatile) {
	if (type) type->is_volatile = is_volatile;
}

APE_CODEGEN_PRIVATE const char *ape_cg_primitive_type_name(ApeCgTypeKind kind) {
	switch (kind) {
	case APE_CG_TYPE_VOID: return "void";
	case APE_CG_TYPE_BOOL: return "bool";
	case APE_CG_TYPE_CHAR: return "char";
	case APE_CG_TYPE_SCHAR: return "signed char";
	case APE_CG_TYPE_UCHAR: return "unsigned char";
	case APE_CG_TYPE_SHORT: return "short";
	case APE_CG_TYPE_USHORT: return "unsigned short";
	case APE_CG_TYPE_INT: return "int";
	case APE_CG_TYPE_UINT: return "unsigned int";
	case APE_CG_TYPE_LONG: return "long";
	case APE_CG_TYPE_ULONG: return "unsigned long";
	case APE_CG_TYPE_LLONG: return "long long";
	case APE_CG_TYPE_ULLONG: return "unsigned long long";
	case APE_CG_TYPE_FLOAT: return "float";
	case APE_CG_TYPE_DOUBLE: return "double";
	default: return NULL;
	}
}

APE_CODEGEN_DEF void ape_cg_type_emit(ApeCgType *type, ApeCgBuffer *buf, const char *var_name) {
	if (!type) {
		ape_cg_buf_append(buf, "void");
		if (var_name) {
			ape_cg_buf_append_char(buf, ' ');
			ape_cg_buf_append(buf, var_name);
		}
		return;
	}

	if (type->is_const && type->kind != APE_CG_TYPE_PTR) ape_cg_buf_append(buf, "const ");
	if (type->is_volatile) ape_cg_buf_append(buf, "volatile ");

	switch (type->kind) {
	case APE_CG_TYPE_VOID:
	case APE_CG_TYPE_BOOL:
	case APE_CG_TYPE_CHAR:
	case APE_CG_TYPE_SCHAR:
	case APE_CG_TYPE_UCHAR:
	case APE_CG_TYPE_SHORT:
	case APE_CG_TYPE_USHORT:
	case APE_CG_TYPE_INT:
	case APE_CG_TYPE_UINT:
	case APE_CG_TYPE_LONG:
	case APE_CG_TYPE_ULONG:
	case APE_CG_TYPE_LLONG:
	case APE_CG_TYPE_ULLONG:
	case APE_CG_TYPE_FLOAT:
	case APE_CG_TYPE_DOUBLE:
		ape_cg_buf_append(buf, ape_cg_primitive_type_name(type->kind));
		if (var_name) {
			ape_cg_buf_append_char(buf, ' ');
			ape_cg_buf_append(buf, var_name);
		}
		break;

	case APE_CG_TYPE_PTR:
		ape_cg_type_emit(type->info.base, buf, NULL);
		ape_cg_buf_append(buf, " *");
		if (type->is_const) ape_cg_buf_append(buf, "const ");
		if (var_name) ape_cg_buf_append(buf, var_name);
		break;

	case APE_CG_TYPE_ARRAY:
		ape_cg_type_emit(type->info.array.base, buf, var_name);
		if (type->info.array.size > 0)
			ape_cg_buf_append_fmt(buf, "[%zu]", type->info.array.size);
		else
			ape_cg_buf_append(buf, "[]");
		break;

	case APE_CG_TYPE_FUNC:
		ape_cg_type_emit(type->info.func.return_type, buf, NULL);
		if (var_name) {
			ape_cg_buf_append_char(buf, ' ');
			ape_cg_buf_append(buf, var_name);
		}
		ape_cg_buf_append_char(buf, '(');
		for (size_t i = 0; i < type->info.func.param_count; i++) {
			if (i > 0) ape_cg_buf_append(buf, ", ");
			const char *pname = (type->info.func.param_names) ? type->info.func.param_names[i] : NULL;
			ape_cg_type_emit(type->info.func.param_types[i], buf, pname);
		}
		if (type->info.func.is_variadic) {
			if (type->info.func.param_count > 0) ape_cg_buf_append(buf, ", ");
			ape_cg_buf_append(buf, "...");
		}
		if (type->info.func.param_count == 0 && !type->info.func.is_variadic) ape_cg_buf_append(buf, "void");
		ape_cg_buf_append_char(buf, ')');
		break;

	case APE_CG_TYPE_STRUCT:
		ape_cg_buf_append(buf, "struct ");
		if (type->name) ape_cg_buf_append(buf, type->name);
		if (var_name) {
			ape_cg_buf_append_char(buf, ' ');
			ape_cg_buf_append(buf, var_name);
		}
		break;

	case APE_CG_TYPE_UNION:
		ape_cg_buf_append(buf, "union ");
		if (type->name) ape_cg_buf_append(buf, type->name);
		if (var_name) {
			ape_cg_buf_append_char(buf, ' ');
			ape_cg_buf_append(buf, var_name);
		}
		break;

	case APE_CG_TYPE_ENUM:
		ape_cg_buf_append(buf, "enum ");
		if (type->name) ape_cg_buf_append(buf, type->name);
		if (var_name) {
			ape_cg_buf_append_char(buf, ' ');
			ape_cg_buf_append(buf, var_name);
		}
		break;

	case APE_CG_TYPE_TYPEDEF:
	case APE_CG_TYPE_CUSTOM:
		if (type->name) ape_cg_buf_append(buf, type->name);
		if (var_name) {
			ape_cg_buf_append_char(buf, ' ');
			ape_cg_buf_append(buf, var_name);
		}
		break;
	}
}

APE_CODEGEN_DEF char *ape_cg_type_to_string(ApeCgType *type, const char *var_name) {
	ApeCgBuffer buf;
	ape_cg_buf_init(&buf);
	ape_cg_type_emit(type, &buf, var_name);
	char *result = ape_cg_buf_to_string(&buf);
	ape_cg_buf_free(&buf);
	return result;
}

APE_CODEGEN_DEF size_t ape_cg_type_size(ApeCgType *type) {
	if (!type) return 0;
	switch (type->kind) {
	case APE_CG_TYPE_VOID: return 0;
	case APE_CG_TYPE_BOOL: return 1;
	case APE_CG_TYPE_CHAR:
	case APE_CG_TYPE_SCHAR:
	case APE_CG_TYPE_UCHAR: return 1;
	case APE_CG_TYPE_SHORT:
	case APE_CG_TYPE_USHORT: return 2;
	case APE_CG_TYPE_INT:
	case APE_CG_TYPE_UINT: return 4;
	case APE_CG_TYPE_LONG:
	case APE_CG_TYPE_ULONG: return 8;
	case APE_CG_TYPE_LLONG:
	case APE_CG_TYPE_ULLONG: return 8;
	case APE_CG_TYPE_FLOAT: return 4;
	case APE_CG_TYPE_DOUBLE: return 8;
	case APE_CG_TYPE_PTR: return 8;
	case APE_CG_TYPE_ARRAY: return ape_cg_type_size(type->info.array.base) * type->info.array.size;
	default: return 0;
	}
}

APE_CODEGEN_DEF size_t ape_cg_type_align(ApeCgType *type) {
	if (!type) return 1;
	switch (type->kind) {
	case APE_CG_TYPE_BOOL:
	case APE_CG_TYPE_CHAR:
	case APE_CG_TYPE_SCHAR:
	case APE_CG_TYPE_UCHAR: return 1;
	case APE_CG_TYPE_SHORT:
	case APE_CG_TYPE_USHORT: return 2;
	case APE_CG_TYPE_INT:
	case APE_CG_TYPE_UINT:
	case APE_CG_TYPE_FLOAT: return 4;
	case APE_CG_TYPE_LONG:
	case APE_CG_TYPE_ULONG:
	case APE_CG_TYPE_LLONG:
	case APE_CG_TYPE_ULLONG:
	case APE_CG_TYPE_DOUBLE:
	case APE_CG_TYPE_PTR: return 8;
	case APE_CG_TYPE_ARRAY: return ape_cg_type_align(type->info.array.base);
	default: return 1;
	}
}

/* ============================================================================
 * Scope / Symbol Table
 * ============================================================================ */

APE_CODEGEN_DEF ApeCgScope *ape_cg_scope_new(ApeCgScope *parent) {
	ApeCgScope *scope = (ApeCgScope *)APE_CODEGEN_MALLOC(sizeof(ApeCgScope));
	memset(scope, 0, sizeof(*scope));
	scope->parent = parent;
	if (parent) scope->stack_size = parent->stack_size;
	return scope;
}

APE_CODEGEN_DEF void ape_cg_scope_free(ApeCgScope *scope) {
	if (!scope) return;
	ApeCgSymbol *sym = scope->symbols;
	while (sym) {
		ApeCgSymbol *next = sym->next;
		APE_CODEGEN_FREE(sym->name);
		APE_CODEGEN_FREE(sym);
		sym = next;
	}
	APE_CODEGEN_FREE(scope);
}

APE_CODEGEN_DEF ApeCgSymbol *ape_cg_scope_add(ApeCgScope *scope, const char *name, ApeCgType *type) {
	ApeCgSymbol *sym = (ApeCgSymbol *)APE_CODEGEN_MALLOC(sizeof(ApeCgSymbol));
	memset(sym, 0, sizeof(*sym));
	sym->name = ape_cg_strdup(name);
	sym->type = type;
	sym->is_local = true;
	sym->stack_offset = scope->stack_size;
	scope->stack_size += (int)ape_cg_type_size(type);
	/* Align stack to type alignment */
	size_t align = ape_cg_type_align(type);
	if (align > 1) scope->stack_size = (scope->stack_size + (int)align - 1) & ~((int)align - 1);
	sym->next = scope->symbols;
	scope->symbols = sym;
	return sym;
}

APE_CODEGEN_DEF ApeCgSymbol *ape_cg_scope_add_param(ApeCgScope *scope, const char *name, ApeCgType *type) {
	ApeCgSymbol *sym = (ApeCgSymbol *)APE_CODEGEN_MALLOC(sizeof(ApeCgSymbol));
	memset(sym, 0, sizeof(*sym));
	sym->name = ape_cg_strdup(name);
	sym->type = type;
	sym->is_param = true;
	sym->stack_offset = scope->param_offset;
	scope->param_offset += (int)ape_cg_type_size(type);
	size_t align = ape_cg_type_align(type);
	if (align > 1) scope->param_offset = (scope->param_offset + (int)align - 1) & ~((int)align - 1);
	sym->next = scope->symbols;
	scope->symbols = sym;
	return sym;
}

APE_CODEGEN_DEF ApeCgSymbol *ape_cg_scope_lookup(ApeCgScope *scope, const char *name) {
	for (ApeCgScope *s = scope; s; s = s->parent) {
		ApeCgSymbol *sym = ape_cg_scope_lookup_local(s, name);
		if (sym) return sym;
	}
	return NULL;
}

APE_CODEGEN_DEF ApeCgSymbol *ape_cg_scope_lookup_local(ApeCgScope *scope, const char *name) {
	if (!scope || !name) return NULL;
	for (ApeCgSymbol *sym = scope->symbols; sym; sym = sym->next) {
		if (strcmp(sym->name, name) == 0) return sym;
	}
	return NULL;
}

/* ============================================================================
 * Expression AST
 * ============================================================================ */

APE_CODEGEN_DEF ApeCgExpr *ape_cg_expr_new(ApeCgExprKind kind) {
	ApeCgExpr *expr = (ApeCgExpr *)APE_CODEGEN_MALLOC(sizeof(ApeCgExpr));
	memset(expr, 0, sizeof(*expr));
	expr->kind = kind;
	return expr;
}

APE_CODEGEN_DEF void ape_cg_expr_free(ApeCgExpr *expr) {
	if (!expr) return;
	switch (expr->kind) {
	case APE_CG_EXPR_BINARY:
		ape_cg_expr_free(expr->data.binary.left);
		ape_cg_expr_free(expr->data.binary.right);
		break;
	case APE_CG_EXPR_UNARY: ape_cg_expr_free(expr->data.unary.operand); break;
	case APE_CG_EXPR_CALL:
		ape_cg_expr_free(expr->data.call.func);
		for (size_t i = 0; i < expr->data.call.arg_count; i++) ape_cg_expr_free(expr->data.call.args[i]);
		APE_CODEGEN_FREE(expr->data.call.args);
		break;
	case APE_CG_EXPR_INDEX:
		ape_cg_expr_free(expr->data.index.array);
		ape_cg_expr_free(expr->data.index.index);
		break;
	case APE_CG_EXPR_MEMBER:
	case APE_CG_EXPR_PTR_MEMBER: ape_cg_expr_free(expr->data.member.struct_expr); break;
	case APE_CG_EXPR_CAST:
		ape_cg_type_free(expr->data.cast.target_type);
		ape_cg_expr_free(expr->data.cast.operand);
		break;
	case APE_CG_EXPR_TERNARY:
		ape_cg_expr_free(expr->data.ternary.cond);
		ape_cg_expr_free(expr->data.ternary.then_expr);
		ape_cg_expr_free(expr->data.ternary.else_expr);
		break;
	case APE_CG_EXPR_SIZEOF:
		if (expr->data.type_expr.expr)
			ape_cg_expr_free(expr->data.type_expr.expr);
		else if (expr->data.type_expr.type)
			ape_cg_type_free(expr->data.type_expr.type);
		break;
	case APE_CG_EXPR_ADDR:
	case APE_CG_EXPR_DEREF: ape_cg_expr_free(expr->data.addr_deref.operand); break;
	default: break;
	}
	APE_CODEGEN_FREE(expr);
}

APE_CODEGEN_DEF ApeCgExpr *ape_cg_expr_int(int64_t val) {
	ApeCgExpr *e = ape_cg_expr_new(APE_CG_EXPR_LITERAL_INT);
	e->data.int_val = val;
	return e;
}

APE_CODEGEN_DEF ApeCgExpr *ape_cg_expr_uint(uint64_t val) {
	ApeCgExpr *e = ape_cg_expr_new(APE_CG_EXPR_LITERAL_UINT);
	e->data.uint_val = val;
	return e;
}

APE_CODEGEN_DEF ApeCgExpr *ape_cg_expr_float(double val) {
	ApeCgExpr *e = ape_cg_expr_new(APE_CG_EXPR_LITERAL_DOUBLE);
	e->data.float_val = val;
	return e;
}

APE_CODEGEN_DEF ApeCgExpr *ape_cg_expr_string(const char *val) {
	ApeCgExpr *e = ape_cg_expr_new(APE_CG_EXPR_LITERAL_STRING);
	e->data.string_val = val;
	return e;
}

APE_CODEGEN_DEF ApeCgExpr *ape_cg_expr_char(char val) {
	ApeCgExpr *e = ape_cg_expr_new(APE_CG_EXPR_LITERAL_CHAR);
	e->data.char_val = val;
	return e;
}

APE_CODEGEN_DEF ApeCgExpr *ape_cg_expr_bool(bool val) {
	ApeCgExpr *e = ape_cg_expr_new(APE_CG_EXPR_LITERAL_BOOL);
	e->data.bool_val = val;
	return e;
}

APE_CODEGEN_DEF ApeCgExpr *ape_cg_expr_ident(const char *name) {
	ApeCgExpr *e = ape_cg_expr_new(APE_CG_EXPR_IDENT);
	e->data.ident = name;
	return e;
}

APE_CODEGEN_DEF ApeCgExpr *ape_cg_expr_binary(ApeCgBinOp op, ApeCgExpr *left, ApeCgExpr *right) {
	ApeCgExpr *e = ape_cg_expr_new(APE_CG_EXPR_BINARY);
	e->data.binary.op = op;
	e->data.binary.left = left;
	e->data.binary.right = right;
	return e;
}

APE_CODEGEN_DEF ApeCgExpr *ape_cg_expr_unary(ApeCgUnOp op, ApeCgExpr *operand) {
	ApeCgExpr *e = ape_cg_expr_new(APE_CG_EXPR_UNARY);
	e->data.unary.op = op;
	e->data.unary.operand = operand;
	return e;
}

APE_CODEGEN_DEF ApeCgExpr *ape_cg_expr_call(ApeCgExpr *func, ApeCgExpr **args, size_t count) {
	ApeCgExpr *e = ape_cg_expr_new(APE_CG_EXPR_CALL);
	e->data.call.func = func;
	e->data.call.arg_count = count;
	if (count > 0) {
		e->data.call.args = (ApeCgExpr **)APE_CODEGEN_MALLOC(sizeof(ApeCgExpr *) * count);
		memcpy(e->data.call.args, args, sizeof(ApeCgExpr *) * count);
	}
	return e;
}

APE_CODEGEN_DEF ApeCgExpr *ape_cg_expr_index(ApeCgExpr *array, ApeCgExpr *index) {
	ApeCgExpr *e = ape_cg_expr_new(APE_CG_EXPR_INDEX);
	e->data.index.array = array;
	e->data.index.index = index;
	return e;
}

APE_CODEGEN_DEF ApeCgExpr *ape_cg_expr_member(ApeCgExpr *struct_expr, const char *member) {
	ApeCgExpr *e = ape_cg_expr_new(APE_CG_EXPR_MEMBER);
	e->data.member.struct_expr = struct_expr;
	e->data.member.member = member;
	return e;
}

APE_CODEGEN_DEF ApeCgExpr *ape_cg_expr_ptr_member(ApeCgExpr *ptr, const char *member) {
	ApeCgExpr *e = ape_cg_expr_new(APE_CG_EXPR_PTR_MEMBER);
	e->data.member.struct_expr = ptr;
	e->data.member.member = member;
	return e;
}

APE_CODEGEN_DEF ApeCgExpr *ape_cg_expr_cast(ApeCgType *type, ApeCgExpr *operand) {
	ApeCgExpr *e = ape_cg_expr_new(APE_CG_EXPR_CAST);
	e->data.cast.target_type = type;
	e->data.cast.operand = operand;
	return e;
}

APE_CODEGEN_DEF ApeCgExpr *ape_cg_expr_ternary(ApeCgExpr *cond, ApeCgExpr *then_expr, ApeCgExpr *else_expr) {
	ApeCgExpr *e = ape_cg_expr_new(APE_CG_EXPR_TERNARY);
	e->data.ternary.cond = cond;
	e->data.ternary.then_expr = then_expr;
	e->data.ternary.else_expr = else_expr;
	return e;
}

APE_CODEGEN_DEF ApeCgExpr *ape_cg_expr_addr(ApeCgExpr *operand) {
	ApeCgExpr *e = ape_cg_expr_new(APE_CG_EXPR_ADDR);
	e->data.addr_deref.operand = operand;
	return e;
}

APE_CODEGEN_DEF ApeCgExpr *ape_cg_expr_deref(ApeCgExpr *operand) {
	ApeCgExpr *e = ape_cg_expr_new(APE_CG_EXPR_DEREF);
	e->data.addr_deref.operand = operand;
	return e;
}

APE_CODEGEN_DEF ApeCgExpr *ape_cg_expr_sizeof_type(ApeCgType *type) {
	ApeCgExpr *e = ape_cg_expr_new(APE_CG_EXPR_SIZEOF);
	e->data.type_expr.type = type;
	e->data.type_expr.expr = NULL;
	return e;
}

APE_CODEGEN_DEF ApeCgExpr *ape_cg_expr_sizeof_expr(ApeCgExpr *expr) {
	ApeCgExpr *e = ape_cg_expr_new(APE_CG_EXPR_SIZEOF);
	e->data.type_expr.type = NULL;
	e->data.type_expr.expr = expr;
	return e;
}

/* Expression emission */

APE_CODEGEN_DEF void ape_cg_expr_emit(ApeCgExpr *expr, ApeCgBuffer *buf) {
	if (!expr) return;

	switch (expr->kind) {
	case APE_CG_EXPR_LITERAL_INT: ape_cg_buf_append_fmt(buf, "%lld", (long long)expr->data.int_val); break;
	case APE_CG_EXPR_LITERAL_UINT: ape_cg_buf_append_fmt(buf, "%lluU", (unsigned long long)expr->data.uint_val); break;
	case APE_CG_EXPR_LITERAL_LONG: ape_cg_buf_append_fmt(buf, "%lldL", (long long)expr->data.int_val); break;
	case APE_CG_EXPR_LITERAL_FLOAT: ape_cg_buf_append_fmt(buf, "%gf", expr->data.float_val); break;
	case APE_CG_EXPR_LITERAL_DOUBLE: ape_cg_buf_append_fmt(buf, "%g", expr->data.float_val); break;
	case APE_CG_EXPR_LITERAL_STRING: {
		char *escaped = ape_cg_escape_string(expr->data.string_val);
		ape_cg_buf_append_char(buf, '"');
		ape_cg_buf_append(buf, escaped);
		ape_cg_buf_append_char(buf, '"');
		APE_CODEGEN_FREE(escaped);
		break;
	}
	case APE_CG_EXPR_LITERAL_CHAR: {
		char c = expr->data.char_val;
		ape_cg_buf_append_char(buf, '\'');
		switch (c) {
		case '\n': ape_cg_buf_append(buf, "\\n"); break;
		case '\t': ape_cg_buf_append(buf, "\\t"); break;
		case '\r': ape_cg_buf_append(buf, "\\r"); break;
		case '\\': ape_cg_buf_append(buf, "\\\\"); break;
		case '\'': ape_cg_buf_append(buf, "\\'"); break;
		case '\0': ape_cg_buf_append(buf, "\\0"); break;
		default: ape_cg_buf_append_char(buf, c); break;
		}
		ape_cg_buf_append_char(buf, '\'');
		break;
	}
	case APE_CG_EXPR_LITERAL_BOOL: ape_cg_buf_append(buf, expr->data.bool_val ? "true" : "false"); break;
	case APE_CG_EXPR_IDENT: ape_cg_buf_append(buf, expr->data.ident); break;
	case APE_CG_EXPR_BINARY:
		ape_cg_buf_append_char(buf, '(');
		ape_cg_expr_emit(expr->data.binary.left, buf);
		ape_cg_buf_append_char(buf, ' ');
		ape_cg_buf_append(buf, ape_cg_binop_str(expr->data.binary.op));
		ape_cg_buf_append_char(buf, ' ');
		ape_cg_expr_emit(expr->data.binary.right, buf);
		ape_cg_buf_append_char(buf, ')');
		break;
	case APE_CG_EXPR_UNARY:
		switch (expr->data.unary.op) {
		case APE_CG_UNOP_POST_INC:
		case APE_CG_UNOP_POST_DEC:
			ape_cg_expr_emit(expr->data.unary.operand, buf);
			ape_cg_buf_append(buf, ape_cg_unop_str(expr->data.unary.op));
			break;
		default:
			ape_cg_buf_append(buf, ape_cg_unop_str(expr->data.unary.op));
			ape_cg_expr_emit(expr->data.unary.operand, buf);
			break;
		}
		break;
	case APE_CG_EXPR_CALL:
		ape_cg_expr_emit(expr->data.call.func, buf);
		ape_cg_buf_append_char(buf, '(');
		for (size_t i = 0; i < expr->data.call.arg_count; i++) {
			if (i > 0) ape_cg_buf_append(buf, ", ");
			ape_cg_expr_emit(expr->data.call.args[i], buf);
		}
		ape_cg_buf_append_char(buf, ')');
		break;
	case APE_CG_EXPR_INDEX:
		ape_cg_expr_emit(expr->data.index.array, buf);
		ape_cg_buf_append_char(buf, '[');
		ape_cg_expr_emit(expr->data.index.index, buf);
		ape_cg_buf_append_char(buf, ']');
		break;
	case APE_CG_EXPR_MEMBER:
		ape_cg_expr_emit(expr->data.member.struct_expr, buf);
		ape_cg_buf_append_char(buf, '.');
		ape_cg_buf_append(buf, expr->data.member.member);
		break;
	case APE_CG_EXPR_PTR_MEMBER:
		ape_cg_expr_emit(expr->data.member.struct_expr, buf);
		ape_cg_buf_append(buf, "->");
		ape_cg_buf_append(buf, expr->data.member.member);
		break;
	case APE_CG_EXPR_CAST:
		ape_cg_buf_append_char(buf, '(');
		ape_cg_type_emit(expr->data.cast.target_type, buf, NULL);
		ape_cg_buf_append_char(buf, ')');
		ape_cg_expr_emit(expr->data.cast.operand, buf);
		break;
	case APE_CG_EXPR_TERNARY:
		ape_cg_buf_append_char(buf, '(');
		ape_cg_expr_emit(expr->data.ternary.cond, buf);
		ape_cg_buf_append(buf, " ? ");
		ape_cg_expr_emit(expr->data.ternary.then_expr, buf);
		ape_cg_buf_append(buf, " : ");
		ape_cg_expr_emit(expr->data.ternary.else_expr, buf);
		ape_cg_buf_append_char(buf, ')');
		break;
	case APE_CG_EXPR_SIZEOF:
		ape_cg_buf_append(buf, "sizeof(");
		if (expr->data.type_expr.type)
			ape_cg_type_emit(expr->data.type_expr.type, buf, NULL);
		else if (expr->data.type_expr.expr)
			ape_cg_expr_emit(expr->data.type_expr.expr, buf);
		ape_cg_buf_append_char(buf, ')');
		break;
	case APE_CG_EXPR_ALIGNOF:
		ape_cg_buf_append(buf, "_Alignof(");
		if (expr->data.type_expr.type) ape_cg_type_emit(expr->data.type_expr.type, buf, NULL);
		ape_cg_buf_append_char(buf, ')');
		break;
	case APE_CG_EXPR_ADDR:
		ape_cg_buf_append_char(buf, '&');
		ape_cg_expr_emit(expr->data.addr_deref.operand, buf);
		break;
	case APE_CG_EXPR_DEREF:
		ape_cg_buf_append(buf, "(*");
		ape_cg_expr_emit(expr->data.addr_deref.operand, buf);
		ape_cg_buf_append_char(buf, ')');
		break;
	case APE_CG_EXPR_COMPOUND: break;
	}
}

APE_CODEGEN_DEF char *ape_cg_expr_to_string(ApeCgExpr *expr) {
	ApeCgBuffer buf;
	ape_cg_buf_init(&buf);
	ape_cg_expr_emit(expr, &buf);
	char *result = ape_cg_buf_to_string(&buf);
	ape_cg_buf_free(&buf);
	return result;
}

/* ============================================================================
 * Statement AST
 * ============================================================================ */

APE_CODEGEN_DEF ApeCgStmt *ape_cg_stmt_new(ApeCgStmtKind kind) {
	ApeCgStmt *stmt = (ApeCgStmt *)APE_CODEGEN_MALLOC(sizeof(ApeCgStmt));
	memset(stmt, 0, sizeof(*stmt));
	stmt->kind = kind;
	return stmt;
}

APE_CODEGEN_DEF void ape_cg_stmt_free(ApeCgStmt *stmt) {
	if (!stmt) return;
	switch (stmt->kind) {
	case APE_CG_STMT_EXPR:
	case APE_CG_STMT_RETURN: ape_cg_expr_free(stmt->data.expr); break;
	case APE_CG_STMT_IF:
		ape_cg_expr_free(stmt->data.if_stmt.cond);
		ape_cg_stmt_free(stmt->data.if_stmt.then_stmt);
		ape_cg_stmt_free(stmt->data.if_stmt.else_stmt);
		break;
	case APE_CG_STMT_WHILE:
	case APE_CG_STMT_DO_WHILE:
		ape_cg_expr_free(stmt->data.while_stmt.cond);
		ape_cg_stmt_free(stmt->data.while_stmt.body);
		break;
	case APE_CG_STMT_FOR:
		ape_cg_expr_free(stmt->data.for_stmt.init);
		ape_cg_expr_free(stmt->data.for_stmt.cond);
		ape_cg_expr_free(stmt->data.for_stmt.update);
		ape_cg_stmt_free(stmt->data.for_stmt.body);
		break;
	case APE_CG_STMT_DECL:
		ape_cg_type_free(stmt->data.decl.type);
		ape_cg_expr_free(stmt->data.decl.init);
		break;
	case APE_CG_STMT_BLOCK:
		for (size_t i = 0; i < stmt->data.block.count; i++) ape_cg_stmt_free(stmt->data.block.stmts[i]);
		APE_CODEGEN_FREE(stmt->data.block.stmts);
		break;
	case APE_CG_STMT_SWITCH:
		ape_cg_expr_free(stmt->data.switch_stmt.expr);
		for (size_t i = 0; i < stmt->data.switch_stmt.case_count; i++) {
			ape_cg_expr_free(stmt->data.switch_stmt.cases[i].value);
			ape_cg_stmt_free(stmt->data.switch_stmt.cases[i].stmt);
		}
		APE_CODEGEN_FREE(stmt->data.switch_stmt.cases);
		ape_cg_stmt_free(stmt->data.switch_stmt.default_case);
		break;
	default: break;
	}
	APE_CODEGEN_FREE(stmt);
}

APE_CODEGEN_DEF ApeCgStmt *ape_cg_stmt_empty(void) { return ape_cg_stmt_new(APE_CG_STMT_EMPTY); }

APE_CODEGEN_DEF ApeCgStmt *ape_cg_stmt_expr(ApeCgExpr *expr) {
	ApeCgStmt *s = ape_cg_stmt_new(APE_CG_STMT_EXPR);
	s->data.expr = expr;
	return s;
}

APE_CODEGEN_DEF ApeCgStmt *ape_cg_stmt_return(ApeCgExpr *expr) {
	ApeCgStmt *s = ape_cg_stmt_new(APE_CG_STMT_RETURN);
	s->data.expr = expr;
	return s;
}

APE_CODEGEN_DEF ApeCgStmt *ape_cg_stmt_if(ApeCgExpr *cond, ApeCgStmt *then_stmt, ApeCgStmt *else_stmt) {
	ApeCgStmt *s = ape_cg_stmt_new(APE_CG_STMT_IF);
	s->data.if_stmt.cond = cond;
	s->data.if_stmt.then_stmt = then_stmt;
	s->data.if_stmt.else_stmt = else_stmt;
	return s;
}

APE_CODEGEN_DEF ApeCgStmt *ape_cg_stmt_while(ApeCgExpr *cond, ApeCgStmt *body) {
	ApeCgStmt *s = ape_cg_stmt_new(APE_CG_STMT_WHILE);
	s->data.while_stmt.cond = cond;
	s->data.while_stmt.body = body;
	return s;
}

APE_CODEGEN_DEF ApeCgStmt *ape_cg_stmt_do_while(ApeCgExpr *cond, ApeCgStmt *body) {
	ApeCgStmt *s = ape_cg_stmt_new(APE_CG_STMT_DO_WHILE);
	s->data.while_stmt.cond = cond;
	s->data.while_stmt.body = body;
	return s;
}

APE_CODEGEN_DEF ApeCgStmt *ape_cg_stmt_for(ApeCgExpr *init, ApeCgExpr *cond, ApeCgExpr *update, ApeCgStmt *body) {
	ApeCgStmt *s = ape_cg_stmt_new(APE_CG_STMT_FOR);
	s->data.for_stmt.init = init;
	s->data.for_stmt.cond = cond;
	s->data.for_stmt.update = update;
	s->data.for_stmt.body = body;
	return s;
}

APE_CODEGEN_DEF ApeCgStmt *ape_cg_stmt_break(void) { return ape_cg_stmt_new(APE_CG_STMT_BREAK); }
APE_CODEGEN_DEF ApeCgStmt *ape_cg_stmt_continue(void) { return ape_cg_stmt_new(APE_CG_STMT_CONTINUE); }

APE_CODEGEN_DEF ApeCgStmt *ape_cg_stmt_goto(const char *label) {
	ApeCgStmt *s = ape_cg_stmt_new(APE_CG_STMT_GOTO);
	s->data.goto_stmt.label = label;
	return s;
}

APE_CODEGEN_DEF ApeCgStmt *ape_cg_stmt_label(const char *name) {
	ApeCgStmt *s = ape_cg_stmt_new(APE_CG_STMT_LABEL);
	s->data.goto_stmt.label = name;
	return s;
}

APE_CODEGEN_DEF ApeCgStmt *ape_cg_stmt_block(ApeCgStmt **stmts, size_t count) {
	ApeCgStmt *s = ape_cg_stmt_new(APE_CG_STMT_BLOCK);
	s->data.block.count = count;
	if (count > 0) {
		s->data.block.stmts = (ApeCgStmt **)APE_CODEGEN_MALLOC(sizeof(ApeCgStmt *) * count);
		memcpy(s->data.block.stmts, stmts, sizeof(ApeCgStmt *) * count);
	}
	return s;
}

APE_CODEGEN_DEF ApeCgStmt *ape_cg_stmt_decl(const char *name, ApeCgType *type, ApeCgExpr *init) {
	ApeCgStmt *s = ape_cg_stmt_new(APE_CG_STMT_DECL);
	s->data.decl.name = name;
	s->data.decl.type = type;
	s->data.decl.init = init;
	return s;
}

APE_CODEGEN_DEF ApeCgStmt *ape_cg_stmt_asm(const char *asm_str) {
	ApeCgStmt *s = ape_cg_stmt_new(APE_CG_STMT_ASM);
	s->data.asm_stmt.asm_str = asm_str;
	return s;
}

APE_CODEGEN_DEF ApeCgStmt *ape_cg_stmt_switch(ApeCgExpr *expr, ApeCgSwitchCase *cases, size_t case_count, ApeCgStmt *default_body) {
	ApeCgStmt *s = ape_cg_stmt_new(APE_CG_STMT_SWITCH);
	s->data.switch_stmt.expr = expr;
	s->data.switch_stmt.case_count = case_count;
	s->data.switch_stmt.default_case = default_body;
	if (case_count > 0) {
		s->data.switch_stmt.cases = (ApeCgSwitchCase *)APE_CODEGEN_MALLOC(sizeof(ApeCgSwitchCase) * case_count);
		memcpy(s->data.switch_stmt.cases, cases, sizeof(ApeCgSwitchCase) * case_count);
	}
	return s;
}

/* Statement emission */

APE_CODEGEN_DEF void ape_cg_stmt_emit(ApeCgStmt *stmt, ApeCgBuffer *buf) {
	if (!stmt) return;

	switch (stmt->kind) {
	case APE_CG_STMT_EMPTY: ape_cg_buf_append_line(buf, ";"); break;

	case APE_CG_STMT_EXPR:
		ape_cg_buf_append_indent(buf);
		ape_cg_expr_emit(stmt->data.expr, buf);
		ape_cg_buf_append(buf, ";\n");
		break;

	case APE_CG_STMT_RETURN:
		ape_cg_buf_append_indent(buf);
		if (stmt->data.expr) {
			ape_cg_buf_append(buf, "return ");
			ape_cg_expr_emit(stmt->data.expr, buf);
			ape_cg_buf_append(buf, ";\n");
		} else {
			ape_cg_buf_append(buf, "return;\n");
		}
		break;

	case APE_CG_STMT_IF:
		ape_cg_buf_append_indent(buf);
		ape_cg_buf_append(buf, "if (");
		ape_cg_expr_emit(stmt->data.if_stmt.cond, buf);
		ape_cg_buf_append(buf, ") {\n");
		ape_cg_indent_push(buf);
		ape_cg_stmt_emit(stmt->data.if_stmt.then_stmt, buf);
		ape_cg_indent_pop(buf);
		if (stmt->data.if_stmt.else_stmt) {
			ape_cg_buf_append_line(buf, "} else {");
			ape_cg_indent_push(buf);
			ape_cg_stmt_emit(stmt->data.if_stmt.else_stmt, buf);
			ape_cg_indent_pop(buf);
		}
		ape_cg_buf_append_line(buf, "}");
		break;

	case APE_CG_STMT_WHILE:
		ape_cg_buf_append_indent(buf);
		ape_cg_buf_append(buf, "while (");
		ape_cg_expr_emit(stmt->data.while_stmt.cond, buf);
		ape_cg_buf_append(buf, ") {\n");
		ape_cg_indent_push(buf);
		ape_cg_stmt_emit(stmt->data.while_stmt.body, buf);
		ape_cg_indent_pop(buf);
		ape_cg_buf_append_line(buf, "}");
		break;

	case APE_CG_STMT_DO_WHILE:
		ape_cg_buf_append_line(buf, "do {");
		ape_cg_indent_push(buf);
		ape_cg_stmt_emit(stmt->data.while_stmt.body, buf);
		ape_cg_indent_pop(buf);
		ape_cg_buf_append_indent(buf);
		ape_cg_buf_append(buf, "} while (");
		ape_cg_expr_emit(stmt->data.while_stmt.cond, buf);
		ape_cg_buf_append(buf, ");\n");
		break;

	case APE_CG_STMT_FOR:
		ape_cg_buf_append_indent(buf);
		ape_cg_buf_append(buf, "for (");
		if (stmt->data.for_stmt.init) ape_cg_expr_emit(stmt->data.for_stmt.init, buf);
		ape_cg_buf_append(buf, "; ");
		if (stmt->data.for_stmt.cond) ape_cg_expr_emit(stmt->data.for_stmt.cond, buf);
		ape_cg_buf_append(buf, "; ");
		if (stmt->data.for_stmt.update) ape_cg_expr_emit(stmt->data.for_stmt.update, buf);
		ape_cg_buf_append(buf, ") {\n");
		ape_cg_indent_push(buf);
		ape_cg_stmt_emit(stmt->data.for_stmt.body, buf);
		ape_cg_indent_pop(buf);
		ape_cg_buf_append_line(buf, "}");
		break;

	case APE_CG_STMT_BREAK: ape_cg_buf_append_line(buf, "break;"); break;

	case APE_CG_STMT_CONTINUE: ape_cg_buf_append_line(buf, "continue;"); break;

	case APE_CG_STMT_GOTO:
		ape_cg_buf_append_indent(buf);
		ape_cg_buf_append_fmt(buf, "goto %s;\n", stmt->data.goto_stmt.label);
		break;

	case APE_CG_STMT_LABEL:
		/* Labels are not indented */
		ape_cg_buf_append_fmt(buf, "%s:\n", stmt->data.goto_stmt.label);
		break;

	case APE_CG_STMT_BLOCK:
		for (size_t i = 0; i < stmt->data.block.count; i++) ape_cg_stmt_emit(stmt->data.block.stmts[i], buf);
		break;

	case APE_CG_STMT_DECL:
		ape_cg_buf_append_indent(buf);
		ape_cg_type_emit(stmt->data.decl.type, buf, stmt->data.decl.name);
		if (stmt->data.decl.init) {
			ape_cg_buf_append(buf, " = ");
			ape_cg_expr_emit(stmt->data.decl.init, buf);
		}
		ape_cg_buf_append(buf, ";\n");
		break;

	case APE_CG_STMT_ASM:
		ape_cg_buf_append_indent(buf);
		ape_cg_buf_append(buf, "__asm__(");
		ape_cg_buf_append(buf, stmt->data.asm_stmt.asm_str);
		ape_cg_buf_append(buf, ");\n");
		break;

	case APE_CG_STMT_SWITCH:
		ape_cg_buf_append_indent(buf);
		ape_cg_buf_append(buf, "switch (");
		ape_cg_expr_emit(stmt->data.switch_stmt.expr, buf);
		ape_cg_buf_append(buf, ") {\n");
		for (size_t i = 0; i < stmt->data.switch_stmt.case_count; i++) {
			ape_cg_buf_append_indent(buf);
			ape_cg_buf_append(buf, "case ");
			ape_cg_expr_emit(stmt->data.switch_stmt.cases[i].value, buf);
			ape_cg_buf_append(buf, ":\n");
			if (stmt->data.switch_stmt.cases[i].stmt) {
				ape_cg_indent_push(buf);
				ape_cg_stmt_emit(stmt->data.switch_stmt.cases[i].stmt, buf);
				ape_cg_indent_pop(buf);
			}
		}
		if (stmt->data.switch_stmt.default_case) {
			ape_cg_buf_append_indent(buf);
			ape_cg_buf_append(buf, "default:\n");
			ape_cg_indent_push(buf);
			ape_cg_stmt_emit(stmt->data.switch_stmt.default_case, buf);
			ape_cg_indent_pop(buf);
		}
		ape_cg_buf_append_line(buf, "}");
		break;

	case APE_CG_STMT_CASE:
	case APE_CG_STMT_DEFAULT:
		/* These are handled inline within SWITCH emission */
		break;
	}
}

/* ============================================================================
 * Function
 * ============================================================================ */

APE_CODEGEN_DEF ApeCgFunction *ape_cg_func_new(const char *name, ApeCgType *type) {
	ApeCgFunction *func = (ApeCgFunction *)APE_CODEGEN_MALLOC(sizeof(ApeCgFunction));
	memset(func, 0, sizeof(*func));
	func->name = name;
	func->type = type;
	return func;
}

APE_CODEGEN_DEF void ape_cg_func_free(ApeCgFunction *func) {
	if (!func) return;
	ape_cg_type_free(func->type);
	ape_cg_stmt_free(func->body);
	APE_CODEGEN_FREE(func);
}

APE_CODEGEN_DEF void ape_cg_func_set_body(ApeCgFunction *func, ApeCgStmt *body) {
	if (func) func->body = body;
}

APE_CODEGEN_DEF void ape_cg_func_set_static(ApeCgFunction *func, bool is_static) {
	if (func) func->is_static = is_static;
}

APE_CODEGEN_DEF void ape_cg_func_set_inline(ApeCgFunction *func, bool is_inline) {
	if (func) func->is_inline = is_inline;
}

APE_CODEGEN_DEF void ape_cg_func_emit_forward_decl(ApeCgFunction *func, ApeCgBuffer *buf) {
	if (!func || !func->type || func->type->kind != APE_CG_TYPE_FUNC) return;
	if (func->is_static) ape_cg_buf_append(buf, "static ");
	if (func->is_inline) ape_cg_buf_append(buf, "inline ");
	ape_cg_type_emit(func->type, buf, func->name);
	ape_cg_buf_append(buf, ";\n");
}

APE_CODEGEN_DEF void ape_cg_func_emit(ApeCgFunction *func, ApeCgBuffer *buf) {
	if (!func || !func->type || func->type->kind != APE_CG_TYPE_FUNC) return;

	if (func->is_static) ape_cg_buf_append(buf, "static ");
	if (func->is_inline) ape_cg_buf_append(buf, "inline ");
	ape_cg_type_emit(func->type, buf, func->name);
	ape_cg_buf_newline(buf);

	if (func->body) {
		ape_cg_buf_append(buf, "{\n");
		ape_cg_indent_push(buf);
		ape_cg_stmt_emit(func->body, buf);
		ape_cg_indent_pop(buf);
		ape_cg_buf_append(buf, "}\n");
	} else {
		ape_cg_buf_append(buf, "{\n}\n");
	}
}

/* ============================================================================
 * Module
 * ============================================================================ */

APE_CODEGEN_DEF ApeCgModule *ape_cg_module_new(const char *name) {
	ApeCgModule *mod = (ApeCgModule *)APE_CODEGEN_MALLOC(sizeof(ApeCgModule));
	memset(mod, 0, sizeof(*mod));
	mod->name = name;
	return mod;
}

APE_CODEGEN_DEF void ape_cg_module_free(ApeCgModule *module) {
	if (!module) return;
	APE_CODEGEN_FREE(module->includes);
	for (size_t i = 0; i < module->func_count; i++) ape_cg_func_free(module->functions[i]);
	APE_CODEGEN_FREE(module->functions);
	APE_CODEGEN_FREE(module->typedefs);
	APE_CODEGEN_FREE(module->structs);
	APE_CODEGEN_FREE(module);
}

APE_CODEGEN_DEF void ape_cg_module_add_include(ApeCgModule *module, const char *path) {
	module->includes = (const char **)APE_CODEGEN_REALLOC(module->includes, sizeof(const char *) * (module->include_count + 1));
	module->includes[module->include_count++] = path;
}

APE_CODEGEN_DEF void ape_cg_module_add_function(ApeCgModule *module, ApeCgFunction *func) {
	module->functions = (ApeCgFunction **)APE_CODEGEN_REALLOC(module->functions, sizeof(ApeCgFunction *) * (module->func_count + 1));
	module->functions[module->func_count++] = func;
}

APE_CODEGEN_DEF void ape_cg_module_add_typedef(ApeCgModule *module, const char *name, const char *def) {
	(void)name;
	module->typedefs = (const char **)APE_CODEGEN_REALLOC(module->typedefs, sizeof(const char *) * (module->typedef_count + 1));
	module->typedefs[module->typedef_count++] = def;
}

APE_CODEGEN_DEF void ape_cg_module_add_struct(ApeCgModule *module, const char *struct_def) {
	module->structs = (const char **)APE_CODEGEN_REALLOC(module->structs, sizeof(const char *) * (module->struct_count + 1));
	module->structs[module->struct_count++] = struct_def;
}

APE_CODEGEN_DEF void ape_cg_module_emit(ApeCgModule *module, ApeCgBuffer *buf) {
	if (!module) return;

	/* File header */
	ape_cg_buf_append_fmt(buf, "/* %s - Generated by ape_codegen */\n\n", module->name);

	/* Includes */
	for (size_t i = 0; i < module->include_count; i++) {
		const char *inc = module->includes[i];
		if (inc[0] == '<')
			ape_cg_buf_append_fmt(buf, "#include %s\n", inc);
		else
			ape_cg_buf_append_fmt(buf, "#include \"%s\"\n", inc);
	}
	if (module->include_count > 0) ape_cg_buf_newline(buf);

	/* Typedefs */
	for (size_t i = 0; i < module->typedef_count; i++) {
		ape_cg_buf_append(buf, module->typedefs[i]);
		ape_cg_buf_newline(buf);
	}
	if (module->typedef_count > 0) ape_cg_buf_newline(buf);

	/* Struct definitions */
	for (size_t i = 0; i < module->struct_count; i++) {
		ape_cg_buf_append(buf, module->structs[i]);
		ape_cg_buf_newline(buf);
	}
	if (module->struct_count > 0) ape_cg_buf_newline(buf);

	/* Forward declarations */
	for (size_t i = 0; i < module->func_count; i++) ape_cg_func_emit_forward_decl(module->functions[i], buf);
	if (module->func_count > 0) ape_cg_buf_newline(buf);

	/* Function definitions */
	for (size_t i = 0; i < module->func_count; i++) {
		ape_cg_func_emit(module->functions[i], buf);
		ape_cg_buf_newline(buf);
	}
}

/* ============================================================================
 * Label Generator
 * ============================================================================ */

APE_CODEGEN_DEF void ape_cg_labelgen_init(ApeCgLabelGen *gen, const char *prefix) {
	gen->counter = 0;
	strncpy(gen->prefix, prefix ? prefix : "L", sizeof(gen->prefix) - 1);
	gen->prefix[sizeof(gen->prefix) - 1] = '\0';
}

APE_CODEGEN_DEF char *ape_cg_labelgen_new(ApeCgLabelGen *gen) {
	char tmp[64];
	snprintf(tmp, sizeof(tmp), "%s%d", gen->prefix, gen->counter++);
	return ape_cg_strdup(tmp);
}

APE_CODEGEN_DEF char *ape_cg_labelgen_named(ApeCgLabelGen *gen, const char *name) {
	char tmp[128];
	snprintf(tmp, sizeof(tmp), "%s%s_%d", gen->prefix, name, gen->counter++);
	return ape_cg_strdup(tmp);
}

/* ============================================================================
 * Utilities
 * ============================================================================ */

APE_CODEGEN_DEF const char *ape_cg_binop_str(ApeCgBinOp op) {
	switch (op) {
	case APE_CG_BINOP_ADD: return "+";
	case APE_CG_BINOP_SUB: return "-";
	case APE_CG_BINOP_MUL: return "*";
	case APE_CG_BINOP_DIV: return "/";
	case APE_CG_BINOP_MOD: return "%";
	case APE_CG_BINOP_AND: return "&";
	case APE_CG_BINOP_OR: return "|";
	case APE_CG_BINOP_XOR: return "^";
	case APE_CG_BINOP_SHL: return "<<";
	case APE_CG_BINOP_SHR: return ">>";
	case APE_CG_BINOP_EQ: return "==";
	case APE_CG_BINOP_NE: return "!=";
	case APE_CG_BINOP_LT: return "<";
	case APE_CG_BINOP_LE: return "<=";
	case APE_CG_BINOP_GT: return ">";
	case APE_CG_BINOP_GE: return ">=";
	case APE_CG_BINOP_LOG_AND: return "&&";
	case APE_CG_BINOP_LOG_OR: return "||";
	}
	return "??";
}

APE_CODEGEN_DEF const char *ape_cg_unop_str(ApeCgUnOp op) {
	switch (op) {
	case APE_CG_UNOP_NEG: return "-";
	case APE_CG_UNOP_POS: return "+";
	case APE_CG_UNOP_NOT: return "!";
	case APE_CG_UNOP_BNOT: return "~";
	case APE_CG_UNOP_PRE_INC: return "++";
	case APE_CG_UNOP_PRE_DEC: return "--";
	case APE_CG_UNOP_POST_INC: return "++";
	case APE_CG_UNOP_POST_DEC: return "--";
	case APE_CG_UNOP_ADDR: return "&";
	case APE_CG_UNOP_DEREF: return "*";
	}
	return "??";
}

APE_CODEGEN_DEF const char *ape_cg_typekind_str(ApeCgTypeKind kind) {
	switch (kind) {
	case APE_CG_TYPE_VOID: return "void";
	case APE_CG_TYPE_BOOL: return "bool";
	case APE_CG_TYPE_CHAR: return "char";
	case APE_CG_TYPE_SCHAR: return "signed char";
	case APE_CG_TYPE_UCHAR: return "unsigned char";
	case APE_CG_TYPE_SHORT: return "short";
	case APE_CG_TYPE_USHORT: return "unsigned short";
	case APE_CG_TYPE_INT: return "int";
	case APE_CG_TYPE_UINT: return "unsigned int";
	case APE_CG_TYPE_LONG: return "long";
	case APE_CG_TYPE_ULONG: return "unsigned long";
	case APE_CG_TYPE_LLONG: return "long long";
	case APE_CG_TYPE_ULLONG: return "unsigned long long";
	case APE_CG_TYPE_FLOAT: return "float";
	case APE_CG_TYPE_DOUBLE: return "double";
	case APE_CG_TYPE_PTR: return "pointer";
	case APE_CG_TYPE_ARRAY: return "array";
	case APE_CG_TYPE_FUNC: return "function";
	case APE_CG_TYPE_STRUCT: return "struct";
	case APE_CG_TYPE_UNION: return "union";
	case APE_CG_TYPE_ENUM: return "enum";
	case APE_CG_TYPE_TYPEDEF: return "typedef";
	case APE_CG_TYPE_CUSTOM: return "custom";
	}
	return "unknown";
}

APE_CODEGEN_DEF char *ape_cg_escape_string(const char *str) {
	if (!str) return ape_cg_strdup("");
	size_t len = strlen(str);
	/* Worst case: every char becomes a 4-char escape */
	char *out = (char *)APE_CODEGEN_MALLOC(len * 4 + 1);
	size_t j = 0;
	for (size_t i = 0; i < len; i++) {
		switch (str[i]) {
		case '\n':
			out[j++] = '\\';
			out[j++] = 'n';
			break;
		case '\t':
			out[j++] = '\\';
			out[j++] = 't';
			break;
		case '\r':
			out[j++] = '\\';
			out[j++] = 'r';
			break;
		case '\\':
			out[j++] = '\\';
			out[j++] = '\\';
			break;
		case '"':
			out[j++] = '\\';
			out[j++] = '"';
			break;
		case '\0':
			out[j++] = '\\';
			out[j++] = '0';
			break;
		default:
			if (str[i] < 32) {
				j += (size_t)sprintf(out + j, "\\x%02x", (unsigned char)str[i]);
			} else {
				out[j++] = str[i];
			}
			break;
		}
	}
	out[j] = '\0';
	return out;
}

APE_CODEGEN_DEF char *ape_cg_mangle_name(const char *prefix, const char *name) {
	if (!prefix && !name) return ape_cg_strdup("");
	if (!prefix) return ape_cg_strdup(name);
	if (!name) return ape_cg_strdup(prefix);

	size_t plen = strlen(prefix);
	size_t nlen = strlen(name);
	char *result = (char *)APE_CODEGEN_MALLOC(plen + 1 + nlen + 1);
	memcpy(result, prefix, plen);
	result[plen] = '_';
	memcpy(result + plen + 1, name, nlen);
	result[plen + 1 + nlen] = '\0';
	return result;
}

APE_CODEGEN_DEF bool ape_cg_is_const_expr(ApeCgExpr *expr) {
	if (!expr) return false;
	switch (expr->kind) {
	case APE_CG_EXPR_LITERAL_INT:
	case APE_CG_EXPR_LITERAL_UINT:
	case APE_CG_EXPR_LITERAL_LONG:
	case APE_CG_EXPR_LITERAL_FLOAT:
	case APE_CG_EXPR_LITERAL_DOUBLE:
	case APE_CG_EXPR_LITERAL_CHAR:
	case APE_CG_EXPR_LITERAL_BOOL: return true;
	case APE_CG_EXPR_BINARY: return ape_cg_is_const_expr(expr->data.binary.left) && ape_cg_is_const_expr(expr->data.binary.right);
	case APE_CG_EXPR_UNARY: return ape_cg_is_const_expr(expr->data.unary.operand);
	default: return false;
	}
}

APE_CODEGEN_DEF int64_t ape_cg_eval_const_int(ApeCgExpr *expr) {
	if (!expr) return 0;
	switch (expr->kind) {
	case APE_CG_EXPR_LITERAL_INT: return expr->data.int_val;
	case APE_CG_EXPR_LITERAL_UINT: return (int64_t)expr->data.uint_val;
	case APE_CG_EXPR_LITERAL_BOOL: return expr->data.bool_val ? 1 : 0;
	case APE_CG_EXPR_LITERAL_CHAR: return (int64_t)expr->data.char_val;
	case APE_CG_EXPR_BINARY: {
		int64_t l = ape_cg_eval_const_int(expr->data.binary.left);
		int64_t r = ape_cg_eval_const_int(expr->data.binary.right);
		switch (expr->data.binary.op) {
		case APE_CG_BINOP_ADD: return l + r;
		case APE_CG_BINOP_SUB: return l - r;
		case APE_CG_BINOP_MUL: return l * r;
		case APE_CG_BINOP_DIV: return r != 0 ? l / r : 0;
		case APE_CG_BINOP_MOD: return r != 0 ? l % r : 0;
		case APE_CG_BINOP_AND: return l & r;
		case APE_CG_BINOP_OR: return l | r;
		case APE_CG_BINOP_XOR: return l ^ r;
		case APE_CG_BINOP_SHL: return l << r;
		case APE_CG_BINOP_SHR: return l >> r;
		case APE_CG_BINOP_EQ: return l == r;
		case APE_CG_BINOP_NE: return l != r;
		case APE_CG_BINOP_LT: return l < r;
		case APE_CG_BINOP_LE: return l <= r;
		case APE_CG_BINOP_GT: return l > r;
		case APE_CG_BINOP_GE: return l >= r;
		case APE_CG_BINOP_LOG_AND: return l && r;
		case APE_CG_BINOP_LOG_OR: return l || r;
		}
		return 0;
	}
	case APE_CG_EXPR_UNARY: {
		int64_t v = ape_cg_eval_const_int(expr->data.unary.operand);
		switch (expr->data.unary.op) {
		case APE_CG_UNOP_NEG: return -v;
		case APE_CG_UNOP_POS: return v;
		case APE_CG_UNOP_NOT: return !v;
		case APE_CG_UNOP_BNOT: return ~v;
		default: return 0;
		}
	}
	default: return 0;
	}
}
/* END ape_codegen.c */

#endif

#endif
