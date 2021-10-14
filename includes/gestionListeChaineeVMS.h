#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>

#define VM_SEGMENT_SIZE 65536

typedef struct remove_item_args{
	int nstart;
	int nend;
} remove_item_args;

typedef struct execute_file_args{
	int noVM;
	char* fileName;
} execute_file_args;

typedef struct noeudVM noeudVM;
typedef struct infoVM infoVM;

struct infoVM {						
	int			   noVM;
	unsigned char  busy; 
	unsigned short *ptrDebutVM;							
};

struct noeudVM {			
	infoVM	VM;
	sem_t semVM;
	noeudVM	*suivant;	
};
	
void cls(void);
void error(const int exitcode, const char * message);

noeudVM *findItem(const int no);
noeudVM *findPrev(const int no);

void  addItem();
void  removeItem(int* p_nbVM);
void  listItems(remove_item_args* arg);
void* readTrans(char* nomFichier);
void  saveItems(const char* sourcefname);
