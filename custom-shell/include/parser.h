#ifndef PARSER_H
#define PARSER_H

typedef enum {
    TOKEN_AMPERSAND,
    TOKEN_SEMICOLON,
    TOKEN_PIPE,
    TOKEN_INPUT,
    TOKEN_OUTPUT_TRUNC,
    TOKEN_OUTPUT_APPEND,
    TOKEN_NAME,
    TOKEN_END
} TokenType;

typedef struct {
    TokenType type;
    char* value;
} Token;

Token* tokenize(const char* input, int* token_count);
void free_tokens(Token* tokens, int token_count);
int parse_shell_cmd(Token* tokens, int* index,int token_count);
int parse_cmd_group(Token* tokens, int* index,int token_count);
int parse_atomic(Token* tokens, int* index,int token_count);

#endif