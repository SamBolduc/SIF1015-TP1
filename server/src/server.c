/*
    Copyright (C) 2021 Killian RAIMBAUD [Asayu] (killian.rai@gmail.com)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; version 2 of the License.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#define _GNU_SOURCE
#include "server.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

static const serverObject emptyServer;

serverObject setupServer() {
    serverObject newServer = emptyServer;

    if (openFifo("/tmp/serv_fifo", &newServer.serverFifo, FIFO_READONLY) < 0)
        printf("Error : name of the error \n");

    return newServer;
}

ClientContext* searchClient(serverObject* server, __pid_t clientPID) {
    clientList* list = server->clients;

    while (list) {
        if (((ClientContext*)list->data)->clientPID == clientPID)
            return (ClientContext*)list->data;
        list = list->next;
    }
    return NULL;
}

ClientContext* addClient(serverObject* server, __pid_t clientPID) {
    ClientContext* newClient = (ClientContext*)calloc(1, sizeof(ClientContext));
    char* clientFifoPath = NULL;

    newClient->clientPID = clientPID;
    asprintf(&clientFifoPath, "/tmp/cli_%d_fifo", clientPID);
    if (openFifo(clientFifoPath, &newClient->clientFifo, FIFO_WRITEONLY) < 0) {
        free(clientFifoPath);
        return NULL;
    }
    free(clientFifoPath);
    if (pthread_mutex_init(&newClient->vmState, NULL) || pthread_mutex_init(&newClient->fifoState, NULL)) {
        printf("Error: Couldn't init client mutexes\n");
        return NULL;
    }
    printf("New client [%d] connected !\n", clientPID);
    return (ClientContext*)AppendRefToLinkedList(&server->clients, newClient)->data;
}

void processClientsTransaction(ClientContext* client, char* querryBuffer) {
    char* querryPointer = NULL;
    char* querry = NULL;

    if (!client)
        return;

    querry = strtok_r(querryBuffer, " ", &querryPointer);
    if (querry)
        printf("Querry from [%d] >%s<\n", client->clientPID, querry);
    if (!querry)
        return;
    switch (querry[0]) {
        case 'A':
            addItem(client);
            break;

        case 'L':
            {
                int nStart = atoi(strtok_r(NULL, "-", &querryPointer));
                int nEnd = atoi(strtok_r(NULL, " ", &querryPointer));
                listItems(client, nStart, nEnd);
            }
            break;

        case 'E':
            {
                int noVM = atoi(strtok_r(NULL, " ", &querryPointer));
                removeItem(client, noVM);
            }
            break;

        case 'X':
            {
                int noVM = atoi(strtok_r(NULL, " ", &querryPointer));
                char* fileName = strtok_r(NULL, "\n", &querryPointer);
                dispatchJob(client, noVM, fileName);
            }
            break;

        default: // Ignores malformed querries
            break;
    }
}

int processClients(serverObject* server) {
    #define transactionBufferSize 100
    char transactionBuffer[transactionBufferSize] = "";
    ssize_t nbOfCharactersRead = 0;
    char* slicePointer = NULL;
    char* token = NULL;
    __pid_t clientPID = 0;
    ClientContext* client = NULL;

    // Process incomming querries
    while ((nbOfCharactersRead = read(server->serverFifo, transactionBuffer, transactionBufferSize - 1)) != -1) {
        strtok(transactionBuffer, "\n");
        if (!(token = strtok_r(transactionBuffer, " ", &slicePointer)))
            continue; // Ignnore malformed querries
        clientPID = atoi(token);

        // Search for client and create it if not found
        if (!(client = searchClient(server, clientPID)))
            client = addClient(server, clientPID);

        // Process the lient querry
        processClientsTransaction(client, slicePointer);
    }

    return 0;
}

void cleanupServer(serverObject* server) {
    if (closeFifo(&server->serverFifo) < 0)
        printf("Error : name of the error \n");
}
