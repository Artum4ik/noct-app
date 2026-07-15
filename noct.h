
#ifndef NOCT_H
#define NOCT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>

// ---- ТИПЫ ДАННЫХ ----
typedef enum {
    TYPE_INTEGER,
    TYPE_FLOAT,
    TYPE_STRING,
    TYPE_CHAR,
    TYPE_BOOL,
    TYPE_VOID,
    TYPE_LIST,
    TYPE_DICT,
    TYPE_SET,
    TYPE_TUPLE,
    TYPE_FUNCTION,
    TYPE_CLASS,
    TYPE_STRUCT,
    TYPE_NULL
} DataType;

// ---- ТОКЕНЫ ----
typedef enum {
    TOK_NUMBER, TOK_STRING, TOK_IDENTIFIER,
    TOK_NEW, TOK_CONST, TOK_FUNC, TOK_CLASS, TOK_STRUCT,
    TOK_ENUM, TOK_UNION, TOK_INTERFACE, TOK_NAMESPACE,
    TOK_IF, TOK_ELSE, TOK_MATCH, TOK_CASE,
    TOK_WRITELN, TOK_WRITE,
    TOK_WHILE, TOK_ITER, TOK_RETURN, TOK_BREAK, TOK_CONTINUE,
    TOK_ASYNC, TOK_AWAIT, TOK_TRY, TOK_CATCH, TOK_FINALLY, TOK_THROW,
    TOK_TAKE, TOK_FROM, TOK_AS, TOK_EXPORT, TOK_IMPORT,
    TOK_STATIC, TOK_EXTENDS, TOK_IMPLEMENTS, TOK_ABSTRACT, TOK_PROPERTY,
    TOK_TRUE, TOK_FALSE, TOK_VOID,
    TOK_INTEGER, TOK_FLOAT, TOK_STRING_TYPE, TOK_CHAR_TYPE, TOK_BOOL_TYPE,
    TOK_LIST, TOK_DICT, TOK_SET, TOK_TUPLE, TOK_RANGE,
    TOK_EQUALS, TOK_PLUS_EQ, TOK_MINUS_EQ, TOK_STAR_EQ, TOK_SLASH_EQ,
    TOK_DOUBLE_SLASH_EQ, TOK_PERCENT_EQ, TOK_DOUBLE_STAR_EQ,
    TOK_PLUS_PLUS, TOK_MINUS_MINUS,
    TOK_PLUS, TOK_MINUS, TOK_STAR, TOK_SLASH, TOK_DOUBLE_SLASH,
    TOK_PERCENT, TOK_DOUBLE_STAR,
    TOK_EQ_EQ, TOK_NOT_EQ, TOK_GT, TOK_LT, TOK_GT_EQ, TOK_LT_EQ,
    TOK_AND_AND, TOK_OR_OR, TOK_NOT,
    TOK_ELVIS, TOK_ELVIS_EQ, TOK_SAFE_DOT,
    TOK_LPAREN, TOK_RPAREN, TOK_LBRACE, TOK_RBRACE,
    TOK_LBRACKET, TOK_RBRACKET,
    TOK_COLON, TOK_SEMICOLON, TOK_COMMA, TOK_DOT,
    TOK_ARROW, TOK_PIPE, TOK_INDENT, TOK_DEDENT, TOK_EOF
} TokenType;

typedef struct {
    TokenType type;
    char text[256];
    int int_value;
    double float_value;
    char str_value[256];
} Token;

// ---- ПЕРЕМЕННЫЕ ----
typedef struct Variable {
    char name[256];
    DataType type;
    union {
        int int_val;
        double float_val;
        char str_val[4096];
        char char_val;
        bool bool_val;
        struct Variable* list_items;
        struct Variable* dict_keys;
        struct Variable* dict_values;
        struct Variable* set_items;
        struct Variable* tuple_items;
    };
    bool is_null;
    struct Variable* next;
} Variable;

// ---- ФУНКЦИИ ----
typedef struct Function {
    char name[256];
    DataType return_type;
    Token* body;
    int body_start;
    int body_end;
    char** param_names;
    DataType* param_types;
    Variable** param_defaults;
    int param_count;
    bool is_async;
    bool is_varargs;
    struct Function* next;
} Function;

// ---- КЛАССЫ ----
typedef struct Class {
    char name[256];
    Variable* fields;
    Function* methods;
    struct Class* parent;
    char** interfaces;
    int interface_count;
    struct Class* next;
} Class;

// ---- ЛЯМБДЫ ----
typedef struct Lambda {
    Token* body;
    int body_start;
    int body_end;
    char** param_names;
    int param_count;
    struct Lambda* next;
} Lambda;

// ---- БИБЛИОТЕКИ ----
typedef struct Library {
    char name[256];
    void (*init)(struct Interpreter*);
    struct Library* next;
} Library;

// ---- ОПЦИОНАЛЬНЫЕ ТИПЫ ----
typedef struct Optional {
    bool has_value;
    Variable value;
} Optional;

// ---- ИСКЛЮЧЕНИЯ ----
typedef struct Exception {
    char message[256];
    struct Exception* next;
} Exception;

// ---- АСИНХРОННОСТЬ ----
typedef struct Future {
    bool is_ready;
    Variable* result;
    void (*task)(struct Future*);
    struct Future* next;
} Future;

// ---- КОНТЕКСТ ВЫПОЛНЕНИЯ ----
typedef struct Context {
    Variable* variables;
    struct Context* parent;
} Context;

// ---- ИНТЕРПРЕТАТОР ----
typedef struct Interpreter {
    Variable* variables;
    Function* functions;
    Class* classes;
    Lambda* lambdas;
    Library* libraries;
    Context* call_stack;
    Future* futures;
    char output[4096];
    bool has_error;
    char error_msg[256];
    bool in_function;
    Variable* return_value;
    bool in_loop;
    bool break_loop;
    bool continue_loop;
    bool has_returned;
    Variable* last_result;
    int loop_depth;
    Exception* exception;
    bool is_async;
    bool is_awaiting;
} Interpreter;

// ---- ФУНКЦИИ ИНТЕРПРЕТАТОРА ----
int tokenize(const char* code, Token* tokens, int max_tokens);
int parse_and_execute(Token* tokens, int token_count, Interpreter* interp);
void init_interpreter(Interpreter* interp);
void free_interpreter(Interpreter* interp);
char* run_noct(const char* code);

// ---- ВСТРОЕННЫЕ ФУНКЦИИ ----
int noct_len(Variable* var);
char* noct_to_str(Variable* var);
double noct_to_float(Variable* var);
int noct_to_int(Variable* var);
bool noct_is_null(Variable* var);
Variable* noct_create_list();
Variable* noct_create_dict();
Variable* noct_create_set();

#endif
