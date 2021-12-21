#include "system.h"

#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

static int socketFileDiscriptor;
static struct sockaddr_in serverAddress;

void ReadFromServer(char* buffer, size_t length) {
    if (!buffer)
        return;
    read(socketFileDiscriptor, buffer, length);
}

void SendToServer(char* buffer) {
    send(socketFileDiscriptor, buffer, strlen(buffer), 0);
}

void ConnectToServer() {
    // Create INET socket
    if ((socketFileDiscriptor = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0)) < 0) {
        printf("Couldn't create socket !\n");
        exit(-1);
    }

    // Convert text address to bin for struct
    if (inet_pton(AF_INET, "127.0.0.1", &serverAddress.sin_addr) <= 0) {
        printf("Invalid address !\n");
        exit(-1);
    }

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(6969);

    // Connect to server through setup socket
    printf("Connecting to server ...\n");
    while (connect(socketFileDiscriptor, (const struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        sleep(1);
    }

    printf("Connected !\n");
    sleep(1);
}

void CloseConnection() {
    
}