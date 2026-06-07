#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fcntl.h"

int main(){
    uint64 ini_cnt= getreadcount();
    char buf[101];
    printf("Initial read count: %lu\n",ini_cnt);
    int fd = open("README",O_RDONLY);
    if(fd<0){
      printf("Error: Unable to open file\n");
      exit(1);
    }
    int n = read(fd,buf,100);
    buf[n] = '\0';
    uint64 final_cnt = getreadcount();
    printf("Final read count: %lu\n",final_cnt); 
    uint64 diff= final_cnt - ini_cnt;
    if(diff<=0){
      printf("Error: Read count did not increase after read operation\n");
      exit(1);
    }
    else{
        printf("Read count increased by %lu after read operation\n",diff);
    }
    exit(0);
    
    
}