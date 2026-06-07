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

int sockfd;
struct sockaddr_in server_addr;
socklen_t server_len;
FILE* log_file = NULL;


uint32_t seq_num = 0;
uint32_t ack_num = 0;
uint16_t window_size = 65535;
float loss_rate = 0.0;

////////////////////////////

void init_log() {
    if (getenv("RUDP_LOG") != NULL && atoi(getenv("RUDP_LOG")) == 1) {
        log_file = fopen("client_log.txt", "w");
        if (log_file == NULL) {
            perror("Failed to open log file");
        }
    }
}
/////////////////////////
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
///////////////////////
int should_drop_packet() {
    if (loss_rate <= 0.0) return 0;
    return ((float)rand() / RAND_MAX) < loss_rate;
}

void perform_three_way_handshake(int sockfd, struct sockaddr_in* server_addr, socklen_t server_len) {
    struct sham_header syn_packet, syn_ack_packet, ack_packet;
    fd_set read_fds;
    struct timeval timeout;
    int retries = 0;
    

    syn_packet.seq_num = htonl(generate_initial_seq());
    syn_packet.flags = htons(SYN_FLAG);
    syn_packet.window_size = htons(window_size);
    
    sendto(sockfd, &syn_packet, sizeof(syn_packet), 0,(struct sockaddr*)server_addr, server_len);
    
    log_event("SND SYN SEQ=%u", ntohl(syn_packet.seq_num));
    seq_num = ntohl(syn_packet.seq_num) + 1;
    
    while (retries < MAX_RETRIES) {
        FD_ZERO(&read_fds);
        FD_SET(sockfd, &read_fds);
        ///////////////////////
        timeout.tv_sec = 5;
        timeout.tv_usec = 0;
        ///////////////
        int activity = select(sockfd + 1, &read_fds, NULL, NULL, &timeout);
        
        if (activity > 0 && FD_ISSET(sockfd, &read_fds)) {
            recvfrom(sockfd, &syn_ack_packet, sizeof(syn_ack_packet), 0, (struct sockaddr*)server_addr, &server_len);
            
            if ((ntohs(syn_ack_packet.flags) & (SYN_FLAG | ACK_FLAG)) == (SYN_FLAG | ACK_FLAG) &&ntohl(syn_ack_packet.ack_num) == seq_num) {
                log_event("RCV SYN-ACK SEQ=%u ACK=%u", ntohl(syn_ack_packet.seq_num), ntohl(syn_ack_packet.ack_num));
                ack_num = ntohl(syn_ack_packet.seq_num) + 1;
                break;
            }
        }
         else {

            sendto(sockfd, &syn_packet, sizeof(syn_packet), 0,(struct sockaddr*)server_addr, server_len);
            log_event("RESEND SYN SEQ=%u", ntohl(syn_packet.seq_num));
            retries++;
        }
    }
    
    if (retries >= MAX_RETRIES) {
        fprintf(stderr, "Handshake failed: Max retries exceeded\n");
        exit(EXIT_FAILURE);
    }
    
    
    ack_packet.ack_num = htonl(ack_num);
    ack_packet.flags = htons(ACK_FLAG);
    ack_packet.window_size = htons(window_size);
    
    sendto(sockfd, &ack_packet, sizeof(ack_packet), 0,(struct sockaddr*)server_addr, server_len);
    
    log_event("SND ACK FOR SYN ACK=%u", ack_num);
}

