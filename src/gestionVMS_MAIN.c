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

// Variables globales

//Pointeur de tête de liste
noeudVM* head;
//Pointeur de queue de liste pour ajout rapide
noeudVM* queue;
// nombre de VM actives
int nbVM;

sem_t semH, semQ;

int main(int argc, char* argv[]){

	//Initialisation des pointeurs
	head = NULL;
	queue = NULL;
	nbVM = 0;

	//"Nettoyage" de la fenêtre console
	//cls();

	sem_init(&semH, 0, 1);
	sem_init(&semQ, 0, 1);
	
	readTrans(argv[1]);


	//Fin du programme
	exit( 0);
}

