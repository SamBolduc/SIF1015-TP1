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
#include <ctype.h>
#include <signal.h>

#define NonNull(a) { \
    if (!(a))        \
        return;      \
}

static const serverObject emptyServer;

serverObject setupServer() {
    serverObject newServer = emptyServer;

    if (openFifo("/tmp/serv_fifo", &newServer.serverFifo, FIFO_READONLY) < 0)
        printf("Error : name of the error \n");

    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
        printf("Error: signal\n");

    return newServer;
}

ClientContext* searchClient(serverObject* server, __pid_t clientPID) {
    ClientList* list = server->clients;

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
    printf("New client [%d] CONNECTED !\n", clientPID);
    return (ClientContext*)AppendRefToLinkedList(&server->clients, newClient)->data;
}

void removeClient(serverObject *server, ClientContext* client) {
    VirtualMachine* currentVM = NULL;
    VMList* clientVMS = client->vms;

    LinkedList** node = NULL;

    // Kill and free all of the client's vms
    while (clientVMS) {
        currentVM = (VirtualMachine*)clientVMS->data;
        printf("Killing client [%d] VM_PID [%lu]\n", client->clientPID, currentVM->vmProcess);
        pthread_cancel(currentVM->vmProcess);

        removeItem(client, currentVM->noVM);
        clientVMS = clientVMS->next;
    }

    // free the client itself
    close(client->clientFifo);
    pthread_mutex_unlock(&client->vmState);
    pthread_mutex_unlock(&client->fifoState);
    pthread_mutex_destroy(&client->vmState);
    pthread_mutex_destroy(&client->fifoState);
    FreeLinkedList(&client->vms);

    node = SearchDataInList(&server->clients, client);
    if (!node) {
        printf("Client not found !\n");
    }

    DeleteLinkedListNode(node);

    printf("Client [%d] Removed !\n", client->clientPID);
}

void processClientsTransaction(serverObject* server, ClientContext* client, char* queryBuffer) {
    char* queryPointer = NULL;
    char* query = NULL;

    if (!client)
        return;

    if (queryBuffer)
        printf("Query from [%d] >%s<\n", client->clientPID, queryBuffer);
    query = strtok_r(queryBuffer, " ", &queryPointer);
        
    if (!query)
        return;
    switch (toupper(query[0])) {
        case 'A':
            addItem(client);
            break;

        case 'L':
            {
                char* nStart = NULL, *nEnd = NULL;
                NonNull(nStart = strtok_r(NULL, "-", &queryPointer));
                NonNull(nEnd = strtok_r(NULL, " ", &queryPointer));
                listItems(client, atoi(nStart), atoi(nEnd));
            }
            break;

        case 'E':
            {
                char* noVM = NULL;
                NonNull(noVM = strtok_r(NULL, " ", &queryPointer));
                removeItem(client, atoi(noVM));
            }
            break;

        case 'X':
            {
                char* noVM = NULL, *fileName = NULL;
                NonNull(noVM = strtok_r(NULL, " ", &queryPointer));
                NonNull(fileName = strtok_r(NULL, " ", &queryPointer));
                fileName[strlen(fileName)] = 0;
                dispatchJob(client, atoi(noVM), fileName);
            }
            break;

        case 'Q':
            {
                printf("Client [%d] DISCONNECTED !\n", client->clientPID);
                removeClient(server, client);
            }
            break;

        default: // Ignores malformed querries
	        dprintf(client->clientFifo, "Unrecognized query.");
            break;
    }
}

void checkClients(serverObject* server) {
    ClientList* clientList = NULL;
    ClientContext* client;

    clientList = server->clients;
    while (clientList) {
        client = (ClientContext*)clientList->data;
        clientList = clientList->next;

        pthread_mutex_lock(&client->fifoState);
        // Check if the clients is disconnected
        if (checkFifo(client->clientFifo) == -1){
            // FreeClient
            printf("Client [%d] CRASHED !\n", client->clientPID);
            removeClient(server, client);
            continue;
        } /*else {
            printf("Client [%d] VALID\n", client->clientPID);
        }*/
        pthread_mutex_unlock(&client->fifoState);

    }
}

/*
    Will check clients fifos at a regular interval to free their context if they disconnects,
    without notifying the server beforehand.
*/
void* threadedClientCheck(void* args) {
    while (true) {
        checkClients((serverObject*)args);
        sleep(1);
    }

    return NULL;
}

int processClients(serverObject* server) {
    #define transactionBufferSize 100
    char transactionBuffer[transactionBufferSize] = "";
    ssize_t nbOfCharactersRead = 0;
    char* slicePointer = NULL;
    char* token = NULL;
    __pid_t clientPID = 0;
    ClientContext* client = NULL;
    pthread_t clientCheckThread;

    // Todo check for errors
    pthread_create(&clientCheckThread, NULL, &threadedClientCheck, server);

    // Process incomming querries
    while ((nbOfCharactersRead = read(server->serverFifo, transactionBuffer, transactionBufferSize - 1)) != -1) {
        if (nbOfCharactersRead == 0) { // If we have a broken pipe then we wait a hot second for clients to connect
            sleep(1);
            continue;
        }
        strtok(transactionBuffer, "\n");
        if (!(token = strtok_r(transactionBuffer, " ", &slicePointer)))
            continue; // Ignnore malformed querries
        clientPID = atoi(token);

        // Search for client and create it if not found
        if (!(client = searchClient(server, clientPID)))
            client = addClient(server, clientPID);

        // Process the lient query
        processClientsTransaction(server, client, slicePointer);
        
        memset(transactionBuffer, 0, sizeof(transactionBuffer));
    }

    pthread_cancel(clientCheckThread);

    return 0;
}

void cleanupServer(serverObject* server) {
    if (closeFifo(&server->serverFifo) < 0)
        printf("Error : name of the error \n");
}