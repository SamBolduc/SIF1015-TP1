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

extern noeudVM* head;  //Pointeur de tête de liste
extern noeudVM* queue; //Pointeur de queue de liste pour ajout rapide
extern int nbVM;       // nombre de VM actives

extern semH;
extern semQ;

//#######################################
//# Recherche un item dans la liste chaînée
//# ENTREE: Numéro de la ligne
//# RETOUR:	Un pointeur vers l'item recherché		
//# 		Retourne NULL dans le cas où l'item
//#			est introuvable
//#
noeudVM* findItem(const int no){

	sem_wait(&semH);
	sem_wait(&semQ);
	//La liste est vide 
	if ((!head)&&(!queue)){
		sem_post(&semQ);
		sem_post(&semH);
		return NULL;
	}

	//Pointeur de navigation
	sem_wait(&head->semVM); // verrouille noeud de tete
	noeudVM * ptr = head;
	sem_post(&semQ);
	sem_post(&semH);

	if(ptr->VM.noVM == no) // premier noeudVM 
		return ptr;

	if(ptr->VM.noVM==no) // premier noeudVM
		return ptr; // retourner le noeud de tete verrouille
		
	if(ptr->suivant!=NULL){
		sem_wait(&ptr->suivant->semVM); // verrouille noeud suivant de ptr
	}
	else{ // ptr->suivant==NULL no invalide
		sem_post(&ptr->semVM); // deverrouille noeud de tete 
	}

	//Tant qu'un item suivant existe
	while (ptr->suivant!=NULL){
		//Déplacement du pointeur de navigation
		noeudVM* optr = ptr;
	
		ptr=ptr->suivant;
		sem_post(&optr->semVM); 

		//Est-ce l'item recherché?
		if (ptr->VM.noVM==no){
			return ptr; // retourner le noeud verrouille
		}
		if(ptr->suivant!=NULL){
			sem_wait(&ptr->suivant->semVM);
		}
		else{ // ptr->suivant==NULL no invalide
			sem_post(&ptr->semVM); // deverrouille dernier noeud verrouille
		}		
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
noeudVM* findPrev(const int no){
	//La liste est vide 
	if ((!head)&&(!queue)) return NULL;
	//Pointeur de navigation
	noeudVM *ptr = head;
	//Tant qu'un item suivant existe
	while (ptr->suivant){

		//Est-ce le prédécesseur de l'item recherché?
		if (ptr->suivant->VM.noVM == no){
			//On retourne un pointeur sur l'item précédent
			return ptr;
		}
		//Déplacement du pointeur de navigation
		ptr=ptr->suivant;
	}
	//On retourne un pointeur NULL
	return NULL;
}

//#####################################################
//# Ajoute un item a la fin de la liste chaînée de VM
//# ENTREE: 
//#	RETOUR:  
void addItem() {
	//Création de l'enregistrement en mémoire
	noeudVM* ni = (noeudVM*)malloc(sizeof(noeudVM));
	
	//printf("\n noVM=%d busy=%d adr ni=%p", ni->VM.noVM, ni->VM.busy, ni);
	//printf("\n noVM=%d busy=%d adrQ deb=%p", ni->VM.noVM, ni->VM.busy,queue);

	//Affectation des valeurs des champs
	ni->VM.noVM	= ++nbVM;
	//printf("\n noVM=%d", ni->VM.noVM);
	ni->VM.busy	= 0;
	//printf("\n busy=%d", ni->VM.busy);
	ni->VM.ptrDebutVM = (unsigned short*)malloc(sizeof(unsigned short)*VM_SEGMENT_SIZE);
	//printf("\n noVM=%d busy=%d adrptr VM=%p", ni->VM.noVM, ni->VM.busy, ni->VM.ptrDebutVM);
	//printf("\n noVM=%d busy=%d adrQ=%p", ni->VM.noVM, ni->VM.busy, queue);
	ni->suivant = NULL;

	sem_init(&ni->semVM, 0, 1);

	if ((!head) && (!queue)){ //liste vide
	  queue = head = ni;
	  return;
	}

	sem_wait(&semQ);
	((noeudVM*)queue)->suivant = ni;
	queue = ni;
	sem_post(&semQ);

	//printf("\n noVM=%d busy=%d adrQ=%p", ni->VM.noVM, ni->VM.busy, queue);	
	//printf("\n noVM=%d busy=%d adr Queue=%p", ni->VM.noVM, ni->VM.busy,queue);
}

//#######################################
//# Retire un item de la liste chaînée
//# ENTREE: arg: Pointer vers le numéro du noeud a retirer 
void removeItem(int* p_nbVM){	
	int noVM = *p_nbVM;
	
	noeudVM *ptr;
	noeudVM *tptr;
	noeudVM *optr;
	//Vérification sommaire (noVM>0 et liste non vide)	
	if ((noVM < 1) || ((!head) && (!queue)))
		return;

	//Pointeur de recherche
	if(noVM == 1) {
		ptr = head; // suppression du premier element de la liste
	} else {
		ptr = findPrev(noVM);
	}
	//L'item a été trouvé
	if (ptr){
		
		nbVM--;

		// Memorisation du pointeur de l'item en cours de suppression
		// Ajustement des pointeurs
		if((head == ptr) && (noVM == 1)) { // suppression de l'element de tete
			if(head == queue) {// un seul element dans la liste
				free(ptr->VM.ptrDebutVM);
				free(ptr);
				queue = head = NULL;
				return;
			}
			tptr = ptr->suivant;
			head = tptr;
			free(ptr->VM.ptrDebutVM);
			free(ptr);
		} else if (queue == ptr->suivant) { // suppression de l'element de queue
			queue=ptr;
			free(ptr->suivant->VM.ptrDebutVM);
			free(ptr->suivant);
			ptr->suivant=NULL;
			return;
		} else { // suppression d'un element dans la liste
			optr = ptr->suivant;	
			ptr->suivant = ptr->suivant->suivant;
			tptr = ptr->suivant;
			free(optr->VM.ptrDebutVM);
			free(optr);
		}
		
		while (tptr){ // ajustement des numeros de VM
		//Est-ce le prédécesseur de l'item recherché?
			tptr->VM.noVM--;
			//On retourne un pointeur sur l'item précédent	

		//Déplacement du pointeur de navigation
			tptr=tptr->suivant;
		}

		sem_destroy(&ptr->semVM);
	}
}

//#######################################
//#
//# Affiche les items dont le numéro séquentiel est compris dans une plage
//#
void listItems(remove_item_args* arg){
	int start = arg->nstart;
	int end = arg->nend;

	//Affichage des entêtes de colonnes
	printf("noVM  Busy?		Adresse Debut VM                        \n");
	printf("========================================================\n");

	noeudVM *ptr = head; //premier element

	while (ptr){

		//L'item a un numéro séquentiel dans l'interval défini
		if ((ptr->VM.noVM >= start) && (ptr->VM.noVM <= end)){
			printf("%d \t %d \t %p\n",
				ptr->VM.noVM,
				ptr->VM.busy, ptr->VM.ptrDebutVM
			);
		}
		if (ptr->VM.noVM > end){
			//L'ensemble des items potentiels sont maintenant passés
			//Déplacement immédiatement à la FIN de la liste
			//Notez que le pointeur optr est toujours valide
			ptr=NULL;
		} else {
			ptr = ptr->suivant;
		}

	}

	//Affichage des pieds de colonnes
	printf("========================================================\n\n");
}
