#include<stdlib.h>
#include<stdio.h>
#include<string.h> 
#include "input.h"
#include<stdlib.h>
#include<stdio.h>
#include<string.h> 
#include "input.h"
#include<unistd.h>
#include <sys/types.h> 



char* read_input() {
    char *input = NULL;
    size_t len = 0;
    ssize_t nread = getline(&input, &len, stdin);  

    if (nread == -1) {    
        free(input);
        return NULL;      
    }

    if (nread > 0 && input[nread - 1] == '\n') {
        input[nread - 1] = '\0';
    }

    return input;
}
