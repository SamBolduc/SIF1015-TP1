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

    LinkedList** node = NULL;

    // Kill and free all of the client's vms
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
    pthread_mutex_unlock(&client->fifoState);
    pthread_mutex_destroy(&client->vmState);
    pthread_mutex_destroy(&client->fifoState);
    FreeLinkedList(&client->vms);

    node = SearchDataInList(&clients, client);
    if (!node) {
        printf("Client not found !\n");
    }

    DeleteLinkedListNode(node);

    printf("Client [%p] Removed !\n", (void*)client->clientIO);
}

static void* processClientsTransaction(void* args) {
    ClientContext* client = (ClientContext*)args;
    char buffer[1024];
    char* queryPointer = NULL;

    if (!client)
        return NULL;

    printf("New client [%p] CONNECTED !\n", (void*)client->clientIO);

    IOWrite(client->clientIO, "Welcome !\n");

    while (true) {
        IORead(client->clientIO, buffer, 1024);
        printf("Query from [%p] >%s<\n", (void*)client->clientIO, buffer);
            
        switch (toupper(buffer[0])) {
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
                    removeItem(client, atoi(noVM));
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
                //dprintf(client->clientFifo, "Unrecognized query.");
                break;
        }
    }

    return NULL;
}

static void addClient(IOClient* clientIO) {
    ClientContext* newClient = (ClientContext*)calloc(1, sizeof(ClientContext));

    newClient->clientIO = clientIO;
    if (pthread_mutex_init(&newClient->vmState, NULL) || pthread_mutex_init(&newClient->fifoState, NULL)) {
        printf("Error: Couldn't init client mutexes\n");
        return;
    }
    pthread_create(&newClient->clientThread, NULL, processClientsTransaction, AppendRefToLinkedList(&clients, newClient)->data);
}

/*
static void checkClients() {
    ClientList* clientList = clients;
    ClientContext* client;

    
    while (clientList) {
        client = (ClientContext*)clientList->data;
        clientList = clientList->next;

        pthread_mutex_lock(&client->fifoState);
        // Check if the clients is disconnected
        if (checkFifo(client->clientFifo) == -1){
            // FreeClient
            printf("Client [%d] CRASHED !\n", client->clientPID);
            removeClient(client);
            continue;
        }
        pthread_mutex_unlock(&client->fifoState);

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