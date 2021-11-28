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
    newVM->consoleState = &client->fifoState;
    newVM->console = &client->clientFifo;
	
	pthread_mutex_lock(&client->vmState); /* Lock head */
	
    AppendRefToLinkedList(&client->vms, newVM); /* Add the VM to the linked list */
	pthread_create(&((VirtualMachine*)(client->vms->data))->vmProcess, NULL, &virtualMachine, ((VirtualMachine*)client->vms->data)); /* Create the VM in a new thread */
	
    pthread_mutex_unlock(&client->vmState); /* Unlock head */
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
    
    if (!(VM = findItem(client, noVM))){
        return;
    }

	((VirtualMachine*)VM->data)->kill = 1;
	((VirtualMachine*)VM->data)->noVM = 0;
	client->nbVM--;

	VM = VM->next;
	while (VM) {
		if (((VirtualMachine*)VM->data)->noVM)
			((VirtualMachine*)VM->data)->noVM--;
		VM = VM->next;
	}
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
    int i;

    if (!client) {
        return;
    }

	/* Affichage des entêtes de colonnes */
	pthread_mutex_lock(&client->vmState); /* Lock head */
	pthread_mutex_lock(&client->fifoState); /* Lock console */
	
    printf("noVM    Busy?    Adresse Debut VM        kill ?              \n");
	printf("=============================================================\n");
	list = client->vms; /* premier element */
	for (i = start; (i <= end) && list; i++) {
        currentVM = (VirtualMachine*)list->data;
		printf("%d \t %d \t %p \t %s\n", currentVM->noVM, currentVM->busy, (void*)currentVM->ptrDebutVM, currentVM->kill ? "flagged for deletion" : "alive");
		
		list = list->next;
	}

	/* Affichage des pieds de colonnes */
	printf("=============================================================\n\n");
	
    pthread_mutex_unlock(&client->fifoState); /* Unlock console */
	pthread_mutex_unlock(&client->vmState); /* Unlock head */
}

int dispatchJob(ClientContext* client, int noVM, char* sourcefname) {
    LinkedList *VM = findItem(client, noVM);
    
    pthread_mutex_lock(&client->fifoState);
    
    if (!((VirtualMachine*)VM->data)->kill){
        printf("Job %s dispatched to vm %d !\n", sourcefname, noVM);        
        AppendToLinkedList(&((VirtualMachine*)VM->data)->binaryList, sourcefname, sizeof(char)*strlen(sourcefname));
    } else {
        printf("Couldn't dispatch job %s ! VM %d already flagged for deletion !\n", sourcefname, noVM);
    }

    pthread_mutex_unlock(&client->fifoState);

    return(1);
}