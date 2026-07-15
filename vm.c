
#include "noct.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>

// ---- ИНИЦИАЛИЗАЦИЯ ----
void init_interpreter(Interpreter* interp) {
    interp->variables = NULL;
    interp->functions = NULL;
    interp->classes = NULL;
    interp->lambdas = NULL;
    interp->libraries = NULL;
    interp->call_stack = NULL;
    interp->futures = NULL;
    interp->output[0] = '\0';
    interp->has_error = false;
    interp->error_msg[0] = '\0';
    interp->in_function = false;
    interp->return_value = NULL;
    interp->in_loop = false;
    interp->break_loop = false;
    interp->continue_loop = false;
    interp->has_returned = false;
    interp->last_result = NULL;
    interp->loop_depth = 0;
    interp->exception = NULL;
    interp->is_async = false;
    interp->is_awaiting = false;
    srand(time(NULL));
}

// ---- РАБОТА С ПЕРЕМЕННЫМИ ----
Variable* find_variable(Interpreter* interp, const char* name) {
    Context* ctx = interp->call_stack;
    while (ctx) {
        Variable* var = ctx->variables;
        while (var) {
            if (strcmp(var->name, name) == 0) return var;
            var = var->next;
        }
        ctx = ctx->parent;
    }
    Variable* var = interp->variables;
    while (var) {
        if (strcmp(var->name, name) == 0) return var;
        var = var->next;
    }
    return NULL;
}

void set_variable(Interpreter* interp, const char* name, Variable* value) {
    Variable* var = find_variable(interp, name);
    if (!var) {
        var = malloc(sizeof(Variable));
        strcpy(var->name, name);
        var->next = interp->variables;
        interp->variables = var;
    }
    memcpy(var, value, sizeof(Variable));
    strcpy(var->name, name);
}

// ---- ВСТРОЕННЫЕ ФУНКЦИИ ----
int noct_len(Variable* var) {
    if (!var) return 0;
    if (var->type == TYPE_STRING) return strlen(var->str_val);
    if (var->type == TYPE_LIST) {
        int count = 0;
        Variable* item = var->list_items;
        while (item) { count++; item = item->next; }
        return count;
    }
    if (var->type == TYPE_DICT) {
        int count = 0;
        Variable* key = var->dict_keys;
        while (key) { count++; key = key->next; }
        return count;
    }
    if (var->type == TYPE_SET) {
        int count = 0;
        Variable* item = var->set_items;
        while (item) { count++; item = item->next; }
        return count;
    }
    if (var->type == TYPE_TUPLE) {
        int count = 0;
        Variable* item = var->tuple_items;
        while (item) { count++; item = item->next; }
        return count;
    }
    return 0;
}

char* noct_to_str(Variable* var) {
    static char buffer[4096];
    if (!var || var->is_null) {
        strcpy(buffer, "null");
        return buffer;
    }
    switch (var->type) {
        case TYPE_INTEGER: sprintf(buffer, "%d", var->int_val); break;
        case TYPE_FLOAT: sprintf(buffer, "%g", var->float_val); break;
        case TYPE_STRING: strcpy(buffer, var->str_val); break;
        case TYPE_CHAR: sprintf(buffer, "%c", var->char_val); break;
        case TYPE_BOOL: strcpy(buffer, var->bool_val ? "true" : "false"); break;
        case TYPE_LIST: {
            strcpy(buffer, "[");
            Variable* item = var->list_items;
            while (item) {
                char* s = noct_to_str(item);
                strcat(buffer, s);
                if (item->next) strcat(buffer, ", ");
                item = item->next;
            }
            strcat(buffer, "]");
            break;
        }
        case TYPE_DICT: {
            strcpy(buffer, "{");
            Variable* key = var->dict_keys;
            Variable* val = var->dict_values;
            while (key && val) {
                char* ks = noct_to_str(key);
                char* vs = noct_to_str(val);
                strcat(buffer, ks);
                strcat(buffer, ": ");
                strcat(buffer, vs);
                if (key->next) strcat(buffer, ", ");
                key = key->next;
                val = val->next;
            }
            strcat(buffer, "}");
            break;
        }
        case TYPE_SET: {
            strcpy(buffer, "{");
            Variable* item = var->set_items;
            while (item) {
                char* s = noct_to_str(item);
                strcat(buffer, s);
                if (item->next) strcat(buffer, ", ");
                item = item->next;
            }
            strcat(buffer, "}");
            break;
        }
        case TYPE_TUPLE: {
            strcpy(buffer, "(");
            Variable* item = var->tuple_items;
            while (item) {
                char* s = noct_to_str(item);
                strcat(buffer, s);
                if (item->next) strcat(buffer, ", ");
                item = item->next;
            }
            strcat(buffer, ")");
            break;
        }
        default: sprintf(buffer, "unknown(%d)", var->type);
    }
    return buffer;
}

