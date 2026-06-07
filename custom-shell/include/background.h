#ifndef BACKGROUND_H
#define BACKGROUND_H

#include <sys/types.h>

typedef enum {
    RUNNING,
    STOPPED
}state;

typedef struct Job {
    int job_id;
    int pid;
    char cmd[256];
    state job_state;
}Job;

#define MAX_JOBS 100

void add_background_job(int pid, const char* cmd);
void check_background_jobs();
void activities();
void add_background_stopped(int pid, const char* cmd);


#endif
