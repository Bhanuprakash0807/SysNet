#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include "prompt.h"
#include <unistd.h>

void print_prompt(const char *shell_home) {
    char cwd[1025];
    char hostname[256];
    char *username = getlogin();

    getcwd(cwd, sizeof(cwd));
    gethostname(hostname, sizeof(hostname));

    if (strncmp(cwd, shell_home, strlen(shell_home)) == 0) {
        printf("<%s@%s:~%s> ", username, hostname, cwd + strlen(shell_home));
    } 
    else {
        printf("<%s@%s:%s> ", username, hostname, cwd);
    }
    fflush(stdout);
}