void perform_four_way_handshake(int sockfd, struct sockaddr_in* server_addr, socklen_t server_len) {
    struct sham_header fin_packet, ack_packet, fin_response;
    fd_set read_fds;
    struct timeval timeout;
    int retries = 0;
    
    fin_packet.seq_num = htonl(seq_num);
    fin_packet.flags = htons(FIN_FLAG);
    fin_packet.window_size = htons(window_size);
    
    sendto(sockfd, &fin_packet, sizeof(fin_packet), 0,(struct sockaddr*)server_addr, server_len);
    
    log_event("SND FIN SEQ=%u", seq_num);
    seq_num++;
    
    while (retries < MAX_RETRIES) {
        FD_ZERO(&read_fds);
        FD_SET(sockfd, &read_fds);
        
        timeout.tv_sec = 5;
        timeout.tv_usec = 0;
        
        int activity = select(sockfd + 1, &read_fds, NULL, NULL, &timeout);
        
        if (activity > 0 && FD_ISSET(sockfd, &read_fds)) {
            recvfrom(sockfd, &ack_packet, sizeof(ack_packet), 0, (struct sockaddr*)server_addr, &server_len);
            
            if ((ntohs(ack_packet.flags) & ACK_FLAG) &&  ntohl(ack_packet.ack_num) == seq_num) {
                log_event("RCV ACK FOR FIN ACK=%u", ntohl(ack_packet.ack_num));
                break;
            }
        } 
        else {
            sendto(sockfd, &fin_packet, sizeof(fin_packet), 0,(struct sockaddr*)server_addr, server_len);
            log_event("RESEND FIN SEQ=%u", seq_num - 1);
            retries++;
        }
    }
    
    if (retries >= MAX_RETRIES) {
        fprintf(stderr, "FIN handshake failed: Max retries exceeded\n");
        return;
    }
    
    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(sockfd, &read_fds);
        ////////////////////////
        timeout.tv_sec = 5;
        timeout.tv_usec = 0;
        ////////////////////////
        int activity = select(sockfd + 1, &read_fds, NULL, NULL, &timeout);
        
        if (activity > 0 && FD_ISSET(sockfd, &read_fds)) {
            recvfrom(sockfd, &fin_response, sizeof(fin_response), 0, (struct sockaddr*)server_addr, &server_len);
            
            if (ntohs(fin_response.flags) & FIN_FLAG) {
                log_event("RCV FIN SEQ=%u", ntohl(fin_response.seq_num));
                break;
            }
        }
    }
    
    ack_packet.ack_num = htonl(ntohl(fin_response.seq_num) + 1);
    ack_packet.flags = htons(ACK_FLAG);
    ack_packet.window_size = htons(window_size);
    
    sendto(sockfd, &ack_packet, sizeof(ack_packet), 0, (struct sockaddr*)server_addr, server_len);
    
    log_event("SND ACK FOR FIN ACK=%u", ntohl(ack_packet.ack_num));
}


void send_file(int sockfd, struct sockaddr_in* server_addr, socklen_t server_len, const char* input_file) {
    FILE* file = fopen(input_file, "rb");
    if (!file) {
        perror("Failed to open input file");
        return;
    }
    
    char buffer[MAX_PAYLOAD_SIZE];
    fd_set read_fds;
    struct timeval timeout;
    

    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, MAX_PAYLOAD_SIZE, file)) > 0) {
     
        struct sham_header packet;
        packet.seq_num = htonl(seq_num);
        packet.flags = 0;
        packet.window_size = htons(window_size);
        
        /////////////////////////////////////////
        char combined_packet[sizeof(struct sham_header) + bytes_read];
        memcpy(combined_packet, &packet, sizeof(struct sham_header));
        memcpy(combined_packet + sizeof(struct sham_header), buffer, bytes_read);
        ///////////////////////////////////////
        sendto(sockfd, combined_packet, sizeof(combined_packet), 0,
              (struct sockaddr*)server_addr, server_len);
        
        log_event("SND DATA SEQ=%u LEN=%zu", seq_num, bytes_read);
        
        
        int ack_received = 0;
        int retries = 0;
        
        while (!ack_received && retries < MAX_RETRIES) {
            FD_ZERO(&read_fds);
            FD_SET(sockfd, &read_fds);
            
            timeout.tv_sec = 0;
            timeout.tv_usec = RTO_MS * 1000;
            
            int activity = select(sockfd + 1, &read_fds, NULL, NULL, &timeout);
            
            if (activity > 0 && FD_ISSET(sockfd, &read_fds)) {
                struct sham_header ack_packet;
                recvfrom(sockfd, &ack_packet, sizeof(ack_packet), 0,  (struct sockaddr*)server_addr, &server_len);
                
                if (ntohs(ack_packet.flags) & ACK_FLAG) {
                    uint32_t received_ack = ntohl(ack_packet.ack_num);
                    log_event("RCV ACK=%u", received_ack);
                    
                    if (received_ack == seq_num + bytes_read) {
                        ack_received = 1;
                        seq_num += bytes_read;
                    }
                }
            }
             else {
                log_event("TIMEOUT SEQ=%u", seq_num);
                log_event("RETX DATA SEQ=%u LEN=%zu", seq_num, bytes_read);
                
                sendto(sockfd, combined_packet, sizeof(combined_packet), 0,(struct sockaddr*)server_addr, server_len);
                
                retries++;
            }
        }
        
        if (retries >= MAX_RETRIES) {
            fprintf(stderr, "Max retries exceeded for SEQ=%u\n", seq_num);
            break;
        }
    }
    
    fclose(file);
    

    struct sham_header fin_packet;
    fin_packet.seq_num = htonl(seq_num);
    fin_packet.flags = htons(FIN_FLAG);
    fin_packet.window_size = htons(window_size);
    
    sendto(sockfd, &fin_packet, sizeof(fin_packet), 0,(struct sockaddr*)server_addr, server_len);
    
    log_event("SND FIN SEQ=%u", seq_num);
}

