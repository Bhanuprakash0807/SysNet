#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/select.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <openssl/md5.h>
#include "sham.h"
#include <stdarg.h>

#define BUFFER_SIZE 1024
#define MAX_CLIENTS 10

int server_sockfd;
struct sockaddr_in server_addr, client_addr;
socklen_t client_len;
FILE* log_file = NULL;


uint32_t expected_seq_num = 0;
uint16_t window_size = 65535;
float loss_rate = 0.0;

void init_log() {
    if (getenv("RUDP_LOG") != NULL && atoi(getenv("RUDP_LOG")) == 1) {
        log_file = fopen("server_log.txt", "w");
        if (log_file == NULL) {
            perror("Failed to open log file");
        }
    }
}

void log_event(const char* format, ...) {
    if (log_file == NULL) return;
    
    va_list args;
    va_start(args, format);
    
    char time_buffer[30];
    struct timeval tv;
    time_t curtime;
    
    gettimeofday(&tv, NULL);
    curtime = tv.tv_sec;
    
    strftime(time_buffer, 30, "%Y-%m-%d %H:%M:%S", localtime(&curtime));
    fprintf(log_file, "[%s.%06ld] [LOG] ", time_buffer, tv.tv_usec);
    
    vfprintf(log_file, format, args);
    fprintf(log_file, "\n");
    va_end(args);
}

int should_drop_packet() {
    if (loss_rate <= 0.0) return 0;
    return ((float)rand() / RAND_MAX) < loss_rate;
}

void perform_three_way_handshake(int sockfd, struct sockaddr_in* client_addr, socklen_t client_len) {
    struct sham_header syn_packet, syn_ack_packet, ack_packet;
    fd_set read_fds;
    struct timeval timeout;
    

    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(sockfd, &read_fds);
        
        timeout.tv_sec = 0;
        timeout.tv_usec = 100000;
        
        int activity = select(sockfd + 1, &read_fds, NULL, NULL, &timeout);
        
        if (activity > 0 && FD_ISSET(sockfd, &read_fds)) {
            recvfrom(sockfd, &syn_packet, sizeof(syn_packet), 0,(struct sockaddr*)client_addr, &client_len);
            
            if (ntohs(syn_packet.flags) & SYN_FLAG) {
                log_event("RCV SYN SEQ=%u", ntohl(syn_packet.seq_num));
                expected_seq_num = ntohl(syn_packet.seq_num) + 1;
                break;
            }
        }
    }
    

    syn_ack_packet.seq_num = htonl(generate_initial_seq());
    syn_ack_packet.ack_num = htonl(expected_seq_num);
    syn_ack_packet.flags = htons(SYN_FLAG | ACK_FLAG);
    syn_ack_packet.window_size = htons(window_size);
    
    sendto(sockfd, &syn_ack_packet, sizeof(syn_ack_packet), 0,(struct sockaddr*)client_addr, client_len);
    
    log_event("SND SYN-ACK SEQ=%u ACK=%u", ntohl(syn_ack_packet.seq_num), ntohl(syn_ack_packet.ack_num));
    

    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(sockfd, &read_fds);
        ///////////////////
        timeout.tv_sec = 5;
        timeout.tv_usec = 0;
        //////////////////
        int activity = select(sockfd + 1, &read_fds, NULL, NULL, &timeout);
        
        if (activity > 0 && FD_ISSET(sockfd, &read_fds)) {
            recvfrom(sockfd, &ack_packet, sizeof(ack_packet), 0, (struct sockaddr*)client_addr, &client_len);
            
            if ((ntohs(ack_packet.flags) & ACK_FLAG) &&  ntohl(ack_packet.ack_num) == ntohl(syn_ack_packet.seq_num) + 1) {
                log_event("RCV ACK FOR SYN");
                break;
            }
        } 
        else {
            sendto(sockfd, &syn_ack_packet, sizeof(syn_ack_packet), 0,(struct sockaddr*)client_addr, client_len);
            log_event("RESEND SYN-ACK SEQ=%u ACK=%u", ntohl(syn_ack_packet.seq_num), ntohl(syn_ack_packet.ack_num));
        }
    }
}

