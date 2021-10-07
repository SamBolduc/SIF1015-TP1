//#########################################################
//#
//# Titre : 	UTILITAIRES (MAIN) TP1 LINUX Automne 21
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

// Variables globales

linkedList* threads = NULL;

linkedList* head;  //Pointeur de tête de liste
linkedList* queue; //Pointeur de queue de liste pour ajout rapide
int nbVM;          // nombre de VM actives

pthread_mutex_t headState;
pthread_mutex_t threadsState;

int main(int argc, char* argv[]){

	//Initialisation des pointeurs
	head = NULL;
	queue = NULL;
	nbVM = 0;

	//"Nettoyage" de la fenêtre console
	//cls();

	readTrans(argv[1]);


	//Fin du programme
	exit( 0);
}

