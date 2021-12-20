#define _GNU_SOURCE
#include "IOManager.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

static int serverSocket;
static struct sockaddr_in serverAddress;

struct IOClient {
    int socket;
};

void IOInit() {
    int opt = 1; // soket option value

    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Error : Couldn't create socket !\n");
        exit(-1);
    }

    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        printf("Error : Couldn't set socket option !\n");
        exit(-1);
    }
    
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(6969);

    if (bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
        printf("Error : Couldn't bind address !\n");
        exit(-1);
    }
}

IOClient* IOGetClient() {
    IOClient* newClient = (IOClient*)malloc(sizeof(IOClient));

    if (listen(serverSocket, SOMAXCONN) < 0) {
        printf("Error: couldn't listen for incomming connections !\n");
        exit(-1);
    }

    int serverAddrLength = sizeof(serverAddress);
    if ((newClient->socket = accept(serverSocket, (struct sockaddr*)&serverAddress, (socklen_t*)&serverAddrLength)) < 0) {
        printf("Error: couldn't accept incomming connections !\n");
        exit(-1);
    }

    return newClient;
}

void IOCloseClient(IOClient* client) {
    if (!client)
        return;
    free(client);
}

void IORead(IOClient* client, char* buffer, size_t length) {
    if (!client | !buffer)
        return;
    read(client->socket, buffer, length);
}

void IOWrite(IOClient* client, char* format, ...) {
    char* buffer = NULL;
    va_list vaargs;

    if (!client)
        return;

    va_start(vaargs, format);
    vasprintf(&buffer, format, vaargs);
    va_end(vaargs);

    send(client->socket, buffer, strlen(buffer), 0);

    free(buffer);
}