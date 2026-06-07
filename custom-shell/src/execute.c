#include "execute.h"
#include "hop.h"
#include "reveal.h"
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include "log.h"
#include "background.h" 
#include "ping.h"
#include <signal.h>

pid_t fg_pid = -1;
char fg_command[256] = "";
/////////////////////////////////////////////////////////////
// Ctrl-C
void sigint_handler(int signo) {
    (void)signo;
    if (fg_pid > 0) {
        kill(-fg_pid, SIGINT);
    }
}

// Ctrl-Z
void sigtstp_handler(int signo) {
    (void)signo;
    if (fg_pid > 0) {
        kill(-fg_pid, SIGTSTP);
        add_background_stopped(fg_pid, fg_command);
        fg_pid = -1;
        fg_command[0] = '\0';
    }
}

void setup_signals() {
    signal(SIGINT, sigint_handler);
    signal(SIGTSTP, sigtstp_handler);
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
}

////////////////////////////
void execute_tokens(Token* tokens,int token_count,char** prev_dir,char* shell_home,char* input){

    if(token_count==0){
        return;
    }
    Token* cmds[100];
    int cmd_cnt=0;
    int cmd_lens[100];
    int sta=0;
    for(int i=0;i<token_count;i++){
        if(tokens[i].type==TOKEN_PIPE || i==token_count-1){
          int end;
          if(tokens[i].type==TOKEN_PIPE){
            end=i;
          }
          else{
            end=i+1;
          }
          cmds[cmd_cnt]=&tokens[sta];
          cmd_lens[cmd_cnt]=end-sta;
          cmd_cnt++;
          sta=i+1;
        }
    }



    if (cmd_cnt == 1) {
        Token* cmd = cmds[0];
        int cmd_len = cmd_lens[0];
        char* argv[100];
        int argc = 0;
        int append = 0;
        char* input_file = NULL;
        char* output_file = NULL;

        for (int j = 0; j < cmd_len; j++) {
            if (cmd[j].type == TOKEN_INPUT) {
                input_file = cmd[++j].value;
            } else if (cmd[j].type == TOKEN_OUTPUT_TRUNC) {
                output_file = cmd[++j].value;
                append = 0;
            } else if (cmd[j].type == TOKEN_OUTPUT_APPEND) {
                output_file = cmd[++j].value;
                append = 1;
            } else if (cmd[j].type == TOKEN_NAME) {
                argv[argc++] = cmd[j].value;
            }
        }
        argv[argc] = NULL;
        if (argc == 0) return;

       if (strcmp(argv[0], "activities") == 0) {
            activities();
            log_save(input);
            return;
        }

        if (strcmp(argv[0], "ping") == 0) {
            ping_cmd(argc, argv);
            log_save(input);
            return;
        }

        if (strcmp(argv[0], "hop") == 0 ||
            strcmp(argv[0], "reveal") == 0 ||
            strcmp(argv[0], "log") == 0) {
            if (strcmp(argv[0], "hop") == 0) {
                hop(argv, argc, prev_dir, shell_home);
                log_save(input);
                return;
            }

            if (strcmp(argv[0], "reveal") == 0) {
                reveal(argc, argv, shell_home, prev_dir);
                log_save(input);
                return;
            }

            if (strcmp(argv[0], "log") == 0) {
                if (argc == 1) {
                    log_show();
                } else if (argc == 2 && strcmp(argv[1], "purge") == 0) {
                    log_purge();
                } else if (argc == 3 && strcmp(argv[1], "execute") == 0) {
                    int ind = atoi(argv[2]);
                    log_execute(ind, prev_dir, shell_home);
                } else {
                    printf("log : Invalid Arguments\n");
                }
                return;
            }

            
        }
         int pid = fork();
        if (pid < 0) {
            perror("fork failed");
            return;
        }
        if (pid == 0) {
            setpgid(0, 0);

            if (input_file) {
                int fd = open(input_file, O_RDONLY);
                if (fd < 0) { fprintf(stderr, "No such file or directory\n"); exit(1); }
                dup2(fd, STDIN_FILENO);
                close(fd);
            }
            if (output_file) {
                int fd;
                if (append)
                    fd = open(output_file, O_WRONLY | O_CREAT | O_APPEND, 0644);
                else
                    fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd < 0) { fprintf(stderr, "Unable to create file for writing\n"); exit(1); }
                dup2(fd, STDOUT_FILENO);
                close(fd);
            }
            execvp(argv[0], argv);
            fprintf(stderr, "Command not found!\n");
            exit(1);
        } else {
              setpgid(pid, pid);
            fg_pid = pid;
            strncpy(fg_command,argv[0], sizeof(fg_command) - 1);
            fg_command[sizeof(fg_command) - 1] = '\0';
            int status;
            waitpid(pid, &status, WUNTRACED);
            if (WIFSTOPPED(status)) {
                add_background_stopped(pid, fg_command);
            }   
            fg_pid = -1;
            fg_command[0] = '\0';


            log_save(input);
            return;
        }
        

       
    }