int noct_to_int(Variable* var) {
    if (!var || var->is_null) return 0;
    switch (var->type) {
        case TYPE_INTEGER: return var->int_val;
        case TYPE_FLOAT: return (int)var->float_val;
        case TYPE_STRING: return atoi(var->str_val);
        case TYPE_CHAR: return (int)var->char_val;
        case TYPE_BOOL: return var->bool_val ? 1 : 0;
        default: return 0;
    }
}

double noct_to_float(Variable* var) {
    if (!var || var->is_null) return 0.0;
    switch (var->type) {
        case TYPE_INTEGER: return (double)var->int_val;
        case TYPE_FLOAT: return var->float_val;
        case TYPE_STRING: return atof(var->str_val);
        case TYPE_CHAR: return (double)var->char_val;
        case TYPE_BOOL: return var->bool_val ? 1.0 : 0.0;
        default: return 0.0;
    }
}

bool noct_is_null(Variable* var) {
    return var == NULL || var->is_null;
}

Variable* noct_create_list() {
    Variable* list = malloc(sizeof(Variable));
    list->type = TYPE_LIST;
    list->list_items = NULL;
    list->is_null = false;
    list->next = NULL;
    return list;
}

Variable* noct_create_dict() {
    Variable* dict = malloc(sizeof(Variable));
    dict->type = TYPE_DICT;
    dict->dict_keys = NULL;
    dict->dict_values = NULL;
    dict->is_null = false;
    dict->next = NULL;
    return dict;
}

Variable* noct_create_set() {
    Variable* set = malloc(sizeof(Variable));
    set->type = TYPE_SET;
    set->set_items = NULL;
    set->is_null = false;
    set->next = NULL;
    return set;
}

