/*
    Copyright (C) 2021 Killian RAIMBAUD [Asayu] (killian.rai@gmail.com)
    Copyright (C) 2021 Francois Meunier

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
#include "client.h"
#include "virtualMachine.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*
  #####################################################
  # Ajoute un item a la fin de la liste chaînée de VM
  # ENTREE: 
  #	RETOUR:
*/
void addItem(ClientContext* client) {
    /* Création de l'enregistrement en mémoire */
    VirtualMachine* newVM = (VirtualMachine*)calloc(1, sizeof(VirtualMachine));

    /* Affectation des valeurs des champs */
    newVM->noVM = ++client->nbVM;
    newVM->ptrDebutVM = (unsigned short*)malloc(sizeof(unsigned short)*VM_SEGMENT_SIZE);
    newVM->consoleState = &client->ioState;
    newVM->console = client->clientIO;
    
    pthread_mutex_lock(&client->vmState); /* Lock head */
    
    newVM = AppendRefToLinkedList(&client->vms, newVM)->data; /* Add the VM to the linked list */
    pthread_create(&newVM->vmProcess, NULL, &virtualMachine, newVM); /* Create the VM in a new thread */

    pthread_mutex_lock(&client->ioState);
    IOWrite(client->clientIO, "New VM => #%d \t Pointer: %p \t\n", newVM->noVM, (void*)newVM->ptrDebutVM);
    pthread_mutex_unlock(&client->ioState);

    pthread_mutex_unlock(&client->vmState); /* Unlock head */
}

void freeVM(VirtualMachine* VM) {
    if (!VM)
        return;
    free(VM->ptrDebutVM);
    {
        LinkedList* list = VM->binaryList;
        while (list) {
            free(list->data);
            list = list->next;
        }
    }
    FreeLinkedList(&VM->binaryList);
    free(VM);
}

VMList* findItem(ClientContext* client, const unsigned int noVM) {
    VMList* list = NULL;

    if (!client)
        return NULL;
    list = client->vms;
    
    while (list) {
        if (((VirtualMachine*)list->data)->noVM == noVM)
            return list;
        list = list->next;
    }
    return NULL;
}

/*
  #######################################
  # Flag a vm for deletion
  # ENTREE: noVM: numéro du noeud a retirer
*/
void removeItem(ClientContext* client, const unsigned int noVM) {
    VMList* VM = NULL;

    if (!client)
        return;
    if (!(VM = findItem(client, noVM)))
        return;

    ((VirtualMachine*)VM->data)->kill = 1;
    ((VirtualMachine*)VM->data)->noVM = 0;
    client->nbVM--;

    VM = VM->next;
    while (VM) {
        if (((VirtualMachine*)VM->data)->noVM)
            ((VirtualMachine*)VM->data)->noVM--;
        VM = VM->next;
    }

    pthread_mutex_lock(&client->ioState);
    IOWrite(client->clientIO, "Flagged VM %d for deletion\n", noVM);
    pthread_mutex_unlock(&client->ioState);
}

/*
  #######################################
  #
  # Affiche les items dont le numéro séquentiel est compris dans une plage
  #
*/
void listItems(ClientContext* client, const int start, const int end) {
    VMList *list = NULL;
    VirtualMachine* currentVM = NULL;

    if (!client) {
        return;
    }

    /* Affichage des entêtes de colonnes */
    pthread_mutex_lock(&client->vmState); /* Lock head */
    pthread_mutex_lock(&client->ioState); /* Lock console */

    IOWrite(client->clientIO, "noVM    Busy?    Adresse Debut VM        kill ?              \n");
    IOWrite(client->clientIO, "=============================================================\n");
    list = client->vms; // premier element
    for (int i = start; (i <= end) && list; i++) {
        currentVM = (VirtualMachine*)list->data;
        IOWrite(client->clientIO, "%d \t %d \t %p \t %s\n", currentVM->noVM, currentVM->busy, (void*)currentVM->ptrDebutVM, currentVM->kill ? "flagged for deletion" : "alive");
        
        list = list->next;
    }

    // Affichage des pieds de colonnes
    IOWrite(client->clientIO, "=============================================================\n\n");

    pthread_mutex_unlock(&client->ioState); /* Unlock console */
    pthread_mutex_unlock(&client->vmState); /* Unlock head */
}

int dispatchJob(ClientContext* client, int noVM, char* sourcefname) {
    LinkedList *VM = findItem(client, noVM);
    
    pthread_mutex_lock(&client->ioState);

    if (!VM) {
        IOWrite(client->clientIO, "Couldn't dispatch job %s ! No such VM %d !\n", sourcefname, noVM);
        pthread_mutex_unlock(&client->ioState);
        return 1;
    }

    if (!((VirtualMachine*)VM->data)->kill){
        IOWrite(client->clientIO, "Job %s dispatched to vm %d !\n", sourcefname, noVM);        
        AppendToLinkedList(&((VirtualMachine*)VM->data)->binaryList, sourcefname, sizeof(char)*(strlen(sourcefname) + 1));
    } else {
        IOWrite(client->clientIO, "Couldn't dispatch job %s ! VM %d already flagged for deletion !\n", sourcefname, noVM);
    }

    pthread_mutex_unlock(&client->ioState);

    return 1;
}


void listFiles(ClientContext* client){
    if (!client)
        return;

    pthread_mutex_lock(&client->ioState);
    
    char cmd[256];

    snprintf(cmd, sizeof(cmd), "ls -l /proc/%d/fd", getpid());
    FILE *ls = popen(cmd, "r");

    char buf[256];
    while (fgets(buf, sizeof(buf), ls) != 0) {
        IOWrite(client->clientIO, buf);
    }

    pclose(ls);

    pthread_mutex_unlock(&client->ioState);
}

void showStat(ClientContext* client, const unsigned int noVM, char* fileName){
    VMList* VM = NULL;

    if (!client)
        return;
    if (!(VM = findItem(client, noVM)))
        return;

    pthread_mutex_lock(&client->ioState);
    
    char cmd[256];

    snprintf(cmd, sizeof(cmd), "stat %s", fileName);
    FILE *stat = popen(cmd, "r");

    char buf[256];
    while (fgets(buf, sizeof(buf), stat) != 0) {
        IOWrite(client->clientIO, buf);
    }

    pclose(stat);

    pthread_mutex_unlock(&client->ioState);
}