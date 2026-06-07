#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "prompt.h"
#include "input.h"
#include "parser.h"
#include "hop.h"
#include "reveal.h"
#include "log.h"
#include "execute.h"
#include "background.h"
#include "ping.h"
#include <signal.h>

int main() {

    log_init();
    char shell_home[1025];  
    getcwd(shell_home, sizeof(shell_home));
    char* input=NULL;
    // char prev_dir[1025] = ""; 
    char* prev_dir = malloc(1025);
    prev_dir[0] = '\0'; 
    setup_signals();
    while(1){
        check_background_jobs();
        fflush(stdout);
        print_prompt(shell_home);
        input=read_input();
        if (!input) { 
            printf("\nlogout\n");
            kill(0, SIGKILL); 
            exit(0);
        }

        int token_count = 0;
        Token* tokens = tokenize(input, &token_count);
        int index = 0;
        if(!parse_shell_cmd(tokens,&index,token_count)){
            printf("Invalid Syntax!\n");
            free_tokens(tokens, token_count);
            free(input);
            continue;
        }
        execute_shell_cmd(tokens, token_count, &prev_dir, shell_home,input);
        free_tokens(tokens, token_count);
        free(input);
    }

    return 0;
}
