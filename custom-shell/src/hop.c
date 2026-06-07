#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include "hop.h"    

void hop(char* argv[],int argc,char** prev_dir,char* shell_home){
char cwd[1025];
if(argc==1){
    //no arguments
    // getcwd(cwd, sizeof(cwd));
    // strcpy(prev_dir, cwd);
    // chdir(shell_home);
     getcwd(cwd, sizeof(cwd));
        if(chdir(shell_home) == 0) {
            strcpy(*prev_dir, cwd);   // update only if success
        }
        return;
}
for(int i=1;i<argc;i++){
    if(strcmp(argv[i],"~")==0){
        // hop ~
        getcwd(cwd, sizeof(cwd));
        strcpy(*prev_dir,cwd);
        chdir(shell_home);
    }
    else if(strcmp(argv[i],".")==0){
        //hop .
        continue;
    }
    else if(strcmp(argv[i],"..")==0){
        //hop..
        getcwd(cwd, sizeof(cwd));
        strcpy(*prev_dir,cwd);
        chdir("..");
    }
else if (strcmp(argv[i], "-") == 0) {
    if (*prev_dir && (*prev_dir)[0] != '\0') {
        getcwd(cwd, sizeof(cwd));
        char temp[1025];
        strcpy(temp, cwd);

        if (chdir(*prev_dir) == 0) {
            strcpy(*prev_dir, temp);
        } else {
            printf("No such directory!\n");
        }
    } else {
        printf("No such directory!\n");
    }
}

    else{
        //hop <dir>
        getcwd(cwd,sizeof(cwd));
        strcpy(*prev_dir,cwd);
        if(chdir(argv[i])!=0){
            printf("No such directory!\n");
        }
    }

}
}