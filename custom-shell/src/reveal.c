#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include "reveal.h"


static int cmp(const void *a, const void *b) {
    const char *sa = *(const char **)a;
    const char *sb = *(const char **)b;
    return strcmp(sa, sb);  
}

void reveal(int argc,char* argv[],char* shell_home,char** prev_dir){
char cwd[1025];
getcwd(cwd, sizeof(cwd));
char dir[1025];
int dir_arg=-1;
int line_format=0;
int show_hidden=0;
for(int i=1;i<argc;i++){
if(argv[i][0]=='-'){
        for(int j=1;argv[i][j];j++){
            if(argv[i][j]=='a'){
                show_hidden=1;
            }
            else if(argv[i][j]=='l'){
                line_format=1;
            }
            else{
                continue;
            }
        }
}
else{
    if(dir_arg==-1){
        dir_arg=i;
    }
    else{
        printf("reveal: Invalid Syntax!\n");
        return;
    }
}
}

if(dir_arg==-1){
    strcpy(dir,cwd);
}
else if(strcmp(argv[dir_arg],"~")==0){
strcpy(dir,shell_home);
}
else if(strcmp(argv[dir_arg],".")==0){
strcpy(dir,cwd);
}
else if(strcmp(argv[dir_arg],"..")==0){
    //////////////////////////////
if(realpath("..", dir) == NULL){
        printf("No such directory!\n");
        return;
    }
//////////////////////////
}
else if (strcmp(argv[dir_arg], "-") == 0) {
    if (*prev_dir && (*prev_dir)[0] != '\0') {
        strcpy(dir, *prev_dir);
    } else {
        printf("No such directory!\n");
        return;
    }
}


else if(dir_arg >= 0){
     getcwd(cwd, sizeof(cwd));
    strcpy(*prev_dir, cwd);  
    strcpy(dir, argv[dir_arg]);
} else {
    printf("reveal: Invalid Syntax!\n");
    return;
}


DIR* d=opendir(dir);
if(d==NULL){
    printf("No such directory!\n");
    return;
}
struct dirent* entry;
char **names=NULL;
int cnt=0;
int capacity=16;
names=malloc(capacity*sizeof(char*));

while((entry=readdir(d))!=NULL){
    if(!show_hidden && entry->d_name[0]=='.'){
        continue;
    }
    if(cnt==capacity){
        capacity*=2;
        names=realloc(names,capacity*sizeof(char*));
    }
    names[cnt]=malloc((strlen(entry->d_name)+1)*sizeof(char));
    strcpy(names[cnt],entry->d_name);
    cnt++;
}
closedir(d);

qsort(names, cnt, sizeof(char *), cmp);

for(int i=0;i<cnt;i++){
    if(line_format){
        printf("%s\n",names[i]);
    }
    else{
        printf("%s",names[i]);
        if(i<cnt-1){
            printf(" ");
        }
        else{
            printf("\n");
        }
    }
    free(names[i]);
}
free(names);

}