#include "sham.h"
#include <sys/time.h>
#include <time.h>

uint32_t generate_initial_seq() {
    return (uint32_t)time(NULL) % 10000 + 1;
}

void set_timeout(struct timeval* tv, int ms) {
    tv->tv_sec = ms / 1000;
    tv->tv_usec = (ms % 1000) * 1000;
}

int is_timeout(struct timeval* start, int ms) {
    struct timeval now, elapsed;
    gettimeofday(&now, NULL);
    
    elapsed.tv_sec = now.tv_sec - start->tv_sec;
    elapsed.tv_usec = now.tv_usec - start->tv_usec;
    
    if (elapsed.tv_usec < 0) {
        elapsed.tv_sec--;
        elapsed.tv_usec += 1000000;
    }
    
    return (elapsed.tv_sec * 1000 + elapsed.tv_usec / 1000) >= ms;
}