void perform_four_way_handshake(int sockfd, struct sockaddr_in* client_addr, socklen_t client_len) {
    struct sham_header fin_packet, ack_packet, fin_response;
    fd_set read_fds;
    struct timeval timeout;
    
   
    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(sockfd, &read_fds);
        //////////////////////////
        timeout.tv_sec = 5;
        timeout.tv_usec = 0;
        /////////////////////////
        int activity = select(sockfd + 1, &read_fds, NULL, NULL, &timeout);
        
        if (activity > 0 && FD_ISSET(sockfd, &read_fds)) {
            recvfrom(sockfd, &fin_packet, sizeof(fin_packet), 0, (struct sockaddr*)client_addr, &client_len);
            
            if (ntohs(fin_packet.flags) & FIN_FLAG) {
                log_event("RCV FIN SEQ=%u", ntohl(fin_packet.seq_num));
                break;
            }
        }
    }
    
    
    ack_packet.ack_num = htonl(ntohl(fin_packet.seq_num) + 1);
    ack_packet.flags = htons(ACK_FLAG);
    ack_packet.window_size = htons(window_size);
    
    sendto(sockfd, &ack_packet, sizeof(ack_packet), 0,
          (struct sockaddr*)client_addr, client_len);
    
    log_event("SND ACK FOR FIN ACK=%u", ntohl(ack_packet.ack_num));
    
    fin_response.seq_num = htonl(generate_initial_seq());
    fin_response.flags = htons(FIN_FLAG);
    fin_response.window_size = htons(window_size);
    
    sendto(sockfd, &fin_response, sizeof(fin_response), 0,(struct sockaddr*)client_addr, client_len);
    
    log_event("SND FIN SEQ=%u", ntohl(fin_response.seq_num));
    
    
    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(sockfd, &read_fds);
        
        timeout.tv_sec = 5;
        timeout.tv_usec = 0;
        
        int activity = select(sockfd + 1, &read_fds, NULL, NULL, &timeout);
        
        if (activity > 0 && FD_ISSET(sockfd, &read_fds)) {
            recvfrom(sockfd, &ack_packet, sizeof(ack_packet), 0, (struct sockaddr*)client_addr, &client_len);
            
            if ((ntohs(ack_packet.flags) & ACK_FLAG) && ntohl(ack_packet.ack_num) == ntohl(fin_response.seq_num) + 1) {
                log_event("RCV ACK FOR FIN ACK=%u", ntohl(ack_packet.ack_num));
                break;
            }
        } 
        else {
            
            sendto(sockfd, &fin_response, sizeof(fin_response), 0,(struct sockaddr*)client_addr, client_len);
            log_event("RESEND FIN SEQ=%u", ntohl(fin_response.seq_num));
        }
    }
}









void handle_file_transfer(int sockfd, struct sockaddr_in* client_addr, socklen_t client_len, const char* output_file) {
    FILE* file = fopen(output_file, "wb");
    if (!file) {
        perror("Failed to open output file");
        return;
    }
    
    char packet_buffer[sizeof(struct sham_header) + MAX_PAYLOAD_SIZE];
    fd_set read_fds;
    struct timeval timeout;
    
    uint32_t next_expected = expected_seq_num;
    
    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(sockfd, &read_fds);
        
        timeout.tv_sec = 0;
        timeout.tv_usec = 100000;
        
        int activity = select(sockfd + 1, &read_fds, NULL, NULL, &timeout); 
        
        if (activity > 0 && FD_ISSET(sockfd, &read_fds)) {
            int n = recvfrom(sockfd, packet_buffer, sizeof(packet_buffer), 0, (struct sockaddr*)client_addr, &client_len);
            
            if (n < (int)sizeof(struct sham_header)) 
            {
                continue;
            }
            struct sham_header *packet = (struct sham_header *)packet_buffer;   
            
            
            if (ntohs(packet->flags) & FIN_FLAG) {
                log_event("RCV FIN SEQ=%u", ntohl(packet->seq_num));
                break;
            }
            
            
            if (n > (int)sizeof(struct sham_header)) {
                int data_len = n - sizeof(struct sham_header);
                char *data = packet_buffer + sizeof(struct sham_header);
                uint32_t seq_num = ntohl(packet->seq_num);
                
                
                if (should_drop_packet()) {
                    log_event("DROP DATA SEQ=%u", seq_num);
                    continue;
                }
                
                log_event("RCV DATA SEQ=%u LEN=%d", seq_num, data_len);
                
                if (seq_num == next_expected) {
                   
                    fwrite(data, 1, data_len, file);
                    next_expected += data_len;
                } else if (seq_num > next_expected) {
                   

                }
                
                
                struct sham_header ack_packet;
                ack_packet.ack_num = htonl(next_expected);
                ack_packet.flags = htons(ACK_FLAG);
                ack_packet.window_size = htons(window_size);
                
                    sendto(sockfd, &ack_packet, sizeof(ack_packet), 0,
                        (struct sockaddr*)client_addr, client_len);
                
                log_event("SND ACK=%u WIN=%u", next_expected, window_size);
            }
        }
    }
    
    fclose(file);
    
   
}













