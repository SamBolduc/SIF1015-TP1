#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>

#define VM_SEGMENT_SIZE 65536

typedef struct noeudVM noeudVM;
typedef struct infoVM infoVM;

struct infoVM {
	unsigned char  busy; 
	unsigned short *ptrDebutVM;							
};

struct noeudVM {			
	infoVM	VM;
	noeudVM	*suivant;
};
	
void cls(void);
void error(const int exitcode, const char * message);

noeudVM *findItem(const int no);
noeudVM *findPrev(const int no);

void addItem();
void removeItem(const int noVM);
void listItems(const int start, const int end);