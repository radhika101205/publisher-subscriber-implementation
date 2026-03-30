#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080

int main(int argc, char const *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <PUB1|PUB2>\n", argv[0]);
        return -1;
    }

    const char *pub_id = argv[1];
    int sock = 0;
    struct sockaddr_in serv_addr;

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

    printf("%s connected to Broker.\n", pub_id);

    if (strcmp(pub_id, "PUB1") == 0) {
        while (1) {
            // PUB1 sends sports and weather simultaneously
            send(sock, "PUB1:sports:vkohli\n", 19, 0);
            send(sock, "PUB1:weather:rainy\n", 19, 0);
            printf("PUB1 published messages.\n");
            sleep(10); // Wait 10 seconds for the next cycle
        }
    } else if (strcmp(pub_id, "PUB2") == 0) {
        sleep(5); // Offset by 5 seconds so it alternates with PUB1
        while (1) {
            // PUB2 sends only technology
            send(sock, "PUB2:technology:cricket\n", 24, 0);
            printf("PUB2 published messages.\n");
            sleep(10); // Wait 10 seconds for the next cycle
        }
    } else {
        printf("Invalid Publisher ID. Use PUB1 or PUB2.\n");
    }

    close(sock);
    return 0;
}