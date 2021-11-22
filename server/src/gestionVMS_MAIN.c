/*
  #########################################################
  #
  # Titre : 	UTILITAIRES (MAIN) TP1 LINUX Automne 21
  #				SIF-1015 - Système d'exploitation
  #				Université du Québec à Trois-Rivières
  #
  # Auteur : 	Francois Meunier
  #	Date :		Septembre 2021
  #
  # Langage : 	ANSI C on LINUX 
  #
  #######################################
*/
#include "gestionListeChaineeVMS.h"
#include "gestionVMS.h"

#include <pthread.h>

/* Variables globales */

linkedList* threads = NULL;

linkedList* head;  /* Pointeur de tête de liste */
linkedList* queue; /* Pointeur de queue de liste pour ajout rapide */

int nbVM;          /* nombre de VM actives */

pthread_mutex_t headState; /* protects head and queue */
pthread_mutex_t consoleState; /* Protects console */

#define SERVER_FIFO_NAME "/tmp/serv_fifo"

int main(int argc, char* argv[]){

	/* Initialisation des pointeurs */
	head = NULL;
	queue = NULL;
	nbVM = 0;

	pthread_mutex_init(&headState, NULL);
  pthread_mutex_init(&consoleState, NULL);
  /*
  if (argc < 2)
  error(-1, "[%s/%u] Not enough arguments\n", __FILE__, __LINE__);
  */
	readTrans(SERVER_FIFO_NAME);
  pthread_mutex_destroy(&consoleState);
	pthread_mutex_destroy(&headState);

	exit( 0);
}