// ---- ВЫПОЛНЕНИЕ ВЫРАЖЕНИЙ (evaluate_expression) ----
Variable* evaluate_expression(Interpreter* interp, Token* tokens, int* pos) {
    if (*pos >= 5000) {
        interp->has_error = true;
        strcpy(interp->error_msg, "Expression too complex");
        return NULL;
    }

    Token* token = &tokens[*pos];
    Variable* result = malloc(sizeof(Variable));
    result->type = TYPE_NULL;
    result->is_null = true;
    result->next = NULL;

    if (token->type == TOK_NUMBER) {
        if (token->float_value != 0) {
            result->type = TYPE_FLOAT;
            result->float_val = token->float_value;
        } else {
            result->type = TYPE_INTEGER;
            result->int_val = token->int_value;
        }
        result->is_null = false;
        (*pos)++;
        return result;
    }

    if (token->type == TOK_STRING) {
        result->type = TYPE_STRING;
        strcpy(result->str_val, token->str_value);
        result->is_null = false;
        (*pos)++;
        return result;
    }

    if (token->type == TOK_TRUE) {
        result->type = TYPE_BOOL;
        result->bool_val = true;
        result->is_null = false;
        (*pos)++;
        return result;
    }
    if (token->type == TOK_FALSE) {
        result->type = TYPE_BOOL;
        result->bool_val = false;
        result->is_null = false;
        (*pos)++;
        return result;
    }

    if (token->type == TOK_IDENTIFIER) {
        Variable* var = find_variable(interp, token->text);
        if (var) {
            memcpy(result, var, sizeof(Variable));
            result->is_null = false;
            result->next = NULL;
        }
        (*pos)++;
        return result;
    }

    if (token->type == TOK_LBRACKET) {
        (*pos)++;
        result = noct_create_list();
        Variable* prev = NULL;
        while (*pos < 5000 && tokens[*pos].type != TOK_RBRACKET) {
            Variable* item = evaluate_expression(interp, tokens, pos);
            if (item) {
                if (!result->list_items) {
                    result->list_items = item;
                } else {
                    prev->next = item;
                }
                prev = item;
            }
            if (*pos < 5000 && tokens[*pos].type == TOK_COMMA) (*pos)++;
        }
        if (*pos < 5000 && tokens[*pos].type == TOK_RBRACKET) (*pos)++;
        return result;
    }

    if (token->type == TOK_LBRACE) {
        (*pos)++;
        result = noct_create_dict();
        Variable* prev_key = NULL;
        Variable* prev_val = NULL;
        while (*pos < 5000 && tokens[*pos].type != TOK_RBRACE) {
            Variable* key = evaluate_expression(interp, tokens, pos);
            if (*pos < 5000 && tokens[*pos].type == TOK_COLON) (*pos)++;
            Variable* val = evaluate_expression(interp, tokens, pos);
            if (key && val) {
                if (!result->dict_keys) {
                    result->dict_keys = key;
                    result->dict_values = val;
                } else {
                    prev_key->next = key;
                    prev_val->next = val;
                }
                prev_key = key;
                prev_val = val;
            }
            if (*pos < 5000 && tokens[*pos].type == TOK_COMMA) (*pos)++;
        }
        if (*pos < 5000 && tokens[*pos].type == TOK_RBRACE) (*pos)++;
        return result;
    }

    if (token->type == TOK_RANGE) {
        (*pos)++;
        if (tokens[*pos].type == TOK_LPAREN) (*pos)++;
        Variable* start = evaluate_expression(interp, tokens, pos);
        if (*pos < 5000 && tokens[*pos].type == TOK_COMMA) (*pos)++;
        Variable* end = evaluate_expression(interp, tokens, pos);
        if (*pos < 5000 && tokens[*pos].type == TOK_RPAREN) (*pos)++;

        result = noct_create_list();
        for (int i = noct_to_int(start); i < noct_to_int(end); i++) {
            Variable* item = malloc(sizeof(Variable));
            item->type = TYPE_INTEGER;
            item->int_val = i;
            item->next = result->list_items;
            result->list_items = item;
        }
        return result;
    }

    // ---- ЛЯМБДА (->) ----
    if (token->type == TOK_LPAREN) {
        (*pos)++;
        Lambda* lambda = malloc(sizeof(Lambda));
        lambda->param_names = malloc(10 * sizeof(char*));
        lambda->param_count = 0;

        while (*pos < 5000 && tokens[*pos].type != TOK_RPAREN) {
            if (tokens[*pos].type == TOK_IDENTIFIER) {
                lambda->param_names[lambda->param_count] = strdup(tokens[*pos].text);
                lambda->param_count++;
                (*pos)++;
            }
            if (*pos < 5000 && tokens[*pos].type == TOK_COMMA) (*pos)++;
        }
        if (*pos < 5000 && tokens[*pos].type == TOK_RPAREN) (*pos)++;
        if (*pos < 5000 && tokens[*pos].type == TOK_ARROW) (*pos)++;

        lambda->body_start = *pos;
        lambda->body_end = *pos;
        if (*pos < 5000 && tokens[*pos].type == TOK_LBRACE) {
            int brace_count = 1;
            (*pos)++;
            while (*pos < 5000 && brace_count > 0) {
                if (tokens[*pos].type == TOK_LBRACE) brace_count++;
                if (tokens[*pos].type == TOK_RBRACE) brace_count--;
                (*pos)++;
            }
            lambda->body_end = *pos;
        } else {
            while (*pos < 5000 && tokens[*pos].type != TOK_EOF &&
                   tokens[*pos].type != TOK_RPAREN && tokens[*pos].type != TOK_SEMICOLON) {
                (*pos)++;
            }
            lambda->body_end = *pos;
        }

        lambda->next = interp->lambdas;
        interp->lambdas = lambda;

        Context* old_context = interp->call_stack;
        Context* new_context = malloc(sizeof(Context));
        new_context->parent = old_context;
        new_context->variables = NULL;
        interp->call_stack = new_context;

        int saved_pos = lambda->body_start;
        interp->in_function = true;
        interp->return_value = NULL;

        parse_and_execute(tokens, saved_pos, interp);

        interp->call_stack = old_context;
        interp->in_function = false;

        if (interp->return_value) {
            result = interp->return_value;
            interp->return_value = NULL;
            result->is_null = false;
        }
        return result;
    }

    // ---- АРИФМЕТИКА ----
    if (token->type == TOK_PLUS || token->type == TOK_MINUS ||
        token->type == TOK_STAR || token->type == TOK_SLASH ||
        token->type == TOK_DOUBLE_SLASH || token->type == TOK_PERCENT ||
        token->type == TOK_DOUBLE_STAR) {

        TokenType op = token->type;
        (*pos)++;
        Variable* left = evaluate_expression(interp, tokens, pos);
        Variable* right = evaluate_expression(interp, tokens, pos);

        if (!left || !right) {
            free(result);
            return NULL;
        }

        if (op == TOK_PLUS) {
            if (left->type == TYPE_STRING || right->type == TYPE_STRING) {
                char str1[4096], str2[4096];
                strcpy(str1, noct_to_str(left));
                strcpy(str2, noct_to_str(right));
                result->type = TYPE_STRING;
                sprintf(result->str_val, "%s%s", str1, str2);
                result->is_null = false;
            } else {
                result->type = TYPE_FLOAT;
                result->float_val = noct_to_float(left) + noct_to_float(right);
                result->is_null = false;
            }
        }
        else if (op == TOK_MINUS) {
            result->type = TYPE_FLOAT;
            result->float_val = noct_to_float(left) - noct_to_float(right);
            result->is_null = false;
        }
        else if (op == TOK_STAR) {
            result->type = TYPE_FLOAT;
            result->float_val = noct_to_float(left) * noct_to_float(right);
            result->is_null = false;
        }
        else if (op == TOK_SLASH) {
            if (noct_to_float(right) != 0) {
                result->type = TYPE_FLOAT;
                result->float_val = noct_to_float(left) / noct_to_float(right);
                result->is_null = false;
            } else {
                interp->has_error = true;
                strcpy(interp->error_msg, "Division by zero");
                free(left); free(right);
                return result;
            }
        }
        else if (op == TOK_DOUBLE_SLASH) {
            if (noct_to_int(right) != 0) {
                result->type = TYPE_INTEGER;
                result->int_val = noct_to_int(left) / noct_to_int(right);
                result->is_null = false;
            } else {
                interp->has_error = true;
                strcpy(interp->error_msg, "Division by zero");
                free(left); free(right);
                return result;
            }
        }
        else if (op == TOK_PERCENT) {
            if (noct_to_int(right) != 0) {
                result->type = TYPE_INTEGER;
                result->int_val = noct_to_int(left) % noct_to_int(right);
                result->is_null = false;
            } else {
                interp->has_error = true;
                strcpy(interp->error_msg, "Modulo by zero");
                free(left); free(right);
                return result;
            }
        }
        else if (op == TOK_DOUBLE_STAR) {
            result->type = TYPE_FLOAT;
            result->float_val = pow(noct_to_float(left), noct_to_float(right));
            result->is_null = false;
        }

        free(left);
        free(right);
        return result;
    }

    // ---- ЛОГИКА ----
    if (token->type == TOK_AND_AND || token->type == TOK_OR_OR ||
        token->type == TOK_EQ_EQ || token->type == TOK_NOT_EQ ||
        token->type == TOK_GT || token->type == TOK_LT ||
        token->type == TOK_GT_EQ || token->type == TOK_LT_EQ) {

        TokenType op = token->type;
        (*pos)++;
        Variable* left = evaluate_expression(interp, tokens, pos);
        Variable* right = evaluate_expression(interp, tokens, pos);

        result->type = TYPE_BOOL;
        result->is_null = false;

        double l = noct_to_float(left);
        double r = noct_to_float(right);

        switch (op) {
            case TOK_AND_AND: result->bool_val = (l != 0 && r != 0); break;
            case TOK_OR_OR: result->bool_val = (l != 0 || r != 0); break;
            case TOK_EQ_EQ: result->bool_val = (l == r); break;
            case TOK_NOT_EQ: result->bool_val = (l != r); break;
            case TOK_GT: result->bool_val = (l > r); break;
            case TOK_LT: result->bool_val = (l < r); break;
            case TOK_GT_EQ: result->bool_val = (l >= r); break;
            case TOK_LT_EQ: result->bool_val = (l <= r); break;
            default: break;
        }

        free(left);
        free(right);
        return result;
    }

    // ---- ELVIS (??) ----
    if (token->type == TOK_ELVIS) {
        Variable* left = evaluate_expression(interp, tokens, pos);
        (*pos)++;
        if (noct_is_null(left)) {
            Variable* right = evaluate_expression(interp, tokens, pos);
            return right;
        }
        return left;
    }

    // ---- SAFE DOT (?. ) ----
    if (token->type == TOK_SAFE_DOT) {
        Variable* obj = evaluate_expression(interp, tokens, pos);
        if (noct_is_null(obj)) {
            result->is_null = true;
            return result;
        }
        return obj;
    }

    // ---- NOT ----
    if (token->type == TOK_NOT) {
        (*pos)++;
        Variable* operand = evaluate_expression(interp, tokens, pos);
        result->type = TYPE_BOOL;
        result->bool_val = !noct_to_int(operand);
        result->is_null = false;
        free(operand);
        return result;
    }

    return result;
}

