#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

void ping_cmd(int argc, char** argv) {
    if (argc != 3) {
        printf("Invalid syntax!\n");
        return;
    }
/////////////////////////////
    char *endptr;
    long pid = strtol(argv[1], &endptr, 10);
    if (*endptr != '\0' || pid <= 0) {
        printf("Invalid syntax!\n");
        return;
    }

    long sig = strtol(argv[2], &endptr, 10);
    if (*endptr != '\0' || sig < 0) {
        printf("Invalid syntax!\n");
        return;
    }
///////////////////////////////
    int actual_signal = sig % 32;

    if (kill(pid, actual_signal) == -1) {
        if (errno == ESRCH) {
            printf("No such process found\n");
        } 
        else if(errno==EINVAL){
            printf("Invalid syntax!\n");
        }
        else {           
            perror("Error sending signal");
        }
        return;
    }
    else{
        printf("Sent signal %ld to process with pid %ld\n", sig, pid);
    }

}
