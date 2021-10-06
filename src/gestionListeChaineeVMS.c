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

extern linkedList* head;  //Pointeur de tête de liste
extern linkedList* queue; //Pointeur de queue de liste pour ajout rapide
extern int nbVM;       // nombre de VM actives

//#######################################
//# Recherche un item dans la liste chaînée
//# ENTREE: Numéro de la ligne
//# RETOUR:	Un pointeur vers l'item recherché		
//# 		Retourne NULL dans le cas où l'item
//#			est introuvable
//#
linkedList* findItem(const int no){
	int i;
	//La liste est vide 
	if ((!head)&&(!queue)) return NULL;

	//Pointeur de navigation
	linkedList *ptr = head;
	//Tant qu'un item suivant existe
	for (i = 1; i < no && ptr; i++) {
		//Déplacement du pointeur de navigation
		ptr = ptr->next;
	}
	//On retourne un pointeur NULL
	return ptr;
}

//#######################################
//#
//# Recherche le PRÉDÉCESSEUR d'un item dans la liste chaînée
//# ENTREE: Numéro de la ligne a supprimer
//# RETOUR:	Le pointeur vers le prédécesseur est retourné		
//# 		Retourne NULL dans le cas où l'item est introuvable
//#
linkedList* findPrev(const int no){
	return findItem(no - 1);
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
void addItem() {
	//Création de l'enregistrement en mémoire
	infoVM* newVM = (infoVM*)malloc(sizeof(infoVM));

	//Affectation des valeurs des champs
	newVM->busy = 0;
	newVM->ptrDebutVM = (unsigned short*)malloc(sizeof(unsigned short)*VM_SEGMENT_SIZE);

	queue = appendToLinkedList(&head, newVM, sizeof(infoVM));
	nbVM++;
}

//#######################################
//# Retire un item de la liste chaînée
//# ENTREE: noVM: numéro du noeud a retirer 
void removeItem(const int noVM) {
	int i;
	linkedList** list = &head;

	for (i = 1; i < noVM && *list; i++){
		list = &((*list)->next);
	}
	deleteLinkedListNode(list);

	if (*list){
		while((*list)->next){
			list = &((*list)->next);
		}
		queue = *list;
	}
}

//#######################################
//#
//# Affiche les items dont le numéro séquentiel est compris dans une plage
//#
void listItems(const int start, const int end){

	int i;
	//Affichage des entêtes de colonnes
	printf("noVM  Busy?		Adresse Debut VM                        \n");
	printf("========================================================\n");

	linkedList *ptr = head; //premier element
	for (i = start; i <= end && ptr; i++) {
		printf("%d \t %d \t %p\n", i, ((infoVM*)ptr->data)->busy, ((infoVM*)ptr->data)->ptrDebutVM);
		
		ptr = ptr->next;
	}

	//Affichage des pieds de colonnes
	printf("========================================================\n\n");
}