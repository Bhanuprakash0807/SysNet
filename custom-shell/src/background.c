#include "background.h"
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include<stdlib.h>

struct Job jobs[MAX_JOBS];
int job_cnt = 0;
int next_job_id = 1;



void add_background_job(int pid,const char* cmd){
if(job_cnt<MAX_JOBS){
    jobs[job_cnt].job_id=next_job_id;
    next_job_id++;
    jobs[job_cnt].pid=pid;
    strncpy(jobs[job_cnt].cmd,cmd,sizeof(jobs[job_cnt].cmd)-1);
    jobs[job_cnt].cmd[sizeof(jobs[job_cnt].cmd)-1]='\0';
     jobs[job_cnt].job_state = RUNNING;
    printf("[%d] %d\n",jobs[job_cnt].job_id,pid);
    fflush(stdout);
    job_cnt++;
}
}

void check_background_jobs() {
    int status;
    int pid;
    /////////////////////////
    for (int i = 0; i < job_cnt; i++) {
        pid = waitpid(jobs[i].pid, &status,WNOHANG | WUNTRACED);
        if (pid > 0) {
            if (WIFEXITED(status)) {
                printf("%s with pid %d exited normally\n",
                       jobs[i].cmd, jobs[i].pid);
                       fflush(stdout);
                       for (int j = i; j < job_cnt - 1; j++) {
                jobs[j] = jobs[j + 1];
            }
            job_cnt--;
            i--;
            } else if (WIFSIGNALED(status)) {
                printf("%s with pid %d exited abnormally\n",
                       jobs[i].cmd, jobs[i].pid);
                          fflush(stdout);
                       for (int j = i; j < job_cnt - 1; j++) {
                jobs[j] = jobs[j + 1];
            }
            job_cnt--;
            i--;
            }else if (WIFSTOPPED(status)) {
                jobs[i].job_state = STOPPED;
                printf("[%d] Stopped %s\n", jobs[i].job_id, jobs[i].cmd);
                fflush(stdout);
            }
        }
    }
    /////////////////////////
}

int cmp_job(const void* a, const void* b) {
    struct Job* jobA = *(struct Job**)a;
    struct Job* jobB = *(struct Job**)b;
    return strcmp(jobA->cmd, jobB->cmd);
}

void activities(){
    check_background_jobs();
    if(job_cnt==0){
        return;
    }
    struct Job* temp[MAX_JOBS];
    for (int i = 0; i < job_cnt; i++) {
        temp[i] = &jobs[i];
    }

    qsort(temp, job_cnt, sizeof(struct Job*), cmp_job);

    for (int i = 0; i < job_cnt; i++) {
        printf("[%d] : %s - %s\n",
               temp[i]->pid,
               temp[i]->cmd,
               (temp[i]->job_state == RUNNING ? "Running" : "Stopped"));
    }
}

void add_background_stopped(int pid, const char* cmd) {
    if (job_cnt < MAX_JOBS) {
        jobs[job_cnt].job_id = next_job_id++;
        jobs[job_cnt].pid = pid;
        strncpy(jobs[job_cnt].cmd, cmd, sizeof(jobs[job_cnt].cmd) - 1);
        jobs[job_cnt].cmd[sizeof(jobs[job_cnt].cmd) - 1] = '\0';
        jobs[job_cnt].job_state = STOPPED;
        int printed_id = jobs[job_cnt].job_id;
        job_cnt++;
        printf("[%d] Stopped %s\n", printed_id, cmd);
        fflush(stdout);
    }
}