// ---- ВЫПОЛНЕНИЕ КОДА (parse_and_execute) ----
int parse_and_execute(Token* tokens, int token_count, Interpreter* interp) {
    int pos = 0;
    while (pos < token_count && tokens[pos].type != TOK_EOF) {
        Token token = tokens[pos];

        if (token.type == TOK_WRITELN) {
            pos++;
            if (pos < token_count && tokens[pos].type == TOK_LPAREN) pos++;
            Variable* expr = evaluate_expression(interp, tokens, &pos);
            if (expr && !expr->is_null) {
                char* str = noct_to_str(expr);
                strcat(interp->output, str);
                strcat(interp->output, "\n");
                free(expr);
            }
            if (pos < token_count && tokens[pos].type == TOK_RPAREN) pos++;
            if (pos < token_count && tokens[pos].type == TOK_SEMICOLON) pos++;
            continue;
        }

        if (token.type == TOK_WRITE) {
            pos++;
            if (pos < token_count && tokens[pos].type == TOK_LPAREN) pos++;
            Variable* expr = evaluate_expression(interp, tokens, &pos);
            if (expr && !expr->is_null) {
                char* str = noct_to_str(expr);
                strcat(interp->output, str);
                free(expr);
            }
            if (pos < token_count && tokens[pos].type == TOK_RPAREN) pos++;
            if (pos < token_count && tokens[pos].type == TOK_SEMICOLON) pos++;
            continue;
        }

        if (token.type == TOK_NEW) {
            pos++;
            if (pos < token_count && tokens[pos].type == TOK_IDENTIFIER) {
                char var_name[256];
                strcpy(var_name, tokens[pos].text);
                pos++;
                if (pos < token_count && tokens[pos].type == TOK_COLON) pos++;
                DataType type = TYPE_NULL;
                if (pos < token_count) {
                    if (tokens[pos].type == TOK_INTEGER) type = TYPE_INTEGER;
                    else if (tokens[pos].type == TOK_FLOAT) type = TYPE_FLOAT;
                    else if (tokens[pos].type == TOK_STRING_TYPE) type = TYPE_STRING;
                    else if (tokens[pos].type == TOK_CHAR_TYPE) type = TYPE_CHAR;
                    else if (tokens[pos].type == TOK_BOOL_TYPE) type = TYPE_BOOL;
                    else if (tokens[pos].type == TOK_LIST) type = TYPE_LIST;
                    else if (tokens[pos].type == TOK_DICT) type = TYPE_DICT;
                    else if (tokens[pos].type == TOK_SET) type = TYPE_SET;
                    else if (tokens[pos].type == TOK_TUPLE) type = TYPE_TUPLE;
                    pos++;
                }
                Variable* value = NULL;
                if (pos < token_count && tokens[pos].type == TOK_EQUALS) {
                    pos++;
                    value = evaluate_expression(interp, tokens, &pos);
                } else {
                    value = malloc(sizeof(Variable));
                    value->type = type;
                    value->is_null = true;
                }
                if (value) {
                    value->type = type;
                    set_variable(interp, var_name, value);
                    free(value);
                }
            }
            if (pos < token_count && tokens[pos].type == TOK_SEMICOLON) pos++;
            continue;
        }

        if (token.type == TOK_CONST) {
            pos++;
            if (pos < token_count && tokens[pos].type == TOK_IDENTIFIER) {
                char var_name[256];
                strcpy(var_name, tokens[pos].text);
                pos++;
                if (pos < token_count && tokens[pos].type == TOK_COLON) pos++;
                DataType type = TYPE_NULL;
                if (pos < token_count) {
                    if (tokens[pos].type == TOK_INTEGER) type = TYPE_INTEGER;
                    else if (tokens[pos].type == TOK_FLOAT) type = TYPE_FLOAT;
                    else if (tokens[pos].type == TOK_STRING_TYPE) type = TYPE_STRING;
                    else if (tokens[pos].type == TOK_CHAR_TYPE) type = TYPE_CHAR;
                    else if (tokens[pos].type == TOK_BOOL_TYPE) type = TYPE_BOOL;
                    pos++;
                }
                Variable* value = NULL;
                if (pos < token_count && tokens[pos].type == TOK_EQUALS) {
                    pos++;
                    value = evaluate_expression(interp, tokens, &pos);
                }
                if (value) {
                    value->type = type;
                    set_variable(interp, var_name, value);
                    free(value);
                }
            }
            if (pos < token_count && tokens[pos].type == TOK_SEMICOLON) pos++;
            continue;
        }

        if (token.type == TOK_IF) {
            pos++;
            if (pos < token_count && tokens[pos].type == TOK_LPAREN) pos++;
            Variable* cond = evaluate_expression(interp, tokens, &pos);
            if (pos < token_count && tokens[pos].type == TOK_RPAREN) pos++;

            bool condition = cond && cond->bool_val;
            free(cond);

            if (pos < token_count && tokens[pos].type == TOK_LBRACE) pos++;
            if (condition) {
                int brace_count = 1;
                while (pos < token_count && brace_count > 0) {
                    if (tokens[pos].type == TOK_LBRACE) brace_count++;
                    if (tokens[pos].type == TOK_RBRACE) brace_count--;
                    if (brace_count > 0) {
                        if (parse_and_execute(tokens, pos, interp) == 0) {
                            if (interp->return_value || interp->break_loop) return 0;
                        }
                        pos++;
                    }
                }
            } else {
                int brace_count = 1;
                while (pos < token_count && brace_count > 0) {
                    if (tokens[pos].type == TOK_LBRACE) brace_count++;
                    if (tokens[pos].type == TOK_RBRACE) brace_count--;
                    pos++;
                }
            }
            continue;
        }

        if (token.type == TOK_ELSE) {
            pos++;
            if (pos < token_count && tokens[pos].type == TOK_IF) {
                continue;
            }
            if (pos < token_count && tokens[pos].type == TOK_LBRACE) pos++;
            int brace_count = 1;
            while (pos < token_count && brace_count > 0) {
                if (tokens[pos].type == TOK_LBRACE) brace_count++;
                if (tokens[pos].type == TOK_RBRACE) brace_count--;
                pos++;
            }
            continue;
        }

        if (token.type == TOK_WHILE) {
            interp->in_loop = true;
            interp->break_loop = false;
            interp->continue_loop = false;

            pos++;
            int loop_start = pos;
            if (pos < token_count && tokens[pos].type == TOK_LPAREN) pos++;
            Variable* cond = evaluate_expression(interp, tokens, &pos);
            if (pos < token_count && tokens[pos].type == TOK_RPAREN) pos++;

            if (cond && cond->bool_val) {
                if (pos < token_count && tokens[pos].type == TOK_LBRACE) pos++;
                int brace_count = 1;
                int block_start = pos;
                int block_end = pos;

                int temp_pos = pos;
                while (temp_pos < token_count && brace_count > 0) {
                    if (tokens[temp_pos].type == TOK_LBRACE) brace_count++;
                    if (tokens[temp_pos].type == TOK_RBRACE) brace_count--;
                    if (brace_count > 0) temp_pos++;
                }
                block_end = temp_pos;

                while (cond && cond->bool_val && !interp->break_loop) {
                    pos = block_start;
                    while (pos < block_end && !interp->break_loop && !interp->continue_loop) {
                        if (parse_and_execute(tokens, pos, interp) == 0) {
                            if (interp->return_value) {
                                interp->in_loop = false;
                                return 0;
                            }
                        }
                        pos++;
                    }
                    interp->continue_loop = false;

                    int temp_pos2 = loop_start;
                    if (temp_pos2 < token_count && tokens[temp_pos2].type == TOK_LPAREN) temp_pos2++;
                    cond = evaluate_expression(interp, tokens, &temp_pos2);
                }
                pos = block_end + 1;
            }
            interp->in_loop = false;
            continue;
        }

        if (token.type == TOK_BREAK) {
            if (interp->in_loop) {
                interp->break_loop = true;
            }
            pos++;
            if (pos < token_count && tokens[pos].type == TOK_SEMICOLON) pos++;
            continue;
        }

        if (token.type == TOK_CONTINUE) {
            if (interp->in_loop) {
                interp->continue_loop = true;
            }
            pos++;
            if (pos < token_count && tokens[pos].type == TOK_SEMICOLON) pos++;
            continue;
        }

        if (token.type == TOK_MATCH) {
            pos++;
            Variable* value = evaluate_expression(interp, tokens, &pos);
            if (pos < token_count && tokens[pos].type == TOK_LBRACE) pos++;

            bool matched = false;
            while (pos < token_count && tokens[pos].type != TOK_RBRACE && !matched) {
                if (tokens[pos].type == TOK_CASE) {
                    pos++;
                    Variable* case_val = evaluate_expression(interp, tokens, &pos);
                    if (pos < token_count && tokens[pos].type == TOK_COLON) pos++;

                    if (noct_to_int(value) == noct_to_int(case_val)) {
                        while (pos < token_count && tokens[pos].type != TOK_CASE &&
                               tokens[pos].type != TOK_RBRACE) {
                            parse_and_execute(tokens, pos, interp);
                            pos++;
                        }
                        matched = true;
                    }
                }
                else if (tokens[pos].type == TOK_ELSE && !matched) {
                    pos++;
                    if (pos < token_count && tokens[pos].type == TOK_COLON) pos++;
                    while (pos < token_count && tokens[pos].type != TOK_RBRACE) {
                        parse_and_execute(tokens, pos, interp);
                        pos++;
                    }
                    matched = true;
                }
                pos++;
            }
            if (pos < token_count && tokens[pos].type == TOK_RBRACE) pos++;
            continue;
        }

        // ---- ИДЕНТИФИКАТОРЫ (ВЫЗОВ ФУНКЦИЙ ИЛИ ПРИСВАИВАНИЕ) ----
        if (token.type == TOK_IDENTIFIER) {
            char name[256];
            strcpy(name, token.text);
            pos++;

            if (pos < token_count && tokens[pos].type == TOK_LPAREN) {
                pos++;
                Function* func = interp->functions;
                while (func) {
                    if (strcmp(func->name, name) == 0) break;
                    func = func->next;
                }

                if (func) {
                    Context* old_context = interp->call_stack;
                    Context* new_context = malloc(sizeof(Context));
                    new_context->parent = old_context;
                    new_context->variables = NULL;
                    interp->call_stack = new_context;

                    for (int i = 0; i < func->param_count; i++) {
                        if (pos < token_count && tokens[pos].type == TOK_RPAREN) break;
                        Variable* arg = evaluate_expression(interp, tokens, &pos);
                        if (arg) {
                            Variable* param = malloc(sizeof(Variable));
                            memcpy(param, arg, sizeof(Variable));
                            strcpy(param->name, func->param_names[i]);
                            param->next = new_context->variables;
                            new_context->variables = param;
                            free(arg);
                        }
                        if (pos < token_count && tokens[pos].type == TOK_COMMA) pos++;
                    }
                    if (pos < token_count && tokens[pos].type == TOK_RPAREN) pos++;

                    interp->in_function = true;
                    interp->return_value = NULL;

                    int saved_pos = pos;
                    pos = func->body_start + 1;
                    parse_and_execute(tokens, pos, interp);
                    pos = saved_pos;

                    interp->call_stack = old_context;
                    interp->in_function = false;

                    if (interp->return_value) {
                        Variable* result = interp->return_value;
                        interp->return_value = NULL;
                        interp->last_result = result;
                    }
                }
                continue;
            }

            if (pos < token_count && tokens[pos].type == TOK_EQUALS) {
                pos++;
                Variable* value = evaluate_expression(interp, tokens, &pos);
                if (value) {
                    set_variable(interp, name, value);
                    free(value);
                }
                if (pos < token_count && tokens[pos].type == TOK_SEMICOLON) pos++;
                continue;
            }

            if (pos < token_count && (
                tokens[pos].type == TOK_PLUS_EQ || tokens[pos].type == TOK_MINUS_EQ ||
                tokens[pos].type == TOK_STAR_EQ || tokens[pos].type == TOK_SLASH_EQ ||
                tokens[pos].type == TOK_DOUBLE_SLASH_EQ || tokens[pos].type == TOK_PERCENT_EQ ||
                tokens[pos].type == TOK_DOUBLE_STAR_EQ)) {

                TokenType op = tokens[pos].type;
                pos++;
                Variable* value = evaluate_expression(interp, tokens, &pos);
                Variable* var = find_variable(interp, name);

                if (var) {
                    Variable* result = malloc(sizeof(Variable));
                    result->type = var->type;
                    result->is_null = false;
                    switch (op) {
                        case TOK_PLUS_EQ: result->int_val = var->int_val + noct_to_int(value); break;
                        case TOK_MINUS_EQ: result->int_val = var->int_val - noct_to_int(value); break;
                        case TOK_STAR_EQ: result->int_val = var->int_val * noct_to_int(value); break;
                        case TOK_SLASH_EQ:
                            if (noct_to_int(value) != 0)
                                result->int_val = var->int_val / noct_to_int(value);
                            break;
                        case TOK_DOUBLE_SLASH_EQ:
                            if (noct_to_int(value) != 0)
                                result->int_val = var->int_val / noct_to_int(value);
                            break;
                        case TOK_PERCENT_EQ:
                            if (noct_to_int(value) != 0)
                                result->int_val = var->int_val % noct_to_int(value);
                            break;
                        case TOK_DOUBLE_STAR_EQ:
                            result->int_val = pow(var->int_val, noct_to_int(value));
                            break;
                        default: break;
                    }
                    set_variable(interp, name, result);
                    free(result);
                }
                if (pos < token_count && tokens[pos].type == TOK_SEMICOLON) pos++;
                continue;
            }
        }

        pos++;
    }
    return 0;
}