void handle_chat_mode(int sockfd, struct sockaddr_in* server_addr, socklen_t server_len) {
    char message[BUFFER_SIZE];
    fd_set read_fds;
    struct timeval timeout;
    
    printf("Chat mode activated. Type /quit to end chat.\n");
    
    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(sockfd, &read_fds);
        FD_SET(STDIN_FILENO, &read_fds);
        
        timeout.tv_sec = 0;
        timeout.tv_usec = 100000;
        
        int activity = select(sockfd + 1, &read_fds, NULL, NULL, &timeout);
        
        if (activity < 0 && errno != EINTR) {
            perror("Select error");
            break;
        }
        
        if (FD_ISSET(sockfd, &read_fds)) {
            char packet_buffer[sizeof(struct sham_header) + BUFFER_SIZE];
            int n = recvfrom(sockfd, packet_buffer, sizeof(packet_buffer), 0, (struct sockaddr*)server_addr, &server_len);
            
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
                data[data_len] = '\0';
                
                if (strcmp(data, "/quit") == 0) {
                    break;
                }
                
                printf("Server: %s\n", data);
                
                struct sham_header ack_packet;
                ack_packet.ack_num = htonl(ntohl(packet->seq_num) + data_len);
                ack_packet.flags = htons(ACK_FLAG);
                ack_packet.window_size = htons(window_size);
                
                sendto(sockfd, &ack_packet, sizeof(ack_packet), 0,  (struct sockaddr*)server_addr, server_len);
            }
        }
        
        if (FD_ISSET(STDIN_FILENO, &read_fds)) {
            if (fgets(message, BUFFER_SIZE, stdin) != NULL) {
                message[strcspn(message, "\n")] = '\0';
                
                if (strcmp(message, "/quit") == 0) {
                    break;
                }
                
                struct sham_header packet;
                packet.seq_num = htonl(seq_num);
                packet.flags = 0;
                packet.window_size = htons(window_size);
                
                char combined_packet[sizeof(struct sham_header) + strlen(message) + 1];
                memcpy(combined_packet, &packet, sizeof(struct sham_header));
                memcpy(combined_packet + sizeof(struct sham_header), message, strlen(message) + 1);
                
                sendto(sockfd, combined_packet, sizeof(combined_packet), 0,(struct sockaddr*)server_addr, server_len);
                
                seq_num += strlen(message);
            }
        }
    }
}
int main(int argc, char* argv[]) {
   
srand(time(NULL));  
    if (argc < 3) {
        printf("Usage:\n");
        printf("  File Transfer Mode: %s <server_ip> <server_port> <input_file> <output_file_name> [loss_rate]\n", argv[0]);
        printf("  Chat Mode: %s <server_ip> <server_port> --chat [loss_rate]\n", argv[0]);
        return 1;
    }
    
    char* server_ip = argv[1];
    int server_port = atoi(argv[2]);
    int chat_mode = 0;
    char* input_file = NULL;
    char* output_file=NULL;
    
    
    if (argc >= 4 && strcmp(argv[3], "--chat") == 0) {
        chat_mode = 1;
        if (argc >= 5) {
            loss_rate = atof(argv[4]);
        }
    } 
    else {
        if (argc >= 5) {
            input_file = argv[3];
            output_file=argv[4];
            if (argc >= 6) {
                loss_rate = atof(argv[5]);
            }
        } else {
            printf("Invalid arguments for file transfer mode\n");
            return 1;
        }
    }
    

    init_log();
    
 
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    server_addr.sin_addr.s_addr = inet_addr(server_ip);
    
    server_len = sizeof(server_addr);
    
 
    perform_three_way_handshake(sockfd, &server_addr, server_len);
    
    printf("Connected to server at %s:%d\n", server_ip, server_port);
    
    if (chat_mode) {
        handle_chat_mode(sockfd, &server_addr, server_len);
    } 
    else {
        printf("Sending file: %s\n", input_file);
        send_file(sockfd, &server_addr, server_len, input_file);
    }
    

    perform_four_way_handshake(sockfd, &server_addr, server_len);
    
    printf("Connection closed.\n");
    
    if (log_file) fclose(log_file);
    close(sockfd);
    return 0;
}