///////////////////////////////////
    int pipe_fds[cmd_cnt-1][2];
     pid_t child_pids[100];
    pid_t pgid = 0;
    if(cmd_cnt>1){
        for(int i=0;i<cmd_cnt-1;i++){
            if(pipe(pipe_fds[i])==-1){
                perror("Pipe failed");
                return;
            }
        }
    }
/////////////////////////////////


    for(int i=0;i<cmd_cnt;i++){
        Token* cmd=cmds[i];
        int cmd_len=cmd_lens[i];

        char* argv[100];
    int argc=0;

    int append=0;
    char* input_file=NULL;
    char* output_file=NULL;

    for(int j=0;j<cmd_len;j++){
        if(cmd[j].type==TOKEN_INPUT) {
            input_file=cmd[++j].value;
        }
        else if(cmd[j].type== TOKEN_OUTPUT_TRUNC){
            output_file=cmd[++j].value;
            append=0;
        }
        else if(cmd[j].type== TOKEN_OUTPUT_APPEND){
            output_file=cmd[++j].value;
            append=1;
        }
        else if(cmd[j].type==TOKEN_NAME){
            argv[argc++]=cmd[j].value;
        }
    }
    argv[argc]=NULL;
    if(argc==0){
        return;
    }


    int pid=fork();
    if(pid==0){
         if (pgid == 0) {
                setpgid(0, 0); 
            } else {
                setpgid(0, pgid); 
            }
        if(i==0 && input_file){
            int fd=open(input_file,O_RDONLY);
               if(fd<0){
                fprintf(stderr, "No such file or directory\n");

                exit(1);
            }
            dup2(fd,STDIN_FILENO);
            close(fd);
        }
        if(i>0){
            dup2(pipe_fds[i-1][0],STDIN_FILENO);
        }
        if(i==cmd_cnt-1 && output_file){
            int fd;
            if(append){
                fd=open(output_file,O_WRONLY | O_CREAT | O_APPEND, 0644);
            }
            else{
                fd=open(output_file,O_WRONLY | O_CREAT | O_TRUNC, 0644);
            }
            if(fd<0){
                perror("Unable to create file for writing");
                exit(1);
            }
            dup2(fd,STDOUT_FILENO);
            close(fd);
        }
        if(i<cmd_cnt-1){
            dup2(pipe_fds[i][1],STDOUT_FILENO);
        }
         for(int j = 0; j < cmd_cnt-1; j++) {
                close(pipe_fds[j][0]);
                close(pipe_fds[j][1]);
            }

             if(strcmp(argv[0],"hop")==0){
            hop(argv,argc,prev_dir,shell_home);
            log_save(input);
            exit(0);
        }
        if(strcmp(argv[0],"reveal")==0){
            reveal(argc,argv,shell_home,prev_dir);
            log_save(input);
            exit(0);
        }
        if(strcmp(argv[0],"log")==0){
            if(argc==1){
                log_show();
            }
            else if(argc==2 && strcmp(argv[1],"purge")==0){
                log_purge();
            }
            else if(argc==3 && strcmp(argv[1],"execute")==0){
                int ind=atoi(argv[2]);
                // log_execute(ind,prev_dir,shell_home);
                 pid_t sub = fork();
                if(sub == 0){
            // grandchild: safely run the replayed command
            log_execute(ind, prev_dir, shell_home);
            exit(0);
        } else {
            int st;
            waitpid(sub, &st, 0);
            exit(WEXITSTATUS(st));
        }
            }
            else{
                printf("log : Invalid Arguments\n");
            }
            exit(0);
        }
        if (strcmp(argv[0], "activities") == 0) {
                activities();
                log_save(input);
                exit(0);
            }
        if (strcmp(argv[0], "ping") == 0) {
            ping_cmd(argc, argv);
            log_save(input);
            exit(0);
        }


        execvp(argv[0], argv);
        fprintf(stderr, "Command not found!\n");

        exit(1);
    }
    else{
        if (pgid == 0) {
                pgid = pid;
            }
            setpgid(pid, pgid);
            child_pids[i] = pid;
    }
}
/////////////////
for(int j=0;j<cmd_cnt-1;j++){
    close(pipe_fds[j][0]);
    close(pipe_fds[j][1]);
}
    ///////////////////
      fg_pid = pgid;
    {
        Token* first_cmd = cmds[0];
        int first_len = cmd_lens[0];
        for (int t = 0; t < first_len; ++t) {
            if (first_cmd[t].type == TOKEN_NAME && first_cmd[t].value) {
                strncpy(fg_command, first_cmd[t].value, sizeof(fg_command)-1);
                fg_command[sizeof(fg_command)-1] = '\0';
                break;
            }
        }
    }
