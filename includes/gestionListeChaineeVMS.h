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

typedef struct remove_item_args{
	int nstart;
	int nend;
} remove_item_args;

typedef struct execute_file_args{
	int noVM;
	char* fileName;
} execute_file_args;

void cls(void);
void error(const int exitcode, const char * message);

struct noeudVM * findItem(const int no);
struct noeudVM * findPrev(const int no);

void  addItem();
void  removeItem(int* p_nbVM);
void  listItems(remove_item_args* arg);
int   executeFile(execute_file_args* arg);
void* readTrans(char* nomFichier);
void  saveItems(const char* sourcefname);