#ifndef EXECUTE_H
#define EXECUTE_H

#include "parser.h"

void sigtstp_handler(int signo);
void sigint_handler(int signo);
void setup_signals();
void execute_tokens(Token* tokens, int token_count, char** prev_dir, char* shell_home,char* input);
void execute_shell_cmd(Token* tokens, int token_count, char** prev_dir, char* shell_home, char* input);

#endif
