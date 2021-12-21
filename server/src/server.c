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
#include "IOManager.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>

#define NonNull(a, b) { \
    if (!(a))        \
        return b;      \
}

static ClientList* clients;

/*
static ClientContext* searchClient(IOClient* clientIO) {
    ClientList* list = clients;

    while (list) {
        if (((ClientContext*)list->data)->clientIO == clientIO)
            return (ClientContext*)list->data;
        list = list->next;
    }
    return NULL;
}
*/

static void removeClient(ClientContext* client) {
    VirtualMachine* currentVM = NULL;
    VMList* clientVMS = client->vms;
    void* clientID = client->clientIO;

    LinkedList** node = NULL;

    // Kill and free all of the client's vms

    pthread_mutex_lock(&client->vmState);
    pthread_mutex_lock(&client->ioState);

    while (clientVMS) {
        currentVM = (VirtualMachine*)clientVMS->data;
        printf("Killing client [%p] VM_PID [%lu]\n", (void*)client->clientIO, currentVM->vmProcess);
        pthread_cancel(currentVM->vmProcess);

        removeItem(client, currentVM->noVM);
        clientVMS = clientVMS->next;
    }

    // free the client itself
    //close(client->clientFifo);
    pthread_mutex_unlock(&client->vmState);
    pthread_mutex_unlock(&client->ioState);
    pthread_mutex_destroy(&client->vmState);
    pthread_mutex_destroy(&client->ioState);
    FreeLinkedList(&client->vms);

    node = SearchDataInList(&clients, client);
    if (!node) {
        printf("Client not found !\n");
    }

    IOCloseClient(client->clientIO);
    DeleteLinkedListNode(node);
    free(client);

    printf("Client [%p] Removed !\n", clientID);

    pthread_exit(NULL);
}

static void* processClientsTransaction(void* args) {
    ClientContext* client = (ClientContext*)args;
    char buffer[1024];
    char* queryPointer = NULL;
    char* token = NULL;

    if (!client)
        return NULL;

    printf("New client [%p] CONNECTED !\n", (void*)client->clientIO);

    pthread_mutex_lock(&client->ioState);
    IOWrite(client->clientIO, "Welcome user [%p]\n", (void*)client->clientIO);
    pthread_mutex_unlock(&client->ioState);

    while (true) {
        if (IORead(client->clientIO, buffer, 1024) == -1){
            removeClient(client);
        }
        printf("Query from [%p] >%s<\n", (void*)client->clientIO, buffer);
            
        token = strtok_r(buffer, " \n\0", &queryPointer);
        switch (toupper(token[0])) {
            case 'A':
                addItem(client);
                break;

            case 'L':
                {
                    char* nStart = NULL, *nEnd = NULL;
                    NonNull(nStart = strtok_r(NULL, "-", &queryPointer), NULL);
                    NonNull(nEnd = strtok_r(NULL, " ", &queryPointer), NULL);
                    listItems(client, atoi(nStart), atoi(nEnd));
                }
                break;

            case 'E':
                {
                    char* noVM = NULL;
                    NonNull(noVM = strtok_r(NULL, " ", &queryPointer), NULL);
                    pthread_mutex_lock(&client->ioState);
                    removeItem(client, atoi(noVM));
                    pthread_mutex_unlock(&client->ioState);
                }
                break;

            case 'X':
                {
                    char* noVM = NULL, *fileName = NULL;
                    NonNull(noVM = strtok_r(NULL, " ", &queryPointer), NULL);
                    NonNull(fileName = strtok_r(NULL, " ", &queryPointer), NULL);
                    fileName[strlen(fileName)] = 0;
                    dispatchJob(client, atoi(noVM), fileName);
                }
                break;

            case 'Q':
                {
                    printf("Client [%p] DISCONNECTED !\n", (void*)client->clientIO);
                    removeClient(client);
                }
                break;

            default: // Ignores malformed querries
                IOWrite(client->clientIO, "Unrecognized query.");
                break;
        }
        memset(buffer, 0, 1024);
    }

    return NULL;
}

static void addClient(IOClient* clientIO) {
    pthread_t clientThread;
    ClientContext* newClient = (ClientContext*)calloc(1, sizeof(ClientContext));

    newClient->clientIO = clientIO;
    if (pthread_mutex_init(&newClient->vmState, NULL) || pthread_mutex_init(&newClient->ioState, NULL)) {
        printf("Error: Couldn't init client mutexes\n");
        return;
    }

    AppendRefToLinkedList(&clients, newClient);
    pthread_create(&clientThread, NULL, processClientsTransaction, newClient);
    pthread_detach(clientThread);
}

/*
static void checkClients() {
    ClientList* clientList = clients;
    ClientContext* client;

    
    while (clientList) {
        client = (ClientContext*)clientList->data;
        clientList = clientList->next;

        pthread_mutex_lock(&client->ioState);
        // Check if the clients is disconnected
        if (checkFifo(client->clientFifo) == -1){
            // FreeClient
            printf("Client [%d] CRASHED !\n", client->clientPID);
            removeClient(client);
            continue;
        }
        pthread_mutex_unlock(&client->ioState);

    }
    
}
*/

/*
    Will check clients fifos at a regular interval to free their context if they disconnects,
    without notifying the server beforehand.
*/
/*
static void* threadedClientCheck(void* args) {
    while (true) {
        checkClients();
        sleep(1);
    }

    return NULL;
}*/

void setupServer() {
    IOInit();
}


void processClients() {
    #define bufferSize 100
    char buffer[bufferSize];
    IOClient* newClient = NULL;

    // Listening for clients connections
    while ((newClient = IOGetClient())){
        memset(buffer, 0, bufferSize);
        IORead(newClient, buffer, bufferSize);

        addClient(newClient);
    }
}

void cleanupServer() {
    /*
    if (closeFifo(&server->serverFifo) < 0)
        printf("Error : name of the error \n");
        */
}