////////////////////////////////
    int any_stopped = 0;
    for (int i = 0; i < cmd_cnt; i++) {
        int status;
        waitpid(child_pids[i], &status, WUNTRACED);
        if (WIFSTOPPED(status)) {
            any_stopped = 1;
        }
    }

    if (any_stopped) {
        add_background_stopped(pgid, fg_command);
    }

    fg_pid = -1;
    fg_command[0] = '\0';

    log_save(input);

}

void execute_shell_cmd(Token* tokens, int token_count, char** prev_dir, char* shell_home, char* input) {
    check_background_jobs(); 
    int start=0;
    for(int i=0;i<token_count;i++){
        if(tokens[i].type==TOKEN_SEMICOLON || tokens[i].type==TOKEN_AMPERSAND || i==token_count-1 ){
            int end;
            if(tokens[i].type==TOKEN_AMPERSAND || tokens[i].type==TOKEN_SEMICOLON){
                end=i;
            }
            else{
                end=i+1;
            }
            int cnt=end-start;
            if(cnt<=0){
                start=i+1;
                continue;
            }

            if(tokens[i].type==TOKEN_SEMICOLON || i==token_count-1){
                    execute_tokens(&tokens[start],cnt,prev_dir,shell_home,input);
            }
            else if(tokens[i].type==TOKEN_AMPERSAND){
                int pid=fork();
                if(pid==0){
                    //////////////////////////////////////////
                     // prevent background job from using stdin
                    int devnull = open("/dev/null", O_RDONLY);
                    if (devnull >= 0) {
                        dup2(devnull, STDIN_FILENO);
                        close(devnull);
                    }
                    //////////////////////////////////////////
                    execute_tokens(&tokens[start],cnt,prev_dir,shell_home,input);
                    exit(0);
                }
                else{
                    char cmd_str[256] = "";
                    ///////////////////////////////////
                    for (int k = start; k < end; k++) {
                        strncat(cmd_str, tokens[k].value, sizeof(cmd_str) - strlen(cmd_str) - 1);
                        strncat(cmd_str, " ", sizeof(cmd_str) - strlen(cmd_str) - 1);
                    }
                    /////////////////
                    add_background_job(pid, cmd_str);
                    log_save(input);
                  }
        }
            start=i+1;
        }

    }


}