#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <poll.h>

#define PORT 8080
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    struct pollfd fds[MAX_CLIENTS];
    int nfds = 1;

    // Fixed mapping descriptors
    int sub1_fd = -1, sub2_fd = -1, sub3_fd = -1;

    // Create server socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // Bind to port
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 5) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Broker started on port %d...\n", PORT);

    // Initialize poll structure
    memset(fds, 0, sizeof(fds));
    fds[0].fd = server_fd;
    fds[0].events = POLLIN;

    for (int i = 1; i < MAX_CLIENTS; i++) {
        fds[i].fd = -1;
    }

    while (1) {
        int poll_count = poll(fds, nfds, -1);
        if (poll_count < 0) {
            perror("Poll error");
            break;
        }

        for (int i = 0; i < nfds; i++) {
            if (fds[i].revents & POLLIN) {
                if (fds[i].fd == server_fd) {
                    // Accept new connection
                    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
                        perror("Accept failed");
                        continue;
                    }
                    printf("New client connected (FD: %d)\n", new_socket);
                    
                    // Add to poll fds
                    fds[nfds].fd = new_socket;
                    fds[nfds].events = POLLIN;
                    nfds++;
                } else {
                    // Read data from a client
                    char buffer[BUFFER_SIZE] = {0};
                    int valread = recv(fds[i].fd, buffer, BUFFER_SIZE - 1, 0);
                    
                    if (valread == 0) {
                        // Client disconnected
                        printf("Client disconnected (FD: %d)\n", fds[i].fd);
                        close(fds[i].fd);
                        
                        // Clear mapping if a subscriber disconnected
                        if (fds[i].fd == sub1_fd) sub1_fd = -1;
                        if (fds[i].fd == sub2_fd) sub2_fd = -1;
                        if (fds[i].fd == sub3_fd) sub3_fd = -1;
                        
                        fds[i].fd = -1; // Mark as available
                    } else {
                        // Process message(s). Using newline to split simultaneous messages.
                        char *line = strtok(buffer, "\n");
                        while (line != NULL) {
                            if (strncmp(line, "INIT:", 5) == 0) {
                                // Register subscriber
                                if (strstr(line, "SUB1")) sub1_fd = fds[i].fd;
                                else if (strstr(line, "SUB2")) sub2_fd = fds[i].fd;
                                else if (strstr(line, "SUB3")) sub3_fd = fds[i].fd;
                                printf("Registered %s to FD %d\n", line + 5, fds[i].fd);
                            } else {
                                // Handle Publisher messages: PUB1:sports:vkohli
                                char pub[10], topic[20], msg[50];
                                if (sscanf(line, "%[^:]:%[^:]:%s", pub, topic, msg) == 3) {
                                    printf("Broker received: %s\n", line);
                                    
                                    char out_msg[100];
                                    sprintf(out_msg, "%s:%s\n", topic, msg);
                                    
                                    // Fixed routing logic
                                    if (strcmp(topic, "sports") == 0) {
                                        if (sub1_fd != -1) send(sub1_fd, out_msg, strlen(out_msg), 0);
                                        if (sub3_fd != -1) send(sub3_fd, out_msg, strlen(out_msg), 0);
                                    } else if (strcmp(topic, "technology") == 0) {
                                        if (sub2_fd != -1) send(sub2_fd, out_msg, strlen(out_msg), 0);
                                        if (sub3_fd != -1) send(sub3_fd, out_msg, strlen(out_msg), 0);
                                    } else if (strcmp(topic, "weather") == 0) {
                                        if (sub1_fd != -1) send(sub1_fd, out_msg, strlen(out_msg), 0);
                                    }
                                }
                            }
                            line = strtok(NULL, "\n");
                        }
                    }
                }
            }
        }
    }
    return 0;
}