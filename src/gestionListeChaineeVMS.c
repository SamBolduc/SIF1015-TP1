//#########################################################
//#
//# Titre : 	Utilitaires Liste Chainee et CVS LINUX Automne 21
//#				SIF-1015 - Système d'exploitation
//#				Université du Québec à Trois-Rivières
//#
//# Auteur : 	Francois Meunier
//#	Date :		Septembre 2021
//#
//# Langage : 	ANSI C on LINUX 
//#
//#######################################

#include "gestionListeChaineeVMS.h"
#include "gestionVMS.h"

#include <pthread.h>

extern linkedList* threads;

extern linkedList* head;  //Pointeur de tête de liste
extern linkedList* queue; //Pointeur de queue de liste pour ajout rapide
extern int nbVM;       // nombre de VM actives

extern pthread_mutex_t headState;
extern pthread_mutex_t threadsState;

//#######################################
//# Recherche un item dans la liste chaînée
//# ENTREE: Numéro de la ligne
//# RETOUR:	Un pointeur vers l'item recherché		
//# 		Retourne NULL dans le cas où l'item
//#			est introuvable
//#
linkedList* findItem(const int no){
	//La liste est vide 
	if ((!head)&&(!queue)) return NULL;

	//Pointeur de navigation
	linkedList *ptr = head;
	//Tant qu'un item suivant existe
	while (ptr) {
		//Déplacement du pointeur de navigation
		if (((infoVM*)ptr->data)->noVM == no)
			return ptr;
		ptr = ptr->next;
	}
	//On retourne un pointeur NULL
	return NULL;
}

//#######################################
//#
//# Recherche le PRÉDÉCESSEUR d'un item dans la liste chaînée
//# ENTREE: Numéro de la ligne a supprimer
//# RETOUR:	Le pointeur vers le prédécesseur est retourné		
//# 		Retourne NULL dans le cas où l'item est introuvable
//#
linkedList** findPrev(const int no){
	//La liste est vide 
	if ((!head)&&(!queue)) return NULL;
	//Pointeur de navigation
	linkedList **ptr = &head;
	//Tant qu'un item suivant existe
	while (*ptr){

		//Est-ce le prédécesseur de l'item recherché?
		if (((infoVM*)(*ptr)->data)->noVM == no){
			//On retourne un pointeur sur l'item précédent
			return ptr;
		}
		//Déplacement du pointeur de navigation
		ptr=&(*ptr)->next;
	}
	//On retourne un pointeur NULL
	return NULL;
}

linkedList* appendToLinkedList(linkedList** List, void* newData, size_t dataSize){
    if (List){
        while (*List){
            List = (linkedList**)&(*List)->next;
        }
        (*List) = (linkedList*)malloc(sizeof(linkedList));
        (*List)->data = malloc(dataSize);
        (*List)->next = NULL;

        memcpy((*List)->data, newData, dataSize);

        return *List;
    }

    return NULL;
}

void deleteLinkedListNode(linkedList** node){
    linkedList* nextNode = (linkedList*)(*node)->next;
    free((*node));
    *node = nextNode;
}

void freeLinkedList(linkedList** List){
    if (List){
        if (*List){
            if ((*List)->next){
                freeLinkedList((linkedList**)&(*List)->next);
            }
            free(*List);
            *List = NULL;
        }
    }
}

//#####################################################
//# Ajoute un item a la fin de la liste chaînée de VM
//# ENTREE: 
//#	RETOUR:  

void* addItem_T(void* args){
	pthread_mutex_lock(&headState);
	//Création de l'enregistrement en mémoire
	infoVM* newVM = (infoVM*)malloc(sizeof(infoVM));

	//Affectation des valeurs des champs
	newVM->noVM = ++nbVM;
	newVM->busy = 0;
	newVM->ptrDebutVM = (unsigned short*)malloc(sizeof(unsigned short)*VM_SEGMENT_SIZE);

	queue = appendToLinkedList(&head, newVM, sizeof(infoVM));

	pthread_mutex_unlock(&headState);

	//printf("ADD OVER\n");

	return NULL;
}

void addItem() {
	pthread_t threadID;
	pthread_create(&threadID, NULL, &addItem_T, NULL);

	pthread_mutex_lock(&threadsState);
	appendToLinkedList(&threads, &threadID, sizeof(pthread_t));
	pthread_mutex_unlock(&threadsState);
}

//#######################################
//# Retire un item de la liste chaînée
//# ENTREE: noVM: numéro du noeud a retirer 
void* removeItem_T(void* args){

	pthread_mutex_lock(&headState);

	printf("vm no %d threre is %d VMS\n", *(int*)args, nbVM);

	linkedList** list = NULL;

	/* Search for the vm to delete. Retry if not found */
	while (!list) {
		list = findPrev(*(int*)args);

		if (list) {
			printf("listid : %d\n", ((infoVM*)(*list)->data)->noVM);
			break;
		} else {
			printf("Searching for vm ...\n");
		}

		pthread_mutex_unlock(&headState);
		sleep(1);
		pthread_mutex_lock(&headState);
	}

	/* Wait for the vm to complete its tasks */
	while (((infoVM*)((*list)->data))->busy){
		printf("VM Busy\n");
		pthread_mutex_unlock(&headState);
		sleep(1);
		pthread_mutex_lock(&headState);
	}

	deleteLinkedListNode(list);

	/* Assign the end of the list to the queue */
	if (*list){
		while((*list)->next){
			list = &((*list)->next);
		}
		queue = *list;
	}

	pthread_mutex_unlock(&headState);

	free(args);

	//printf("REMOVE OVER\n");

	return NULL;
}

void removeItem(const int noVM) {
	pthread_t threadID;
	int* noVM_p = (int*)malloc(sizeof(int));
	*noVM_p = noVM;
	pthread_create(&threadID, NULL, &removeItem_T, noVM_p);

	pthread_mutex_lock(&threadsState);
	appendToLinkedList(&threads, &threadID, sizeof(pthread_t));
	pthread_mutex_unlock(&threadsState);
}

//#######################################
//#
//# Affiche les items dont le numéro séquentiel est compris dans une plage
//#

typedef struct {
	int x;
	int y;
} Vector2i;

void* listItems_T(void* args){

	int i;
	//Affichage des entêtes de colonnes
	pthread_mutex_lock(&headState);
	printf("noVM  Busy?		Adresse Debut VM                        \n");
	printf("========================================================\n");
	linkedList *ptr = head; //premier element
	for (i = ((Vector2i*)args)->x; (i <= ((Vector2i*)args)->y) && ptr; i++) {
		printf("%d \t %d \t %p\n", ((infoVM*)ptr->data)->noVM, ((infoVM*)ptr->data)->busy, ((infoVM*)ptr->data)->ptrDebutVM);
		
		ptr = ptr->next;
	}

	//Affichage des pieds de colonnes
	printf("========================================================\n\n");
	pthread_mutex_unlock(&headState);

	free(args);

	//printf("LIST OVER\n");

	return NULL;
}

void listItems(const int start, const int end){
	pthread_t threadID;
	Vector2i* args = (Vector2i*)malloc(sizeof(Vector2i));
	args->x = start;
	args->y = end;

	pthread_create(&threadID, NULL, &listItems_T, args);

	pthread_mutex_lock(&threadsState);
	appendToLinkedList(&threads, &threadID, sizeof(pthread_t));
	pthread_mutex_unlock(&threadsState);
}