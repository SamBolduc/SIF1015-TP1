#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>

struct infoVM{						
	int		noVM;
	unsigned char 	busy; 
	unsigned short * 	ptrDebutVM;							
	};								 

struct noeudVM{			
	struct infoVM	VM;		
	struct noeudVM		*suivant;
	};	
	
struct remove_item_args{
	int nstart;
	int nend;
};

struct execute_file_args{
	int noVM;
	char* fileName;
};

void cls(void);
void error(const int exitcode, const char * message);

struct noeudVM * findItem(const int no);
struct noeudVM * findPrev(const int no);

void *addItem(void *arg);
void *removeItem(void *arg);
void *listItems(void *arg);
void *executeFile(void *arg);
void* readTrans(char* nomFichier);
void saveItems(const char* sourcefname);