void handle_chat_mode(int sockfd, struct sockaddr_in* client_addr, socklen_t client_len) {
    
    char message[BUFFER_SIZE];
    fd_set read_fds;
    struct timeval timeout;
    
    printf("Chat mode activated. Type /quit to end chat.\n");
    
    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(sockfd, &read_fds);
        FD_SET(STDIN_FILENO, &read_fds);
    //////////////////////////////////////
        timeout.tv_sec = 0;
        timeout.tv_usec = 100000;
    //////////////////////////////////////
        int activity = select(sockfd + 1, &read_fds, NULL, NULL, &timeout);
        
        if (activity < 0 && errno != EINTR) {
            perror("Select error");
            break;
        }
        
        // Check for data from client
        if (FD_ISSET(sockfd, &read_fds)) {
            char packet_buffer[sizeof(struct sham_header) + BUFFER_SIZE];
            int n = recvfrom(sockfd, packet_buffer, sizeof(packet_buffer), 0, 
                           (struct sockaddr*)client_addr, &client_len);
            
            if (n < (int)sizeof(struct sham_header)) 
            {
                continue;
            }
            
            struct sham_header *packet = (struct sham_header *)packet_buffer;
            ///////////////////////
            // Check for FIN
            if (ntohs(packet->flags) & FIN_FLAG) {
                log_event("RCV FIN SEQ=%u", ntohl(packet->seq_num));
                break;
            }
            ////////////////////////////////
            // Check for data
            if (n > (int)sizeof(struct sham_header)) {
                int data_len = n - sizeof(struct sham_header);
                char *data = packet_buffer + sizeof(struct sham_header);
                data[data_len] = '\0';
                
                if (strcmp(data, "/quit") == 0) {
                    break;
                }
                
                printf("Client: %s\n", data);
                
                // Send ACK
                struct sham_header ack_packet;
                ack_packet.ack_num = htonl(ntohl(packet->seq_num) + data_len);
                ack_packet.flags = htons(ACK_FLAG);
                ack_packet.window_size = htons(window_size);
                
                sendto(sockfd, &ack_packet, sizeof(ack_packet), 0,(struct sockaddr*)client_addr, client_len);
            }
        }
        
        // Check for user input
        if (FD_ISSET(STDIN_FILENO, &read_fds)) {
            if (fgets(message, BUFFER_SIZE, stdin) != NULL) {
                // Remove newline
                message[strcspn(message, "\n")] = '\0';
                
                if (strcmp(message, "/quit") == 0) {
                    break;
                }
                
                // Send message with SHAM header
                struct sham_header packet;
                packet.seq_num = htonl(generate_initial_seq());
                packet.flags = 0;
                packet.window_size = htons(window_size);
                
                // Combine header and data into a single packet
                char combined_packet[sizeof(struct sham_header) + strlen(message) + 1];
                memcpy(combined_packet, &packet, sizeof(struct sham_header));
                memcpy(combined_packet + sizeof(struct sham_header), message, strlen(message) + 1);
                
                sendto(sockfd, combined_packet, sizeof(combined_packet), 0,
                      (struct sockaddr*)client_addr, client_len);
            }
        }
    }
}

int main(int argc, char* argv[]) {
//////////////////////////////////////
srand(time(NULL));  
//////////////////////////////////
    if (argc < 2) {
        printf("Usage: %s <port> [--chat] [loss_rate]\n", argv[0]);
        return 1;
    }
    
    int port = atoi(argv[1]);
    int chat_mode = 0;
    char* output_file = "received_file.txt";
    
    
    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "--chat") == 0) {
            chat_mode = 1;
        } else {
            loss_rate = atof(argv[i]);
        }
    }
////////////////////////////////////////    
   
    init_log();
///////////////////////////////////////
    
    if ((server_sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    
   
    if (bind(server_sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_sockfd);
        exit(EXIT_FAILURE);
    }
    
    printf("Server listening on port %d...\n", port);
    printf("Waiting for client connection...\n");
    
    client_len = sizeof(client_addr);
    
    
    perform_three_way_handshake(server_sockfd, &client_addr, client_len);
    
    printf("Client connected from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
    
    if (chat_mode) {
        handle_chat_mode(server_sockfd, &client_addr, client_len);
    } 
    else {
        printf("File transfer mode activated.\n");
        handle_file_transfer(server_sockfd, &client_addr, client_len, output_file);
    }
    
    
    perform_four_way_handshake(server_sockfd, &client_addr, client_len);
    
    printf("Connection closed.\n");
    
    if (log_file) 
    {
        fclose(log_file);
    }
    close(server_sockfd);
    return 0;
}