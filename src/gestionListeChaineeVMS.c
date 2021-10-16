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

void deleteLinkedListNode(linkedList** node) {
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
void addItem(){
	//Création de l'enregistrement en mémoire
	infoVM* newVM = (infoVM*)calloc(1, sizeof(infoVM));

	//Affectation des valeurs des champs
	newVM->noVM = ++nbVM;
	newVM->ptrDebutVM = (unsigned short*)malloc(sizeof(unsigned short)*VM_SEGMENT_SIZE);
	
	pthread_mutex_lock(&headState);
	queue = appendToLinkedList(&head, newVM, sizeof(infoVM));
	pthread_create(&((infoVM*)queue->data)->vmProcess, NULL, &virtualMachine, ((infoVM*)queue->data));
	pthread_mutex_unlock(&headState);
}

//#######################################
//# Flag a vm for deletion
//# ENTREE: noVM: numéro du noeud a retirer 
void removeItem(const int noVM){
	linkedList* VM = findItem(noVM);

	((infoVM*)VM->data)->kill = 1;
}

//#######################################
//#
//# Affiche les items dont le numéro séquentiel est compris dans une plage
//#

typedef struct {
	int x;
	int y;
} Vector2i;

void listItems(const int start, const int end) {
	int i;
	//Affichage des entêtes de colonnes
	pthread_mutex_lock(&headState);
	printf("noVM    Busy?    Adresse Debut VM        kill ?              \n");
	printf("=============================================================\n");
	linkedList *ptr = head; //premier element
	for (i = start; (i <= end) && ptr; i++) {
		printf("%d \t %d \t %p \t %s\n", ((infoVM*)ptr->data)->noVM, ((infoVM*)ptr->data)->busy, ((infoVM*)ptr->data)->ptrDebutVM, ((infoVM*)ptr->data)->kill ? "flagged for deletion" : "alive");
		
		ptr = ptr->next;
	}

	//Affichage des pieds de colonnes
	printf("=============================================================\n\n");
	pthread_mutex_unlock(&headState);
}