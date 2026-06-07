#include "log.h"
#include "hop.h"
#include "reveal.h"
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include "execute.h"


static char* logs[15];
static int log_cnt=0;


void log_init(){
    FILE* f =fopen(".shell_log","r");
if (!f) return; 

      char line[1024];
    while (fgets(line, sizeof(line), f)) {
        line[strcspn(line, "\n")] = 0;
        logs[log_cnt] = strdup(line);
        log_cnt++;
        if (log_cnt>=15){
            break;
        }
    }
    fclose(f);
}

static void persist_log() {
    FILE *f = fopen(".shell_log", "w");
    if (!f) return;
    for (int i = 0; i < log_cnt; i++) {
        fprintf(f, "%s\n", logs[i]);
    }
    fclose(f);
}

void log_show(){
    for(int i=0;i<log_cnt;i++){
        printf("%s\n",logs[i]);
    }
}

void log_purge(){
    for(int i=0;i<log_cnt;i++){
        free(logs[i]);
    }
    log_cnt=0;
    persist_log();
}

void log_save(char* input){
    if(strncmp(input,"log",3)==0 && (input[3]==' ' || input[3]=='\0')){
        return;
    }
    if(log_cnt>0 && strcmp(logs[log_cnt-1],input)==0){
        return;
    }
    if(log_cnt<15){
        logs[log_cnt]=strdup(input);
        log_cnt++;
    }
    else{
        free(logs[0]);
        for(int i=1;i<15;i++){
            logs[i-1]=logs[i];
        }
        logs[14]=strdup(input);
    }
    persist_log();
}

void log_execute(int index,char** prev_dir,char* shell_home){
    if(index<=0 || index>log_cnt){
        printf("log execute : Invalid Index!\n");
        return;
    }
    char* cmd=logs[log_cnt-index];
    int token_cnt=0;
    Token* tokens=tokenize(cmd,&token_cnt);
    int idx=0;
    if (!parse_shell_cmd(tokens, &idx, token_cnt)) {
        printf("Invalid Syntax\n");
        free_tokens(tokens, token_cnt);
        return;
    }
    execute_tokens(tokens,token_cnt,prev_dir,shell_home,cmd);
    free_tokens(tokens, token_cnt);
}
