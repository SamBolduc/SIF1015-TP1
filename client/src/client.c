// Here's the client, client.c. The first part of this program opens the server FIFO,
// if it already exists, as a file. It then gets its own process ID, which forms some
// of the data that will be sent to the server. The client FIFO is created, ready for
// the next section.

#define _GNU_SOURCE

#include "client.h"
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <limits.h>
#include <stdbool.h>
#include <pthread.h>

void ConnectToServer(pid_t clientPID, int server_fifo_fd, int client_fifo_fd) {
    const unsigned int maxCommandLength = 100;
    char commandBuffer[maxCommandLength];
    char charBuffer = '\0';

    printf("Connecting to server\n");

    if (!fork()){
        while (true) {
            if (read(client_fifo_fd, &charBuffer, 1) != EOF)
                putc(charBuffer, stdout);
        }
    } else {
        while (true) { // Main loop
            fgets(commandBuffer, maxCommandLength, stdin);
            dprintf(server_fifo_fd, "%d %s", clientPID, commandBuffer);
        }
    }

}

int main() {
    pid_t clientPID;
    int server_fifo_fd, client_fifo_fd;
    char* client_fifo = NULL;

    // Open server fifo
    printf("Oppening Server FIFO ...\n");
    server_fifo_fd = open(SERVER_FIFO_NAME, O_WRONLY);
    if (server_fifo_fd == -1) {
        fprintf(stderr, "Sorry, no server\n");
        exit(EXIT_FAILURE);
    }
    printf("Success !\n");

    // Create client fifo
    printf("Creating Client FIFO ...\n");
    clientPID = getpid();
    asprintf(&client_fifo, CLIENT_FIFO_NAME, clientPID);
    if (mkfifo(client_fifo, 0777) == -1) {
        fprintf(stderr, "Sorry, can't make %s\n", client_fifo);
        goto Error;
    }
    printf("Success !\n");


    // Sending pid to server
    printf("Sending PID ...\n");
    dprintf(server_fifo_fd, "%u\n", clientPID);
    printf("Success !\n");

    // Open client fifo
    printf("Oppening Client FIFO ...\n");
    if ((client_fifo_fd = open(client_fifo, O_RDONLY)) == -1) {
        goto Error;
    }
    fcntl(client_fifo_fd, F_SETFL, fcntl(client_fifo_fd, F_GETFL, 0) | O_NONBLOCK);
    printf("Success !\n");

    ConnectToServer(clientPID, server_fifo_fd, client_fifo_fd);

    close(server_fifo_fd);
    unlink(client_fifo);

    if (client_fifo)
        free(client_fifo);

    exit(EXIT_SUCCESS);

Error:

    if (client_fifo)
        free(client_fifo);

    exit(EXIT_FAILURE);
}

