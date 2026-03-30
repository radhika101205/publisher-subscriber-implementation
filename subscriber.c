#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main(int argc, char const *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <SUB1|SUB2|SUB3>\n", argv[0]);
        return -1;
    }

    const char *sub_id = argv[1];
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }

    // Send initialization message to map the FD in the broker
    char init_msg[20];
    sprintf(init_msg, "INIT:%s\n", sub_id);
    send(sock, init_msg, strlen(init_msg), 0);

    printf("%s connected to Broker and waiting for messages...\n", sub_id);

    while (1) {
        int valread = recv(sock, buffer, BUFFER_SIZE - 1, 0);
        if (valread > 0) {
            buffer[valread] = '\0';
            // Because messages are separated by \n, we print them cleanly
            printf("%s received -> %s", sub_id, buffer);
            memset(buffer, 0, BUFFER_SIZE);
        } else if (valread == 0) {
            printf("Broker disconnected.\n");
            break;
        }
    }

    close(sock);
    return 0;
}