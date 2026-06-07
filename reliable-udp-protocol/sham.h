#ifndef SHAM_H
#define SHAM_H

#include <stdint.h>
#include <sys/time.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// S.H.A.M. Header Structure
struct sham_header {
    uint32_t seq_num;      // Sequence Number
    uint32_t ack_num;      // Acknowledgment Number
    uint16_t flags;        // Control flags (SYN, ACK, FIN)
    uint16_t window_size;  // Flow control window size
};

// Flag definitions
#define SYN_FLAG 0x1
#define ACK_FLAG 0x2
#define FIN_FLAG 0x4

// Protocol constants
#define MAX_PAYLOAD_SIZE 1024
#define WINDOW_SIZE 10
#define RTO_MS 500
#define MAX_RETRIES 5

// Utility functions
void log_event(const char* format, ...);
uint32_t generate_initial_seq();
void set_timeout(struct timeval* tv, int ms);
int is_timeout(struct timeval* start, int ms);

#endif