// ---- ГЛАВНАЯ ФУНКЦИЯ ДЛЯ ВЫЗОВА ИЗ PYTHON (run_noct) ----
char* run_noct(const char* code) {
    Interpreter interp;
    init_interpreter(&interp);

    Token tokens[5000];
    int token_count = tokenize(code, tokens, 5000);
    parse_and_execute(tokens, token_count, &interp);

    if (interp.has_error) {
        char* result = malloc(strlen("Error: ") + strlen(interp.error_msg) + 1);
        sprintf(result, "Error: %s", interp.error_msg);
        return result;
    }

    char* result = malloc(strlen(interp.output) + 1);
    strcpy(result, interp.output);
    return result;
}

// ---- ОСВОБОЖДЕНИЕ ПАМЯТИ ----
void free_variable(Variable* var) {
    if (!var) return;
    if (var->type == TYPE_LIST) {
        Variable* item = var->list_items;
        while (item) {
            Variable* next = item->next;
            free(item);
            item = next;
        }
    }
    if (var->type == TYPE_DICT) {
        Variable* key = var->dict_keys;
        while (key) {
            Variable* next = key->next;
            free(key);
            key = next;
        }
        Variable* val = var->dict_values;
        while (val) {
            Variable* next = val->next;
            free(val);
            val = next;
        }
    }
    if (var->type == TYPE_SET) {
        Variable* item = var->set_items;
        while (item) {
            Variable* next = item->next;
            free(item);
            item = next;
        }
    }
    free(var);
}

void free_interpreter(Interpreter* interp) {
    Variable* var = interp->variables;
    while (var) {
        Variable* next = var->next;
        free_variable(var);
        var = next;
    }
}
