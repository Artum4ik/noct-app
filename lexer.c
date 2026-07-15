
#include "noct.h"
#include <string.h>

int tokenize(const char* code, Token* tokens, int max_tokens) {
    int pos = 0, t = 0;
    int indent_level = 0;
    int last_indent = 0;
    
    while (code[pos] != '\0' && t < max_tokens) {
        // ---- ОБРАБОТКА ОТСТУПОВ ----
        if (code[pos] == '\n') {
            pos++;
            int new_indent = 0;
            while (code[pos] == ' ') {
                new_indent++;
                pos++;
            }
            if (new_indent > last_indent) {
                tokens[t].type = TOK_INDENT;
                t++;
            }
            else if (new_indent < last_indent) {
                tokens[t].type = TOK_DEDENT;
                t++;
            }
            last_indent = new_indent;
            continue;
        }
        
        if (code[pos] == ' ' || code[pos] == '\t') {
            pos++;
            continue;
        }

        // ---- КОММЕНТАРИИ ----
        if (code[pos] == '#') {
            while (code[pos] != '\n' && code[pos] != '\0') pos++;
            continue;
        }
        if (code[pos] == '/' && code[pos+1] == '*') {
            pos += 2;
            while (code[pos] != '\0' && !(code[pos] == '*' && code[pos+1] == '/')) pos++;
            if (code[pos] == '*') pos += 2;
            continue;
        }

        // ---- СТРОКИ ----
        if (code[pos] == '"') {
            tokens[t].type = TOK_STRING;
            int i = 0;
            pos++;
            while (code[pos] != '"' && code[pos] != '\0') {
                if (code[pos] == '\\') {
                    pos++;
                    switch (code[pos]) {
                        case 'n': tokens[t].str_value[i++] = '\n'; break;
                        case 't': tokens[t].str_value[i++] = '\t'; break;
                        default: tokens[t].str_value[i++] = code[pos]; break;
                    }
                } else {
                    tokens[t].str_value[i++] = code[pos];
                }
                pos++;
            }
            tokens[t].str_value[i] = '\0';
            if (code[pos] == '"') pos++;
            t++;
            continue;
        }

        // ---- ЧИСЛА ----
        if (isdigit(code[pos]) || (code[pos] == '-' && isdigit(code[pos+1]))) {
            bool is_float = false;
            char num[64];
            int i = 0;
            if (code[pos] == '-') { num[i++] = code[pos++]; }
            while (isdigit(code[pos]) || code[pos] == '.') {
                if (code[pos] == '.') is_float = true;
                num[i++] = code[pos++];
            }
            num[i] = '\0';
            if (is_float) {
                tokens[t].type = TOK_NUMBER;
                tokens[t].float_value = atof(num);
            } else {
                tokens[t].type = TOK_NUMBER;
                tokens[t].int_value = atoi(num);
            }
            t++;
            continue;
        }

        // ---- ИДЕНТИФИКАТОРЫ И КЛЮЧЕВЫЕ СЛОВА ----
        if (isalpha(code[pos]) || code[pos] == '_') {
            int i = 0;
            char word[256];
            while (isalnum(code[pos]) || code[pos] == '_') {
                word[i++] = code[pos++];
            }
            word[i] = '\0';

            if (strcmp(word, "new") == 0) tokens[t].type = TOK_NEW;
            else if (strcmp(word, "const") == 0) tokens[t].type = TOK_CONST;
            else if (strcmp(word, "func") == 0) tokens[t].type = TOK_FUNC;
            else if (strcmp(word, "class") == 0) tokens[t].type = TOK_CLASS;
            else if (strcmp(word, "struct") == 0) tokens[t].type = TOK_STRUCT;
            else if (strcmp(word, "enum") == 0) tokens[t].type = TOK_ENUM;
            else if (strcmp(word, "union") == 0) tokens[t].type = TOK_UNION;
            else if (strcmp(word, "interface") == 0) tokens[t].type = TOK_INTERFACE;
            else if (strcmp(word, "namespace") == 0) tokens[t].type = TOK_NAMESPACE;
            else if (strcmp(word, "if") == 0) tokens[t].type = TOK_IF;
            else if (strcmp(word, "else") == 0) tokens[t].type = TOK_ELSE;
            else if (strcmp(word, "match") == 0) tokens[t].type = TOK_MATCH;
            else if (strcmp(word, "case") == 0) tokens[t].type = TOK_CASE;
            else if (strcmp(word, "while") == 0) tokens[t].type = TOK_WHILE;
            else if (strcmp(word, "iter") == 0) tokens[t].type = TOK_ITER;
            else if (strcmp(word, "return") == 0) tokens[t].type = TOK_RETURN;
            else if (strcmp(word, "break") == 0) tokens[t].type = TOK_BREAK;
            else if (strcmp(word, "continue") == 0) tokens[t].type = TOK_CONTINUE;
            else if (strcmp(word, "async") == 0) tokens[t].type = TOK_ASYNC;
            else if (strcmp(word, "await") == 0) tokens[t].type = TOK_AWAIT;
            else if (strcmp(word, "try") == 0) tokens[t].type = TOK_TRY;
            else if (strcmp(word, "catch") == 0) tokens[t].type = TOK_CATCH;
            else if (strcmp(word, "finally") == 0) tokens[t].type = TOK_FINALLY;
            else if (strcmp(word, "throw") == 0) tokens[t].type = TOK_THROW;
            else if (strcmp(word, "take") == 0) tokens[t].type = TOK_TAKE;
            else if (strcmp(word, "from") == 0) tokens[t].type = TOK_FROM;
            else if (strcmp(word, "as") == 0) tokens[t].type = TOK_AS;
            else if (strcmp(word, "export") == 0) tokens[t].type = TOK_EXPORT;
            else if (strcmp(word, "import") == 0) tokens[t].type = TOK_IMPORT;
            else if (strcmp(word, "static") == 0) tokens[t].type = TOK_STATIC;
            else if (strcmp(word, "extends") == 0) tokens[t].type = TOK_EXTENDS;
            else if (strcmp(word, "implements") == 0) tokens[t].type = TOK_IMPLEMENTS;
            else if (strcmp(word, "abstract") == 0) tokens[t].type = TOK_ABSTRACT;
            else if (strcmp(word, "property") == 0) tokens[t].type = TOK_PROPERTY;
            else if (strcmp(word, "true") == 0) tokens[t].type = TOK_TRUE;
            else if (strcmp(word, "false") == 0) tokens[t].type = TOK_FALSE;
            else if (strcmp(word, "void") == 0) tokens[t].type = TOK_VOID;
            else if (strcmp(word, "Integer") == 0) tokens[t].type = TOK_INTEGER;
            else if (strcmp(word, "Float") == 0) tokens[t].type = TOK_FLOAT;
            else if (strcmp(word, "String") == 0) tokens[t].type = TOK_STRING_TYPE;
            else if (strcmp(word, "Char") == 0) tokens[t].type = TOK_CHAR_TYPE;
            else if (strcmp(word, "Bool") == 0) tokens[t].type = TOK_BOOL_TYPE;
            else if (strcmp(word, "list") == 0) tokens[t].type = TOK_LIST;
            else if (strcmp(word, "dict") == 0) tokens[t].type = TOK_DICT;
            else if (strcmp(word, "set") == 0) tokens[t].type = TOK_SET;
            else if (strcmp(word, "tuple") == 0) tokens[t].type = TOK_TUPLE;
            else if (strcmp(word, "range") == 0) tokens[t].type = TOK_RANGE;
            else {
                tokens[t].type = TOK_IDENTIFIER;
                strcpy(tokens[t].text, word);
            }
            t++;
            continue;
        }

        // ---- ОПЕРАТОРЫ И СИМВОЛЫ ----
        switch (code[pos]) {
            case '=':
                if (code[pos+1] == '=') { tokens[t].type = TOK_EQ_EQ; pos += 2; }
                else { tokens[t].type = TOK_EQUALS; pos++; }
                t++;
                break;
            case '+':
                if (code[pos+1] == '+') { tokens[t].type = TOK_PLUS_PLUS; pos += 2; }
                else if (code[pos+1] == '=') { tokens[t].type = TOK_PLUS_EQ; pos += 2; }
                else { tokens[t].type = TOK_PLUS; pos++; }
                t++;
                break;
            case '-':
                if (code[pos+1] == '-') { tokens[t].type = TOK_MINUS_MINUS; pos += 2; }
                else if (code[pos+1] == '=') { tokens[t].type = TOK_MINUS_EQ; pos += 2; }
                else if (code[pos+1] == '>') { tokens[t].type = TOK_ARROW; pos += 2; }
                else { tokens[t].type = TOK_MINUS; pos++; }
                t++;
                break;
            case '*':
                if (code[pos+1] == '*') { 
                    if (code[pos+2] == '=') { tokens[t].type = TOK_DOUBLE_STAR_EQ; pos += 3; }
                    else { tokens[t].type = TOK_DOUBLE_STAR; pos += 2; }
                }
                else if (code[pos+1] == '=') { tokens[t].type = TOK_STAR_EQ; pos += 2; }
                else { tokens[t].type = TOK_STAR; pos++; }
                t++;
                break;
            case '/':
                if (code[pos+1] == '/') { 
                    if (code[pos+2] == '=') { tokens[t].type = TOK_DOUBLE_SLASH_EQ; pos += 3; }
                    else { tokens[t].type = TOK_DOUBLE_SLASH; pos += 2; }
                }
                else if (code[pos+1] == '=') { tokens[t].type = TOK_SLASH_EQ; pos += 2; }
                else { tokens[t].type = TOK_SLASH; pos++; }
                t++;
                break;
            case '%':
                if (code[pos+1] == '=') { tokens[t].type = TOK_PERCENT_EQ; pos += 2; }
                else { tokens[t].type = TOK_PERCENT; pos++; }
                t++;
                break;
            case '!':
                if (code[pos+1] == '=') { tokens[t].type = TOK_NOT_EQ; pos += 2; }
                else { tokens[t].type = TOK_NOT; pos++; }
                t++;
                break;
            case '>':
                if (code[pos+1] == '=') { tokens[t].type = TOK_GT_EQ; pos += 2; }
                else { tokens[t].type = TOK_GT; pos++; }
                t++;
                break;
            case '<':
                if (code[pos+1] == '=') { tokens[t].type = TOK_LT_EQ; pos += 2; }
                else { tokens[t].type = TOK_LT; pos++; }
                t++;
                break;
            case '&':
                if (code[pos+1] == '&') { tokens[t].type = TOK_AND_AND; pos += 2; }
                else { pos++; }
                t++;
                break;
            case '|':
                if (code[pos+1] == '|') { tokens[t].type = TOK_OR_OR; pos += 2; }
                else if (code[pos+1] == '>') { tokens[t].type = TOK_PIPE; pos += 2; }
                else { pos++; }
                t++;
                break;
            case '?':
                if (code[pos+1] == '?') {
                    if (code[pos+2] == '=') { tokens[t].type = TOK_ELVIS_EQ; pos += 3; }
                    else { tokens[t].type = TOK_ELVIS; pos += 2; }
                } else if (code[pos+1] == '.') { tokens[t].type = TOK_SAFE_DOT; pos += 2; }
                else { pos++; }
                t++;
                break;
            case '(': tokens[t].type = TOK_LPAREN; pos++; t++; break;
            case ')': tokens[t].type = TOK_RPAREN; pos++; t++; break;
            case '{': tokens[t].type = TOK_LBRACE; pos++; t++; break;
            case '}': tokens[t].type = TOK_RBRACE; pos++; t++; break;
            case '[': tokens[t].type = TOK_LBRACKET; pos++; t++; break;
            case ']': tokens[t].type = TOK_RBRACKET; pos++; t++; break;
            case ':': tokens[t].type = TOK_COLON; pos++; t++; break;
            case ';': tokens[t].type = TOK_SEMICOLON; pos++; t++; break;
            case ',': tokens[t].type = TOK_COMMA; pos++; t++; break;
            case '.': tokens[t].type = TOK_DOT; pos++; t++; break;
            default: pos++; break;
        }
    }
    tokens[t].type = TOK_EOF;
    return t;
}
