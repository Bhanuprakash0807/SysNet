#include "parser.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h> 
Token* tokenize(const char* input, int* token_count) {
    if (input == NULL) {
        *token_count = 0;
        return NULL;
    }
    
    int capacity = 10;
    Token* tokens = malloc(capacity * sizeof(Token));
    int count = 0;
    int i = 0;
    int len = strlen(input); 
    
    while (i < len) { 
        if (isspace(input[i])) {
            i++;
            continue;
        }
        if (count >= capacity) {
            capacity *= 2;
            tokens = realloc(tokens, capacity * sizeof(Token));
        }
        if (input[i] == '>') {
            if (i + 1 < len && input[i+1] == '>') { 
                tokens[count].type = TOKEN_OUTPUT_APPEND;
                tokens[count].value = NULL;
                count++;
                i += 2;
            } else {
                tokens[count].type = TOKEN_OUTPUT_TRUNC;
                tokens[count].value = NULL;
                count++;
                i += 1;
            }
        }
    else if (input[i] == '<') {
            tokens[count].type = TOKEN_INPUT;
            tokens[count].value = NULL;
            count++;
            i += 1;
        } else if (input[i] == '|') {
            tokens[count].type = TOKEN_PIPE;
            tokens[count].value = NULL;
            count++;
            i += 1;
        } else if (input[i] == '&') {
            tokens[count].type = TOKEN_AMPERSAND;
            tokens[count].value = NULL;
            count++;
            i += 1;
        } else if (input[i] == ';') {
            tokens[count].type = TOKEN_SEMICOLON;
            tokens[count].value = NULL;
            count++;
            i += 1;
        } else {
            int start = i;
            while (i<len && !isspace(input[i]) && strchr("|&><;", input[i]) == NULL) {
                i++;
            }
            int name_len = i - start;
            char* name = malloc(name_len + 1);
            strncpy(name, input + start, name_len);
            name[name_len] = '\0';
            tokens[count].type = TOKEN_NAME;
            tokens[count].value = name;
            count++;
        }
    }
    if (count >= capacity) {
        capacity += 1;
        tokens = realloc(tokens, capacity * sizeof(Token));
    }
    tokens[count].type = TOKEN_END;
    tokens[count].value = NULL;
    count++;
    *token_count = count;
    return tokens;
}

void free_tokens(Token* tokens, int token_count) {
    if (tokens == NULL) return;
    
    for (int i = 0; i < token_count; i++) {
        if (tokens[i].type == TOKEN_NAME && tokens[i].value != NULL) {
            free(tokens[i].value);
        }
    }
    free(tokens);
}


int parse_atomic(Token* tokens, int* index, int token_count) {
    if (*index >= token_count || tokens[*index].type != TOKEN_NAME) {
        return 0;
    }
    (*index)++;
    
    while (*index < token_count) {
        TokenType current_type = tokens[*index].type;
        
        if (current_type == TOKEN_NAME) {
            (*index)++;
        } 
        else if (current_type == TOKEN_INPUT || 
                   current_type == TOKEN_OUTPUT_TRUNC || 
                   current_type == TOKEN_OUTPUT_APPEND) {
            (*index)++;
            if (*index >= token_count || tokens[*index].type != TOKEN_NAME) {
                return 0;
            }
            (*index)++;
        } 
        else {
            break;
        }
    }
    
    return 1;
}


int parse_cmd_group(Token* tokens, int* index,int token_count) {
    if (!parse_atomic(tokens, index,token_count)) {
        return 0;
    }
    while (*index<token_count && tokens[*index].type == TOKEN_PIPE) {
        (*index)++;
        if (!parse_atomic(tokens, index,token_count)) {
            return 0;
        }
    }
    return 1;
}





int parse_shell_cmd(Token* tokens, int* index, int token_count) {
    if (!parse_cmd_group(tokens, index, token_count)) {
        return 0;
    }
    
    // Parse zero or more (& | ;) cmd_group sequences
    while (*index < token_count) {
        TokenType current_type = tokens[*index].type;
        
        if (current_type == TOKEN_AMPERSAND || current_type == TOKEN_SEMICOLON) {
            // Check if this is followed by another command group (not the end)
            if (*index + 1 < token_count && tokens[*index + 1].type != TOKEN_END) {
                // It's a separator between command groups
                (*index)++;
                if (!parse_cmd_group(tokens, index, token_count)) {
                    return 0;
                }
            } else {
                if (current_type == TOKEN_AMPERSAND) {
                    (*index)++;
                    return (*index >= token_count || tokens[*index].type == TOKEN_END);
                } else {
                    return 0;
                }
            }
        } else {
            break;
        }
    }
    
    return (*index >= token_count || tokens[*index].type == TOKEN_END);
}